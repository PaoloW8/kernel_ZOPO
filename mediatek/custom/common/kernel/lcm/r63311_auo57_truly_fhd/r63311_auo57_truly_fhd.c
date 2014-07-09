/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

#ifndef BUILD_LK
#include <linux/string.h>
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK
#include <platform/mt_gpio.h>
#include <platform/mt_pmic.h>
#elif defined(BUILD_UBOOT)
#include <asm/arch/mt_gpio.h>
#else
#include <mach/mt_gpio.h>
#include <linux/xlog.h>
#include <mach/mt_pm_ldo.h>
#endif

#ifdef BUILD_LK
	#define	LCM_DEBUG	printf
#else
	#define	LCM_DEBUG	printk
#endif
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  (1080)
#define FRAME_HEIGHT (1920)

#define LCM_ID_R63311 (0x3311)
#define GPIO_LCD_RST_EN      GPIO131
// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	        lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)											lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

#define dsi_lcm_set_gpio_out(pin, out)										lcm_util.set_gpio_out(pin, out)
#define dsi_lcm_set_gpio_mode(pin, mode)									lcm_util.set_gpio_mode(pin, mode)
#define dsi_lcm_set_gpio_dir(pin, dir)										lcm_util.set_gpio_dir(pin, dir)
#define dsi_lcm_set_gpio_pull_enable(pin, en)								lcm_util.set_gpio_pull_enable(pin, en)

#define   LCM_DSI_CMD_MODE							0

void lcm_compare_id(void);

static void init_lcm_registers(void)
{
	unsigned int data_array[16];

     data_array[0]=0x00022902;
    data_array[1]=0x000004b0;//cmd mode
    dsi_set_cmdq(data_array, 2, 1);

	  data_array[0]=0x00023902;
    data_array[1]=0x00000000;//cmd mode
    dsi_set_cmdq(data_array, 2, 1);
	  data_array[0]=0x00023902;
    data_array[1]=0x00000000;//cmd mode
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00022902;
    data_array[1]=0x000001d6;//cmd mode
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00033902;
    data_array[1]=0x00ff0f51;//cmd mode
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00000453;//cmd mode
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00023902;
    data_array[1]=0x00000255;//cmd mode
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x000b2902;//set_DDB_write_control
    data_array[1]=0x000003e1;//cmd mode
    data_array[2]=0x00000000;//cmd mode
    data_array[3]=0x00000000;//cmd mode
    dsi_set_cmdq(data_array, 4, 1);

    data_array[0]=0x00072902;//Read_ID_code
    data_array[1]=0x000001e2;//cmd mode
    data_array[2]=0x00000000;//cmd mode
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0]=0x00072902;//Interface_setting(Video mode)
    data_array[1]=0x000014b3;//cmd mode
    data_array[2]=0x00000000;//cmd mode
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0]=0x00032902;//Interface_ID_setting(DSI 4lane)///////////
    data_array[1]=0x00000cb4;//cmd mode
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00032902;//DSI_control
    data_array[1]=0x00c33ab6;//cmd mode
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00022902;//Checksum_and_ECC_Error_Count
    data_array[1]=0x000000b7;//cmd mode
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00082902;//Back_Light_Control_2
    data_array[1]=0x04180fb9;//cmd mode
    data_array[2]=0x801f9f40;//cmd mode
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0]=0x00082902;//Back_Light_Control_4
    data_array[1]=0x04180fba;//cmd mode
    data_array[2]=0xd71f9f40;//cmd mode
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0]=0x00232902;//Display_Setting
    data_array[1]=0x406004C1;//cmd mode//0x406004c1
    data_array[2]=0xE83098A0;//cmd mode
	  data_array[3]=0x4A529441;//cmd mode
    data_array[4]=0x254B5A93;//cmd mode
	  data_array[5]=0xE8A5294A;//cmd mode
	  data_array[6]=0x014C2250;//cmd mode
	  data_array[7]=0x00000000;//cmd mode
    data_array[8]=0x00006000;//cmd mode
	  data_array[9]=0x00000200;//cmd mode
	  dsi_set_cmdq(data_array, 10, 1);

	  data_array[0]=0x00082902;// (sub pix-inv:0x30)
    data_array[1]=0x83f730c2;//cmd mode
    data_array[2]=0x00000805;//cmd mode
    dsi_set_cmdq(data_array, 3, 1);

	  data_array[0]=0x00042902;//TPC_Sync_Control
    data_array[1]=0x000000c3;//cmd mode
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x000172902;////Source_Timing_Setting
    data_array[1]=0x000170C4;//cmd mode
    data_array[2]=0x05050505;//cmd mode
	  data_array[3]=0x03000505;//cmd mode
    data_array[4]=0x00000000;//cmd mode
	  data_array[5]=0x00000000;//cmd mode
	  data_array[6]=0x00030000;//cmd mode
	  dsi_set_cmdq(data_array, 7, 1);

	  data_array[0]=0x00292902;
    data_array[1]=0x004947C6;//cmd mode
    data_array[2]=0x00000000;//cmd mode
	  data_array[3]=0x00000000;//cmd mode
    data_array[4]=0x00000000;//cmd mode
	  data_array[5]=0x0E190900;//cmd mode
	  data_array[6]=0x49470100;//cmd mode
	  data_array[7]=0x00000000;//cmd mode
    data_array[8]=0x00000000;//cmd mode
	  data_array[9]=0x00000000;//cmd mode
	  data_array[10]=0x19090000;//cmd mode
	  data_array[11]=0x0000000E;//cmd mode
	  dsi_set_cmdq(data_array, 12, 1);

	  data_array[0]=0x00192902;//Gamma_Setting_A
    data_array[1]=0x2B261FC7;//cmd mode
    data_array[2]=0x45533E33;//cmd mode
	  data_array[3]=0x6F6B6154;//cmd mode
    data_array[4]=0x2B261F77;//cmd mode
	  data_array[5]=0x45533E33;//cmd mode
	  data_array[6]=0x6F6B6154;//cmd mode
	  data_array[7]=0x00000077;//cmd mode
	  dsi_set_cmdq(data_array, 8, 1);

	  data_array[0]=0x00192902;//Gamma_Setting_B
    data_array[1]=0x2B261FC8;//cmd mode
    data_array[2]=0x45533E33;//cmd mode
	  data_array[3]=0x6F6B6154;//cmd mode
    data_array[4]=0x2B261F77;//cmd mode
	  data_array[5]=0x45533E33;//cmd mode
	  data_array[6]=0x6F6B6154;//cmd mode
	  data_array[7]=0x00000077;//cmd mode
	  dsi_set_cmdq(data_array, 8, 1);

	  data_array[0]=0x00192902;//Gamma_Setting_C
    data_array[1]=0x261E0BC9;//cmd mode
    data_array[2]=0x44523B2E;//cmd mode
	  data_array[3]=0x6C686055;//cmd mode
    data_array[4]=0x261E0B77;//cmd mode
	  data_array[5]=0x44523B2E;//cmd mode
	  data_array[6]=0x6C686055;//cmd mode
	  data_array[7]=0x00000077;//cmd mode
	  dsi_set_cmdq(data_array, 8, 1);

	  data_array[0]=0x00212902;//Color_enhancement set ce
    data_array[1]=0x808001CA;//cmd mode
    data_array[2]=0xA0808080;//cmd mode
	  data_array[3]=0x80141480;//cmd mode
    data_array[4]=0x374A0A80;//cmd mode
	  data_array[5]=0x0CF855A0;//cmd mode
	  data_array[6]=0x3F10200C;//cmd mode
	  data_array[7]=0x1000003F;//cmd mode
	  data_array[8]=0x3F3F3F10;//cmd mode
	  data_array[9]=0x0000003F;//cmd mode
	  dsi_set_cmdq(data_array, 10, 1);

	  data_array[0]=0x000a2902;//Panel PIN Control
    data_array[1]=0x07E0DFCB;//cmd mode
    data_array[2]=0x000000FB;//cmd mode
	  data_array[3]=0x0000C000;//cmd mode
	  dsi_set_cmdq(data_array, 4, 1);

	  data_array[0]=0x00022902;//Panel Interface Control (Type B)
    data_array[1]=0x000035cc;//cmd mode
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00042902;////Back Light Control 5
    data_array[1]=0xff0000cd;//cmd mode
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00082902;//Back Light Control 6
    data_array[1]=0x080600ce;//cmd mode
    data_array[2]=0x041e00c1;//cmd mode
    dsi_set_cmdq(data_array, 3, 1);

	  data_array[0]=0x00062902;//GPO Control
    data_array[1]=0xc10000cf;//cmd mode
    data_array[2]=0x00003f05;//cmd mode
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0]=0x000f2902;//Power Setting 1
    data_array[1]=0x190000D0;//cmd mode
    data_array[2]=0x1D9D9918;//cmd mode
	  data_array[3]=0xBB008D01;//cmd mode
    data_array[4]=0x0001D759;//cmd mode
    dsi_set_cmdq(data_array, 5, 1);

    data_array[0]=0x001e2902;/////Power Setting 2
    data_array[1]=0x000028D1;//cmd mode
    data_array[2]=0x18100c08;//cmd mode
	  data_array[3]=0x00000000;//cmd mode
    data_array[4]=0x28043C00;//cmd mode
	  data_array[5]=0x0c080000;//cmd mode
	  data_array[6]=0x00001810;//cmd mode
	  data_array[7]=0x0040053C;//cmd mode
	  data_array[8]=0x00003132;//cmd mode
	  dsi_set_cmdq(data_array, 9, 1);

	  data_array[0]=0x000042902;//Power Setting for common
    data_array[1]=0x00005cd2;//cmd mode
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x001b2902;//Power Setting for Internal Power
    data_array[1]=0xBB331BD3;//cmd mode
    data_array[2]=0x3333B3BB;//cmd mode
	  data_array[3]=0x00010033;//cmd mode
    data_array[4]=0x0DA0D8A0;//cmd mode
	  data_array[5]=0x2233334B;//cmd mode
	  data_array[6]=0x534B0270;//cmd mode
	  data_array[7]=0x0011BF3D;//cmd mode
	  dsi_set_cmdq(data_array, 8, 1);

	  data_array[0]=0x00082902;
    data_array[1]=0x000006d5;//cmd mode
    data_array[2]=0x20012001;//cmd mode
    dsi_set_cmdq(data_array, 3, 1);

	  data_array[0]=0x00152902;//sequence_Timing_Control_for_Power_On
    data_array[1]=0x7FE084D7;//cmd mode
    data_array[2]=0xFC38CEA8;//cmd mode
	  data_array[3]=0x8FE783C1;//cmd mode
    data_array[4]=0xFA103C1F;//cmd mode
	  data_array[5]=0x41040FC3;//cmd mode
	  data_array[6]=0x00000000;//cmd mode
	  dsi_set_cmdq(data_array, 7, 1);

	  data_array[0]=0x00072902;//Auto Contrast Optimize
    data_array[1]=0x808000d8;//cmd mode
    data_array[2]=0x00214240;//cmd mode
    dsi_set_cmdq(data_array, 3, 1);

	  data_array[0]=0x00032902;//Sequencer_Control
    data_array[1]=0x000000d9;//cmd mode
    dsi_set_cmdq(data_array, 2, 1);

	  data_array[0]=0x00032902;//Outline Sharpening Control
    data_array[1]=0x008c10dd;//cmd mode
    dsi_set_cmdq(data_array, 2, 1);

	  data_array[0]=0x00072902;//Test Image Generator
    data_array[1]=0x07ff00de;//cmd mode
    data_array[2]=0x007a0010;//cmd mode
	  dsi_set_cmdq(data_array, 3, 1);

//	data_array[0] = 0x00351500;
//	dsi_set_cmdq(data_array, 1, 1);


	data_array[0] = 0x00290500;
	dsi_set_cmdq(data_array, 1, 1);

	MDELAY(200);

	data_array[0] = 0x00110500;
	dsi_set_cmdq(data_array, 1, 1);

	MDELAY(200);

}

// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{

		memset(params, 0, sizeof(LCM_PARAMS));

		params->type   = LCM_TYPE_DSI;

		params->width  = FRAME_WIDTH;
		params->height = FRAME_HEIGHT;

		// enable tearing-free
		params->dbi.te_mode 				= LCM_DBI_TE_MODE_VSYNC_ONLY;
		params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

        #if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
        #else
		params->dsi.mode   =SYNC_EVENT_VDO_MODE;  //SYNC_EVENT_VDO_MODE;
        #endif

		// DSI
		/* Command mode setting */
		//1 Three lane or Four lane
		params->dsi.LANE_NUM				= LCM_FOUR_LANE;
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
		params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
		params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

		// Highly depends on LCD driver capability.
		// Not support in MT6573
		params->dsi.packet_size=256;

		// Video mode setting
		params->dsi.intermediat_buffer_num = 0;//because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage

		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
		params->dsi.word_count=1080*3;


		params->dsi.vertical_sync_active				= 2;//1
		params->dsi.vertical_backporch					= 8;//2
		params->dsi.vertical_frontporch					= 8;//2
		params->dsi.vertical_active_line				= FRAME_HEIGHT;

		params->dsi.horizontal_sync_active			= 8;//12
		params->dsi.horizontal_backporch				= 60;//72
		params->dsi.horizontal_frontporch				= 140;//120
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
		//params->dsi.pll_select=1;	//0: MIPI_PLL; 1: LVDS_PLL
		// Bit rate calculation
		//1 Every lane speed
		params->dsi.pll_div1=0;		// div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
		params->dsi.pll_div2=0;		// div2=0,1,2,3;div1_real=1,2,4,4
		params->dsi.fbk_div =15;  //0x12  // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)

}

static void lcm_init(void)
{
#if defined(BUILD_LK)
	printf("==in uboot=== lcm_init\n");
#else
	printk("==in kernel=== lcm_init\n");
#endif

	// Enable EN_PWR for R63311 PMIC
#ifdef BUILD_LK
    upmu_set_rg_vgp6_vosel(6);
    upmu_set_rg_vgp6_en(1);
#else
    hwPowerOn(MT65XX_POWER_LDO_VGP6, VOL_2800, "LCM");
#endif

    mt_set_gpio_mode(GPIO_LCD_RST_EN, GPIO_MODE_00);
    mt_set_gpio_dir(GPIO_LCD_RST_EN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
    MDELAY(20);
    mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ZERO);
    MDELAY(10);
    mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
    MDELAY(150);

    init_lcm_registers();
    mt_set_gpio_mode(GPIO_LCD_RST_EN, GPIO_MODE_01);
}

static void lcm_suspend(void)
{
	unsigned int data_array[16];
	//unsigned char buffer[2];

	mt_set_gpio_mode(GPIO_LCD_RST_EN, GPIO_MODE_00);
	mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
	MDELAY(10);
	mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ZERO);
	MDELAY(20);
	mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
	MDELAY(20);

	data_array[0]=0x00280500; // Display Off
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(120);

	data_array[0] = 0x00100500; // Sleep In
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(120);
	data_array[0]=0x00022902;
	data_array[1]=0x000004b0;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
	MDELAY(10);
	data_array[0]=0x00022902;
	data_array[1]=0x000001b1;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);
	MDELAY(60);
	mt_set_gpio_mode(GPIO_LCD_RST_EN, GPIO_MODE_01);
#ifdef BUILD_LK
        upmu_set_rg_vgp6_en(0);
#else
        hwPowerDown(MT65XX_POWER_LDO_VGP6, "LCM");
#endif

}


static void lcm_resume(void)
{
#ifndef BUILD_LK
	unsigned int data_array[16];
	//unsigned char buffer[2];

	lcm_init();
	//lcm_compare_id();
#endif
}

#if (LCM_DSI_CMD_MODE)
static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0>>8)&0xFF);
	unsigned char x0_LSB = (x0&0xFF);
	unsigned char x1_MSB = ((x1>>8)&0xFF);
	unsigned char x1_LSB = (x1&0xFF);
	unsigned char y0_MSB = ((y0>>8)&0xFF);
	unsigned char y0_LSB = (y0&0xFF);
	unsigned char y1_MSB = ((y1>>8)&0xFF);
	unsigned char y1_LSB = (y1&0xFF);

	unsigned int data_array[16];

	data_array[0]= 0x00053902;
	data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
	data_array[2]= (x1_LSB);
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0]= 0x00053902;
	data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[2]= (y1_LSB);
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0]= 0x00290508; //HW bug, so need send one HS packet
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0]= 0x002c3909;
	dsi_set_cmdq(data_array, 1, 0);

}
#endif

void lcm_compare_id(void)
{
	unsigned int data_array[16];
	unsigned char buffer[5];
	unsigned int temp;
	unsigned int id=0x0;
#ifdef BUILD_LK
	printf("===in uboot========lcm_compare_id============\n");
#else
	printk("===in kernel========lcm_compare_id============\n");
#endif
	temp = 0;

#ifdef BUILD_LK
	upmu_set_rg_vgp6_vosel(6);
	upmu_set_rg_vgp6_en(1);
#else
	hwPowerOn(MT65XX_POWER_LDO_VGP6, VOL_2800, "LCM");
#endif

	mt_set_gpio_mode(GPIO_LCD_RST_EN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LCD_RST_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
	MDELAY(20);
	mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ZERO);
	MDELAY(10);
	mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
	MDELAY(150);
	mt_set_gpio_mode(GPIO_LCD_RST_EN, GPIO_MODE_01);

	data_array[0] = 0x00013700;// read id return two byte,version and id
	dsi_set_cmdq(data_array, 1, 1);

	read_reg_v2(0x0A, buffer, 1);   //DA:03H DA:03H DC:05H
	id = buffer[0]; //we only need ID
	LCM_DEBUG("%s, kernel r63315 horse debug: r63315 0A = 0x%08x\n", __func__, id);


	data_array[0] = 0x00013700;// read id return two byte,version and id
	dsi_set_cmdq(data_array, 1, 1);

	read_reg_v2(0x0E, buffer, 1);   //DA:03H DA:03H DC:05H
	id = buffer[0]; //we only need ID
	LCM_DEBUG("%s, kernel r63315 horse debug: r63315 0E = 0x%08x\n", __func__, id);


	data_array[0] = 0x00013700;// read id return two byte,version and id
	dsi_set_cmdq(data_array, 1, 1);

	read_reg_v2(0x05, buffer, 1);   //DA:03H DA:03H DC:05H
	id = buffer[0]; //we only need ID
	LCM_DEBUG("%s, kernel r63315 horse debug: r63315 05 = 0x%08x\n", __func__, id);

	data_array[0]=0x00022902;
	data_array[1]=0x000004b0;//cmd mode
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0] = 0x00043700;// read id return two byte,version and id
	dsi_set_cmdq(data_array, 1, 1);

	read_reg_v2(0xBF, buffer, 4);   //DA:03H DA:03H DC:05H
	id = (buffer[3] | (buffer[2] << 8));
	LCM_DEBUG("%s, kernel r63311 horse debug: r63311 BF1 = 0x%08x\n", __func__, buffer[0]);
	LCM_DEBUG("%s, kernel r63311 horse debug: r63311 BF2 = 0x%08x\n", __func__, buffer[1]);
	LCM_DEBUG("%s, kernel r63311 horse debug: r63311 BF3 = 0x%08x\n", __func__, buffer[2]);
	LCM_DEBUG("%s, kernel r63311 horse debug: r63311 BF4 = 0x%08x\n", __func__, buffer[3]);
	LCM_DEBUG("%s, kernel r63311 horse debug: r63311 id = 0x%x\n", __func__, id);

	return ((LCM_ID_R63311 == id) ? 1:0);
}

LCM_DRIVER r63311_auo57_truly_fhd_lcm_drv=
{
    .name			= "r63311_auo57_truly__fhd",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id     = lcm_compare_id,
#if (LCM_DSI_CMD_MODE)
    .update         = lcm_update,
#endif
};

