#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/bitops.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/byteorder/generic.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/rtpm_prio.h>

#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>

#include "tpd_custom_cy8ctma301.h"
#include "tpd.h"
#include <cust_eint.h>
#include <mach/mt_pm_ldo.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_boot.h>

#ifndef TPD_NO_GPIO
#include "cust_gpio_usage.h"
#endif
#define CY_USE_UPGRADE

#ifdef CY_USE_UPGRADE
static u8 fw_loader_mode = 0;
#endif
#define TPD_REG_BASE 0x00
#define TPD_SOFT_RESET_MODE 0x01
#define TPD_OP_MODE 0x00
#define TPD_LOW_PWR_MODE 0x04
#define TPD_SYSINFO_MODE 0x10
#define IS_LARGE_AREA(x)        (((x) & 0x10) >> 4)
#define GET_HSTMODE(reg)  ((reg & 0x70) >> 4)  // in op mode or not
#define GET_BOOTLOADERMODE(reg) ((reg & 0x10) >> 4)  // in bl mode
#define GET_NUM_TOUCHES(x)      ((x) & 0x0F)
extern struct tpd_device *tpd;
extern int tpd_show_version;
extern int tpd_debuglog;
extern int tpd_register_flag;
extern int tpd_load_status;

static char cal_stat=0;
static char cal_error_data=0;
static char upgrade_addr=0;;
static char upgrade_mode = 0;
static char upgrade_len = 0;

static int tpd_flag = 0;
static int tpd_halt=0;
static struct task_struct *thread = NULL;
static DECLARE_WAIT_QUEUE_HEAD(waiter);

#ifdef TPD_HAVE_BUTTON
static int tpd_keys_local[TPD_KEY_COUNT] = TPD_KEYS;
static int tpd_keys_dim_local[TPD_KEY_COUNT][4] = TPD_KEYS_DIM;
#endif
#if (defined(TPD_WARP_START) && defined(TPD_WARP_END))
static int tpd_wb_start_local[TPD_WARP_CNT] = TPD_WARP_START;
static int tpd_wb_end_local[TPD_WARP_CNT]   = TPD_WARP_END;
#endif
#if (defined(TPD_HAVE_CALIBRATION) && !defined(TPD_CUSTOM_CALIBRATION))
static int tpd_calmat_local[8]     = TPD_CALIBRATION_MATRIX;
static int tpd_def_calmat_local[8] = TPD_CALIBRATION_MATRIX;
#endif

static void tpd_eint_interrupt_handler(void);
static int touch_event_handler(void *unused);
static int tpd_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int tpd_i2c_detect(struct i2c_client *client, struct i2c_board_info *info);
static int tpd_i2c_remove(struct i2c_client *client);
extern void mt65xx_eint_unmask(unsigned int line);
extern void mt65xx_eint_mask(unsigned int line);
extern void mt65xx_eint_set_hw_debounce(kal_uint8 eintno, kal_uint32 ms);
extern kal_uint32 mt65xx_eint_set_sens(kal_uint8 eintno, kal_bool sens);
extern void mt65xx_eint_registration(kal_uint8 eintno, kal_bool Dbounce_En,
                                     kal_bool ACT_Polarity, void (EINT_FUNC_PTR)(void),
                                     kal_bool auto_umask);

static struct i2c_client *i2c_client = NULL;
static const struct i2c_device_id tpd_i2c_id[] = {{"cy8ctma301",0},{}};
static struct i2c_board_info __initdata cy8ctma301_i2c_tpd={ I2C_BOARD_INFO("cy8ctma301", (0x24))};


//static const struct i2c_device_id tpd_i2c_id[] = {{"mtk-tpd",0},{}};
//static unsigned short force[] = {0, 0xce, I2C_CLIENT_END,I2C_CLIENT_END};
//static const unsigned short * const forces[] = { force, NULL };
//static struct i2c_client_address_data addr_data = { .forces = forces,};
struct i2c_driver tpd_i2c_driver = {
    .probe = tpd_i2c_probe,
    .remove = tpd_i2c_remove,
 //   .detect = tpd_i2c_detect,
    .driver.name = "cy8ctma301",
    .id_table = tpd_i2c_id,
  //  .address_data = &addr_data,
};
/*
static int tpd_i2c_detect(struct i2c_client *client, struct i2c_board_info *info) {
    strcpy(info->type, "mtk-tpd");
    return 0;
}
*/

#ifdef CY_USE_UPGRADE
/**************************************************************************
 * Sysfs device file for upgrade
 ***************************************************************************/
static int mxncytp_cmd_validate(char *buf, size_t size)
{
    u8 i;
    u16 calChecksum, expChecksum;
    char *p;
    unsigned int len;
    int retval = 0;

    p = buf;
    len = (unsigned int)(*(p+2));

    /* Check SOP */
    if (*(p+0) != 0x01)
    {
        retval = -EINVAL;
        goto mxncytp_cmd_validate_exit;
    }

    /* Check EOP */
    if (*(p+len+7-1) != 0x17)
    {
        retval = -EINVAL;
        goto mxncytp_cmd_validate_exit;
    }

    /* Check checksum */
    calChecksum = 0;
    for (i=0; i<len+7-3; i++)
    {
        calChecksum += *(p+i);
    }

    calChecksum = (~calChecksum+1) & 0xffff;
    expChecksum = *(p+len+7-3) | ((u16)(*(p+len+7-2))<<8);

    if (calChecksum != expChecksum)
    {
        printk("%s: calChecksum is 0x%04X, expChecksum is 0x%04X\n",
                __func__, calChecksum, expChecksum);
        retval = -EINVAL;
        goto mxncytp_cmd_validate_exit;
    }

mxncytp_cmd_validate_exit:
    return retval;
}

static int mxncytp_enter_into_bootloader(void)
{
    u8 buffer[0x10];
    int retval = 0;

    if (!fw_loader_mode)
    {
        printk("%s: Force enter-into bootloader mode\n", __func__);

        mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
        msleep(20);

        /* Force enter-into bootloader mode */
        mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
        mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
        mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
        msleep(20);

        mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_GPIO);
        mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_OUT);
        mt_set_gpio_out(GPIO_CTP_EINT_PIN, GPIO_OUT_ZERO);
        msleep(20);
        mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
        msleep(150);
        mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_IN);
        msleep(100);

        if (i2c_smbus_read_i2c_block_data(i2c_client, TPD_REG_BASE, 0x8,  buffer)>=0)
        {
            printk("%s: Check  in bootloader mode, bl = %d\n", __func__, buffer[1]);
            printk("%s: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n", __func__,
                    buffer[0], buffer[1], buffer[2], buffer[3],
                    buffer[4], buffer[5], buffer[6], buffer[7]);

            fw_loader_mode =1;
        }
        else
        {
            pr_err("%s: Read bootloader status error\n", __func__);
            retval = -EIO;
            goto mxncytp_enter_into_bootloader_exit;
        }

        if ((buffer[1] & 0x10) == 0x00)
        {
            pr_err("%s: Can not switch to bootloader mode\n", __func__);
            retval = -EIO;
            goto mxncytp_enter_into_bootloader_exit;
        }
    }

    printk("%s: Enter-into bootloader mode, tp can not be used\n", __func__);

mxncytp_enter_into_bootloader_exit:
    return retval;
}

static int mxncytp_exit_bootloader(void)
{
    int retval = 0;

    mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
    msleep(30);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
    msleep(10);
    mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_EINT);
    mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_IN);
    mt_set_gpio_pull_enable(GPIO_CTP_EINT_PIN, GPIO_PULL_ENABLE);
    mt_set_gpio_pull_select(GPIO_CTP_EINT_PIN, GPIO_PULL_UP);
    msleep(100);

    mt65xx_eint_set_sens(CUST_EINT_TOUCH_PANEL_NUM,
            CUST_EINT_EDGE_SENSITIVE);

    mt65xx_eint_set_hw_debounce(CUST_EINT_TOUCH_PANEL_NUM,
            CUST_EINT_TOUCH_PANEL_DEBOUNCE_CN);

    mt65xx_eint_registration(CUST_EINT_TOUCH_PANEL_NUM,
            CUST_EINT_TOUCH_PANEL_DEBOUNCE_EN,
            CUST_EINT_TOUCH_PANEL_POLARITY,
            tpd_eint_interrupt_handler, 1);

    mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);

    fw_loader_mode = 0;

    return retval;
}

static ssize_t cy301_cal_show(struct device *dev,struct device_attribute *attr,char *buf)
{
    /*place holder for future use*/
	  return  sprintf(buf, "0x%x 0x%x \n",cal_stat,cal_error_data);
}
static ssize_t cy301_cal_store(struct device *dev,struct device_attribute *attr,const char *buf, size_t count)
{
    char buffer[8];
    int status=0;
    int cal_time_out=0;

    cal_stat=0;  // init cal result stat; 0==success;
    cal_error_data =0;
    printk("---start cal-----\n");
    mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
    buffer[0] =  0x00;
    buffer[1] =  0x18;
    i2c_master_send(i2c_client,buffer,2);
    msleep(100);
	status = i2c_smbus_read_i2c_block_data(i2c_client, 0x00, 3, &(buffer[0]));
	if(status<0) 
	{
		cal_stat =1;
		TPD_DMESG("[mtk-tpd], cal write  iic failed!!\n");
		mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
		return count;
	}  
	
	if((buffer[0]&0x78) != 0x10)
	{
		cal_stat =2;
		TPD_DMESG("[mtk-tpd], cal enter system model failed!!\n");
		mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
		return count;
	}


	if(buffer[1]!= 0x00)
	{
		buffer[0] =  0x02;
		buffer[1] =  0x2f;
		i2c_master_send(i2c_client,buffer,2);
		msleep(50);   
		i2c_smbus_read_i2c_block_data(i2c_client, 0x00, 3, &(buffer[0]));
		if(buffer[1]!= 0x00)
		{
			cal_stat =3;
			cal_error_data = buffer[1];
			TPD_DMESG("[mtk-tpd], cal common status init failed,need restart TP!!\n");
			mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
			return count;
		}
	}

	msleep(50);
	buffer[0] =  0x02;
	buffer[1] =  0x20;
	buffer[2]	=  0x00;
	i2c_master_send(i2c_client,buffer,3);
	msleep(50);
	fw_loader_mode =1;
	for(cal_time_out=0;;cal_time_out++)
	{
		i2c_smbus_read_i2c_block_data(i2c_client, 0x00, 3, &(buffer[0]));
		if(buffer[1] == 0x82)
		{
			cal_stat=0;
			cal_error_data = buffer[1];
			TPD_DMESG("[mtk-tpd], cal TP success!!\n");
			break;
		}

		if(cal_time_out > 6)
		{
			cal_stat =5;
			cal_error_data = buffer[1];
			TPD_DMESG("[mtk-tpd], cal TP fail!!\n");
			mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
			fw_loader_mode =0;
			return count;
		}
		msleep(500);
	}
	fw_loader_mode =0;

	msleep(50);
	buffer[0] =  0x02;
	buffer[1] =  0x2f;
	i2c_master_send(i2c_client,buffer,2);
	msleep(50);

	buffer[0] =  0x00;
	buffer[1] =  0x08;
	i2c_master_send(i2c_client,buffer,2);
	msleep(100);
	status = i2c_smbus_read_i2c_block_data(i2c_client, 0x00, 3, &(buffer[0]));

	if((buffer[0]&0x78) != 0x00)
	{
		cal_stat =6;
		cal_error_data = buffer[0];
		TPD_DMESG("[mtk-tpd], cal exit system model failed!!\n");
		mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
		return count;
	}

	mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
	printk("---end cal-----\n");

    /*place holder for future use*/
    return count;
}

static ssize_t cy301_fwupgradeapp_show(struct device *dev,struct device_attribute *attr,char *buf)
{
    char buffer[32];
    unsigned int bt_version;
    unsigned int tts_version;
    char id_endcust;
    char id_tpcust;
    char id_projects;
    char fw_ver;
	int string_len = 0, index;
    mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
    if(upgrade_mode == 1)
	{
		printk("%s: Start upgrade read data \n", __func__);
		printk("%s: I2C upgrade_addr=0x%02X, upgrade_mode=0x%02X, upgrade_len=0x%02X\n", __func__, upgrade_addr, upgrade_mode,upgrade_len);
		if(upgrade_len <= 8)
		{
			i2c_smbus_read_i2c_block_data(i2c_client, upgrade_addr, upgrade_len, buffer);
		}
		else if(upgrade_len <= 16)
		{
			i2c_smbus_read_i2c_block_data(i2c_client, upgrade_addr, 8, buffer);
			i2c_smbus_read_i2c_block_data(i2c_client, upgrade_addr+8, upgrade_len-8, &buffer[8]);
		}
		else if(upgrade_len <= 24)
		{
			i2c_smbus_read_i2c_block_data(i2c_client, upgrade_addr, 8, buffer);
			i2c_smbus_read_i2c_block_data(i2c_client, upgrade_addr+8, 8, &buffer[8]);
			i2c_smbus_read_i2c_block_data(i2c_client, upgrade_addr+16, upgrade_len-16, &buffer[16]);
		}
		else if(upgrade_len <= 32)
		{
			i2c_smbus_read_i2c_block_data(i2c_client, upgrade_addr, 8, buffer);
			i2c_smbus_read_i2c_block_data(i2c_client, upgrade_addr+8, 8, &buffer[8]);
			i2c_smbus_read_i2c_block_data(i2c_client, upgrade_addr+16, 8, &buffer[16]);
			i2c_smbus_read_i2c_block_data(i2c_client, upgrade_addr+24, upgrade_len-24, &buffer[24]);
		}
		else
		{
			printk("%s: read error len \n", __func__);
		}
		mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);

		for(index=0; index<upgrade_len; index++)
		{
			string_len += sprintf(buf+string_len, "0x%x ", buffer[index]);
		}
		string_len += sprintf(buf+string_len, "\n", buffer[index]);
		
		return string_len;		
	}
    else
    {
		buffer[0] =  0x00;
		buffer[1] =  0x10;
		i2c_master_send(i2c_client,buffer,2);
		msleep(50);
		i2c_smbus_read_i2c_block_data(i2c_client, 0x0f, 8, buffer);
		bt_version =(unsigned int) (buffer[0] << 8) | buffer[1];
		tts_version = (unsigned int) (buffer[2] << 8) | buffer[3];
		id_endcust = buffer[4];
		id_tpcust = buffer[5];
		id_projects = buffer[6];
		fw_ver = buffer[7];
		buffer[0] =  0x00;
		buffer[1] =  0x04;
		i2c_master_send(i2c_client,buffer,2);
		msleep(50);

		mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
		return  sprintf(buf, "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",bt_version,tts_version,id_endcust,id_tpcust,id_projects,fw_ver);
	}
}
/*upgrade from app.bin*/
static ssize_t cy301_fwupgradeapp_store(struct device *dev,struct device_attribute *attr,const char *buf, size_t size)
{
    int i;
    int retval = size;
    char cmd;
    size_t len;
    char addr;
    u8 buffer[10];
    U16 delayMs;

    printk("%s: Start downloading\n", __func__);

    if (mxncytp_cmd_validate(buf, size)<0)
    {
        pr_err("%s: Command 0x%02X is not supported\n", __func__, *(buf+1));
        retval = -EINVAL;
        goto mxncytp_drv_upgrade_store_exit;
    }

    cmd = *(buf+1);
    len = (size_t)(*(buf+2));

    switch (cmd)
    {
        case 0x42:
            printk("-----line :%d: -----\n", __LINE__);
            if (mxncytp_enter_into_bootloader()<0)
            {
                pr_err("%s: Cannot enter into bootloader mode\n", __func__);
                retval = -EIO;
                goto mxncytp_drv_upgrade_store_exit;
            }
            break;

        case 0x43:
            printk("-----line :%d: -----\n", __LINE__);
            if (mxncytp_exit_bootloader()<0)
            {
                pr_err("%s: Cannot exit bootloader mode\n", __func__);
                retval = -EIO;
                goto mxncytp_drv_upgrade_store_exit;
            }
            break;


        case 0x52:
            printk("-----line :%d: -----\n", __LINE__);
            break;

        case 0x53:
            printk("-----line :%d: -----\n", __LINE__);
            {
                printk("%s: Write I2C:", __func__);
                for (i=0; i<len; i++)
                {
                    printk("0x%02X ", *(buf+4+i));
                }
                printk("\n");

                addr = *(buf+4);

                if (i2c_smbus_write_i2c_block_data(i2c_client, addr, len-1, (buf+5))<0)
                {
                    printk("%s: Write I2C error!!!\n", __func__);
                    printk("%s: I2C addr=0x%02X, length=0x%02X\n", __func__, addr, len);
                    retval = -EIO;
                    goto mxncytp_drv_upgrade_store_exit;
                }
                break;
            }

        case 0x62:
            printk("-----line :%d: -----\n", __LINE__);
            {
                printk("%s: Write I2C:", __func__);
                for (i=0; i<len; i++)
                {
                    printk("0x%02X ", *(buf+4+i));
                }
                printk("\n");

                addr = *(buf+4);

                if (i2c_smbus_write_i2c_block_data(i2c_client, addr, len-1, (buf+5))<0)
                {
                    printk("%s: Write I2C error!!!\n", __func__);
                    printk("%s: I2C addr=0x%02X, length=0x%02X\n", __func__, addr, len);
                    retval = -EIO;
                    goto mxncytp_drv_upgrade_store_exit;
                }
                break;
            }
         case 0x63:
            printk("-----line :%d: -----\n", __LINE__);
            {
                printk("%s: Write I2C:", __func__);
                for (i=0; i<len; i++)
                {
                    printk("0x%02X ", *(buf+4+i));
                }
                printk("\n");

                upgrade_addr = *(buf+4);
								upgrade_mode = *(buf+5);
								upgrade_len = *(buf+6);
                printk("%s: I2C upgrade_addr=0x%02X, upgrade_mode=0x%02X, upgrade_len=0x%02X\n", __func__, upgrade_addr, upgrade_mode,upgrade_len);

                break;
            }
        default:
            printk("-----line :%d: -----\n", __LINE__);
            printk("%s: Command 0x%02X is not supported\n", __func__, cmd);
            retval = -EINVAL;
            goto mxncytp_drv_upgrade_store_exit;
    }

mxncytp_drv_upgrade_store_exit:
    return retval;
}

static DEVICE_ATTR(cyupdate, S_IRUGO | S_IWUSR | S_IRWXUGO, cy301_fwupgradeapp_show,cy301_fwupgradeapp_store);
static DEVICE_ATTR(cycal, S_IRUGO | S_IWUSR | S_IRWXUGO, cy301_cal_show,cy301_cal_store);

static struct attribute *cy301_attributes[] = {
    &dev_attr_cycal.attr,
    &dev_attr_cyupdate.attr,
    NULL
};

static struct attribute_group cy301_attribute_group = {
    .attrs = cy301_attributes
};

int cy301_create_sysfs(struct i2c_client *client)
{
    int err;
    err = sysfs_create_group(&client->dev.kobj, &cy301_attribute_group);
    if (0 != err) {
        dev_err(&client->dev,"%s() - ERROR: sysfs_create_group() failed.\n",__func__);
        sysfs_remove_group(&client->dev.kobj, &cy301_attribute_group);
        return -EIO;
    } else {
        //mutex_init(&g_device_mutex);
        pr_info("cy301:%s() - sysfs_create_group() succeeded.\n",__func__);
    }
    return err;
}
#endif
static int tpd_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id) {
    int err = 0;
    char buffer[8];
    int status=0;
    i2c_client = client;


    #ifndef TPD_NO_GPIO
    mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
    msleep(10);
#ifdef TPD_POWER_SOURCE_CUSTOM
    hwPowerOn(TPD_POWER_SOURCE_CUSTOM, VOL_2800, "TP");
#endif
#ifdef TPD_POWER_SOURCE_1800
    hwPowerOn(TPD_POWER_SOURCE_1800, VOL_1800, "TP");
#endif
#ifdef GPIO_CTP_EN_PIN
    mt_set_gpio_mode(GPIO_CTP_EN_PIN, GPIO_CTP_EN_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_EN_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ONE);
    msleep(5);
#endif

    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
    msleep(1);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);

    mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_EINT);
    mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_IN);
    mt_set_gpio_pull_enable(GPIO_CTP_EINT_PIN, GPIO_PULL_ENABLE);
    mt_set_gpio_pull_select(GPIO_CTP_EINT_PIN, GPIO_PULL_UP);
    #endif

    msleep(10);
    buffer[0] =  0x00;
    buffer[1] =  0x10;
    i2c_master_send(i2c_client,buffer,2);
    msleep(50);
    status = i2c_smbus_read_i2c_block_data(i2c_client, 0x11, 1, &(buffer[0]));
    if(status<0) {
        TPD_DMESG("[mtk-tpd], cy8ctma300 tpd_i2c_probe failed!!\n");
        status = i2c_smbus_read_i2c_block_data(i2c_client, 0x11, 1, &(buffer[0]));
        if(status<0) {
            TPD_DMESG("[mtk-tpd], cy8ctma300 tpd_i2c_probe retry failed!!\n");
            return status;
        }
    }

    TPD_DMESG("[mtk-tpd], cy8ctma300 tpd_i2c_probe success!!\n");
    tpd_load_status = 1;
    buffer[0] =  0x00;
    buffer[1] =  0x00;
    i2c_master_send(i2c_client,buffer,2);
    msleep(50);

#ifdef CY_USE_UPGRADE
    fw_loader_mode = 0;
#endif
    thread = kthread_run(touch_event_handler, 0, TPD_DEVICE);
    if (IS_ERR(thread)) {
        err = PTR_ERR(thread);
        TPD_DMESG(TPD_DEVICE " failed to create kernel thread: %d\n", err);
    }

    cy301_create_sysfs(client);
    mt65xx_eint_set_sens(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_SENSITIVE);
    mt65xx_eint_set_hw_debounce(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_CN);
    mt65xx_eint_registration(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_EN, CUST_EINT_TOUCH_PANEL_POLARITY, tpd_eint_interrupt_handler, 0);
    mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
    return 0;
}

void tpd_eint_interrupt_handler(void) {
	if(tpd_debuglog==1) TPD_DMESG("[mtk-tpd], %s\n", __FUNCTION__);
    TPD_DEBUG_PRINT_INT; tpd_flag=1; wake_up_interruptible(&waiter);
}
static int tpd_i2c_remove(struct i2c_client *client) {return 0;}

void tpd_down(int x, int y, int p) {
	if(tpd && tpd->dev && tpd_register_flag==1) {
    //input_report_abs(tpd->dev, ABS_PRESSURE, 100);
    input_report_key(tpd->dev, BTN_TOUCH, 1);
    input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 1);
   // input_report_abs(tpd->dev, ABS_MT_WIDTH_MAJOR, p/PRESSURE_FACTOR);
    input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
    input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
    input_report_abs(tpd->dev, ABS_MT_TRACKING_ID, p);

    input_mt_sync(tpd->dev);
    TPD_DEBUG("D[%4d %4d %4d]\n", x, y, p);
  }
}

void tpd_up(int x, int y, int p) {
	if(tpd && tpd->dev && tpd_register_flag==1) {
    //input_report_abs(tpd->dev, ABS_PRESSURE, 0);
    input_report_key(tpd->dev, BTN_TOUCH, 0);
    //input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 0);
    //input_report_abs(tpd->dev, ABS_MT_WIDTH_MAJOR, 0);
    input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
    input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
    input_mt_sync(tpd->dev);
    TPD_DEBUG("U[%4d %4d %4d]\n", x, y, 0);
  }
}

volatile unsigned long g_temptimerdiff;
int x_min=0, y_min=0, x_max=0, y_max=0, counter_pointer=0;
static int touch_event_handler(void *unused) {
    struct sched_param param = { .sched_priority = RTPM_PRIO_TPD };
    int i = 0,x1=0, y1=0, finger_num = 0,finger_id = 0;
    char buffer[32];//[16];
	u32 temp;

    sched_setscheduler(current, SCHED_RR, &param);
    g_temptimerdiff=get_jiffies_64();//jiffies;
    do {
        mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
		if(tpd_debuglog==1) {
			TPD_DMESG("[mtk-tpd] %s\n", __FUNCTION__);
		}
        set_current_state(TASK_INTERRUPTIBLE);
	if(tpd_debuglog==1)
		TPD_DMESG("[mtk-tpd], %s, tpd_halt=%d\n", __FUNCTION__, tpd_halt);
        while (tpd_halt) {tpd_flag = 0; msleep(20);}
        wait_event_interruptible(waiter, tpd_flag != 0);
        //tpd_flag = 0;
        TPD_DEBUG_SET_TIME;
        set_current_state(TASK_RUNNING);
        i2c_smbus_read_i2c_block_data(i2c_client, 0x00, 8, &(buffer[0]));
        i2c_smbus_read_i2c_block_data(i2c_client, 0x08, 8, &(buffer[8]));
        i2c_smbus_read_i2c_block_data(i2c_client, 0x10, 8, &(buffer[16]));
        i2c_smbus_read_i2c_block_data(i2c_client, 0x18, 8, &(buffer[24]));
        if((buffer[1]&0x20))
        {
            continue;
        }
	if(tpd_debuglog==1)
	{
        TPD_DMESG("[mtk-tpd]HST_MODE  : %x\n", buffer[0]);
        TPD_DMESG("[mtk-tpd]TT_MODE   : %x\n", buffer[1]);
        TPD_DMESG("[mtk-tpd]TT_STAT   : %x\n", buffer[2]);
       // TPD_DEBUG("[mtk-tpd]TOUCH_ID  : %x\n", buffer[8]);
		TPD_DMESG("[mtk-tpd]TOUCH_12ID  : %x\n", buffer[8]);
		TPD_DMESG("[mtk-tpd]TOUCH_34ID  : %x\n", buffer[21]);
	}
                finger_num = buffer[2]&0x0f;
                //finger_num = 0;
        for(i = 0; i < finger_num;i++)
        {
            switch(i)
            {
                case 0:
                    x1 = (buffer[3] << 8) | ((buffer[4])) ;
                    y1 = (buffer[5] << 8) | ((buffer[6])) ;
                    finger_id = (buffer[8] & 0xf0) >> 4;
                    tpd_down(x1, y1, finger_id);
                    break;
                case 1:
                    x1 = (buffer[9] << 8) | ((buffer[10])) ;
                    y1 = (buffer[11] << 8) | ((buffer[12])) ;
                    finger_id = (buffer[8] & 0x0f);
                    tpd_down(x1, y1, finger_id);
                    break;
                case 2:
                    x1 = (buffer[14] << 8) | ((buffer[15])) ;
                    y1 = (buffer[16] << 8) | ((buffer[17])) ;
                    finger_id = (buffer[19] & 0xf0) >> 4;
                    tpd_down(x1, y1, finger_id);
                    break;
                case 3:
                    x1 = (buffer[20] << 8) | ((buffer[21])) ;
                    y1 = (buffer[22] << 8) | ((buffer[23])) ;
                    finger_id = (buffer[19])&0x0f;
                    tpd_down(x1, y1, finger_id);
                    break;
                case 4:
                    x1 = (buffer[25] << 8) | ((buffer[26])) ;
                    y1 = (buffer[27] << 8) | ((buffer[28])) ;
                    finger_id = (buffer[30] & 0xf0) >> 4;
                    tpd_down(x1, y1, finger_id);
                    break;
            }
        }
           //printk("-----finger_num = %d-------\n",finger_num);
        if(finger_num > 0)
            input_sync(tpd->dev);
        else
        {
            x1 = (buffer[3] << 8) | ((buffer[4])) ;
            y1 = (buffer[5] << 8) | ((buffer[6])) ;
           //printk("-----x:%d,y:%d-------up\n",x1,y1);
            tpd_up(x1, y1, 0);
            //input_mt_sync(tpd->dev);
            input_sync(tpd->dev);
        }
       tpd_flag = 0;

    } while (!kthread_should_stop());
    return 0;
}

int tpd_local_init(void)
{
	if(tpd_debuglog==1) {
		TPD_DMESG("[mtk-tpd] %s\n", __FUNCTION__);
	}
     if(i2c_add_driver(&tpd_i2c_driver)!=0) {
      TPD_DMESG("unable to add i2c driver.\n");
      return -1;
    }
    if(tpd_load_status == 0)
    {
    	TPD_DMESG("add error touch panel driver.\n");
    	i2c_del_driver(&tpd_i2c_driver);
    	return -1;
    }
#ifdef TPD_HAVE_BUTTON
    tpd_button_setting(TPD_KEY_COUNT, tpd_keys_local, tpd_keys_dim_local);// initialize tpd button data
#endif

#if (defined(TPD_WARP_START) && defined(TPD_WARP_END))
    TPD_DO_WARP = 1;
    memcpy(tpd_wb_start, tpd_wb_start_local, TPD_WARP_CNT*4);
    memcpy(tpd_wb_end, tpd_wb_start_local, TPD_WARP_CNT*4);
#endif

#if (defined(TPD_HAVE_CALIBRATION) && !defined(TPD_CUSTOM_CALIBRATION))
    memcpy(tpd_calmat, tpd_calmat_local, 8*4);
    memcpy(tpd_def_calmat, tpd_def_calmat_local, 8*4);
#endif
		TPD_DMESG("end %s, %d\n", __FUNCTION__, __LINE__);
		tpd_type_cap = 1;
    return 0;
}

/* Function to manage low power suspend */
void tpd_suspend(struct early_suspend *h)
{
	if(tpd_debuglog==1) {
		TPD_DMESG("[mtk-tpd] %s\n", __FUNCTION__);
	}
  	tpd_halt = 1;
#ifdef CY_USE_UPGARDE
        if (fw_loader_mode == 1)
        {
            return ;
        }
#endif
    mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);

#ifdef TPD_POWER_SOURCE_CUSTOM
    hwPowerDown(TPD_POWER_SOURCE_CUSTOM, "TP");
#endif
#ifdef TPD_POWER_SOURCE_1800
    hwPowerDown(TPD_POWER_SOURCE_1800, "TP");
#endif
#ifdef GPIO_CTP_EN_PIN
    mt_set_gpio_mode(GPIO_CTP_EN_PIN, GPIO_CTP_EN_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_EN_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ZERO);
#endif
    mdelay(1);
    mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
}

/* Function to manage power-on resume */
void tpd_resume(struct early_suspend *h)
{
	if(tpd_debuglog==1) {
		TPD_DMESG("[mtk-tpd] %s\n", __FUNCTION__);
	}
#ifdef CY_USE_UPGRADE
        if (fw_loader_mode == 1)
        {
            return ;
        }
#endif
#ifdef TPD_POWER_SOURCE_CUSTOM
    hwPowerOn(TPD_POWER_SOURCE_CUSTOM, VOL_2800, "TP");
#endif
#ifdef TPD_POWER_SOURCE_1800
    hwPowerOn(TPD_POWER_SOURCE_1800, VOL_1800, "TP");
#endif
#ifdef GPIO_CTP_EN_PIN
    mt_set_gpio_mode(GPIO_CTP_EN_PIN, GPIO_CTP_EN_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_EN_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ONE);
#endif
    msleep(1);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
    msleep(1);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
    msleep(100);
    mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
    tpd_halt = 0;
}

static struct tpd_driver_t tpd_device_driver = {
		.tpd_device_name = "cy8ctma301",
		.tpd_local_init = tpd_local_init,
		.suspend = tpd_suspend,
		.resume = tpd_resume,
#ifdef TPD_HAVE_BUTTON
		.tpd_have_button = 1,
#else
		.tpd_have_button = 0,
#endif
};
/* called when loaded into kernel */
static int __init tpd_driver_init(void) {
    printk("MediaTek cy8ctma301 touch panel driver init\n");
    i2c_register_board_info(0, &cy8ctma301_i2c_tpd, 1);
		if(tpd_driver_add(&tpd_device_driver) < 0)
			TPD_DMESG("add generic driver failed\n");
    return 0;
}

/* should never be called */
static void __exit tpd_driver_exit(void) {
    TPD_DMESG("MediaTek cy8ctma301 touch panel driver exit\n");
    //input_unregister_device(tpd->dev);
    tpd_driver_remove(&tpd_device_driver);
}

module_init(tpd_driver_init);
module_exit(tpd_driver_exit);

