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
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  (720)
#define FRAME_HEIGHT (1280)

#define LCM_ID  (0x21)

#define GPIO_LCD_RST_EN      GPIO131
#define REGFLAG_DELAY             							0xEE
#define REGFLAG_END_OF_TABLE      							0xEF   // END OF REGISTERS MARKER
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

static struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};
static struct LCM_setting_table lcm_compare_id_setting[] = {
    // Display off sequence
    {0xf0,  5,      {0x55,0xaa,0x52,0x08,0x01}},
    {REGFLAG_DELAY, 10, {}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};
#define dsi_lcm_set_gpio_out(pin, out)										lcm_util.set_gpio_out(pin, out)
#define dsi_lcm_set_gpio_mode(pin, mode)									lcm_util.set_gpio_mode(pin, mode)
#define dsi_lcm_set_gpio_dir(pin, dir)										lcm_util.set_gpio_dir(pin, dir)
#define dsi_lcm_set_gpio_pull_enable(pin, en)								lcm_util.set_gpio_pull_enable(pin, en)

#define   LCM_DSI_CMD_MODE							0
//#endif

static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
        unsigned int i;

    for(i = 0; i < count; i++) {
                
        unsigned cmd;
        cmd = table[i].cmd;
                
        switch (cmd) {
                        
            case REGFLAG_DELAY :
                MDELAY(table[i].count);
                break;
                                
            case REGFLAG_END_OF_TABLE :
                break;
                                
            default:
                dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
        }
    }
        
}

static void init_lcm_registers(void)
{
	unsigned int data_array[16];

	data_array[0] = 0x00023902;                          
    data_array[1] = 0x00009036;                 
    dsi_set_cmdq(data_array, 2, 1); 

	data_array[0] = 0x00023902;//CMD1                           
    data_array[1] = 0x000000FF;                 
    dsi_set_cmdq(data_array, 2, 1);     
    	
    data_array[0] = 0x00023902;//03 4lane  02 3lanes               
    data_array[1] = 0x000002BA;                 
    dsi_set_cmdq(data_array, 2, 1);    
    	
    data_array[0] = 0x00023902;//03 Video 08 command
    #if (LCM_DSI_CMD_MODE)
		data_array[1] = 0x000008C2; 
    #else
		data_array[1] = 0x000003C2; 
    #endif                
    dsi_set_cmdq(data_array, 2, 1);   
    	
    data_array[0] = 0x00023902;//CMD2,Page0  
    data_array[1] = 0x000001FF;                 
    dsi_set_cmdq(data_array, 2, 1);   
    	
    data_array[0] = 0x00023902;//720*1280 
    data_array[1] = 0x00003A00;                 
    dsi_set_cmdq(data_array, 2, 1);  
    	
    data_array[0] = 0x00023902;
    data_array[1] = 0x00003301; //4401                
    dsi_set_cmdq(data_array, 2, 1);  
    	
    data_array[0] = 0x00023902;
    data_array[1] = 0x00005302; //5402               
    dsi_set_cmdq(data_array, 2, 1); 
	
	data_array[0] = 0x00023902;//VGL=-6V 
    data_array[1] = 0x00008509; //0309                
    dsi_set_cmdq(data_array, 2, 1);  
    	
    data_array[0] = 0x00023902;//VGH=+8.6V 
    data_array[1] = 0x0000250E;                 
    dsi_set_cmdq(data_array, 2, 1);  
    	
    data_array[0] = 0x00023902;//turn off VGLO regulator   
    data_array[1] = 0x00000A0F;                 
    dsi_set_cmdq(data_array, 2, 1);  
    	
    data_array[0] = 0x00023902;//GVDDP=4V     
    data_array[1] = 0x0000970B;                 
    dsi_set_cmdq(data_array, 2, 1);  
    	
    data_array[0] = 0x00023902;
    data_array[1] = 0x0000970C;                 
    dsi_set_cmdq(data_array, 2, 1);  

    data_array[0] = 0x00023902; 
    data_array[1] = 0x00008611; //8611                
    dsi_set_cmdq(data_array, 2, 1); 

	data_array[0] = 0x00023902;//VCOMDC 
    data_array[1] = 0x00000312;                 
    dsi_set_cmdq(data_array, 2, 1); 
    	
    data_array[0] = 0x00023902;  
    data_array[1] = 0x00007B36;                 
    dsi_set_cmdq(data_array, 2, 1);
	
#if 1
	data_array[0] = 0x00023902;  
    data_array[1] = 0x000080B0;                 
    dsi_set_cmdq(data_array, 2, 1); 

	data_array[0] = 0x00023902;  
    data_array[1] = 0x000002B1;                 
    dsi_set_cmdq(data_array, 2, 1); 
#endif 

    data_array[0] = 0x00023902;//GVDDP=4V     
    data_array[1] = 0x00002C71;                 
    dsi_set_cmdq(data_array, 2, 1);  
#if 1
    data_array[0] = 0x00023902;
    data_array[1] = 0x000005FF;         
    dsi_set_cmdq(data_array, 2, 1);   

	data_array[0] = 0x00023902; /////////////LTPS 
    data_array[1] = 0x00000001;                   
    dsi_set_cmdq(data_array, 2, 1);              
    data_array[0] = 0x00023902;                   
    data_array[1] = 0x00008D02;                   
    dsi_set_cmdq(data_array, 2, 1);              
    data_array[0] = 0x00023902;                   
    data_array[1] = 0x00008D03;                   
    dsi_set_cmdq(data_array, 2, 1);              
    data_array[0] = 0x00023902;                   
    data_array[1] = 0x00008D04;     
    dsi_set_cmdq(data_array, 2, 1);
    data_array[0] = 0x00023902;     
    data_array[1] = 0x00003005;     
    dsi_set_cmdq(data_array, 2, 1);
    data_array[0] = 0x00023902;//06         
    data_array[1] = 0x00003306;             
    dsi_set_cmdq(data_array, 2, 1); 
	
    data_array[0] = 0x00023902;             
    data_array[1] = 0x00007707;             
    dsi_set_cmdq(data_array, 2, 1);        
    data_array[0] = 0x00023902;             
    data_array[1] = 0x00000008;        
    dsi_set_cmdq(data_array, 2, 1);   
    data_array[0] = 0x00023902;        
    data_array[1] = 0x00000009;        
    dsi_set_cmdq(data_array, 2, 1);   
    data_array[0] = 0x00023902;        
    data_array[1] = 0x0000000A;        
    dsi_set_cmdq(data_array, 2, 1);   
    data_array[0] = 0x00023902;        
    data_array[1] = 0x0000800B;     
    dsi_set_cmdq(data_array, 2, 1);
    data_array[0] = 0x00023902;//0C 
    data_array[1] = 0x0000C80C;     
    dsi_set_cmdq(data_array, 2, 1);
    data_array[0] = 0x00023902; //0D
    data_array[1] = 0x0000000D;     
    dsi_set_cmdq(data_array, 2, 1);
    data_array[0] = 0x00023902;     
    data_array[1] = 0x00001B0E; 
	
    dsi_set_cmdq(data_array, 2, 1);
    data_array[0] = 0x00023902;     
    data_array[1] = 0x0000070F;     
    dsi_set_cmdq(data_array, 2, 1);
    data_array[0] = 0x00023902;     
    data_array[1] = 0x00005710;     
    dsi_set_cmdq(data_array, 2, 1);
    data_array[0] = 0x00023902;     
    data_array[1] = 0x00000011;     
    dsi_set_cmdq(data_array, 2, 1);
    data_array[0] = 0x00023902;//12 
    data_array[1] = 0x00000012;     
    dsi_set_cmdq(data_array, 2, 1);
    data_array[0] = 0x00023902;            
    data_array[1] = 0x00001E13;            
    dsi_set_cmdq(data_array, 2, 1);       
    data_array[0] = 0x00023902;            
    data_array[1] = 0x00000014;            
    dsi_set_cmdq(data_array, 2, 1);       
    data_array[0] = 0x00023902;            
    data_array[1] = 0x00001A15;            
    dsi_set_cmdq(data_array, 2, 1);       
    data_array[0] = 0x00023902;            
    data_array[1] = 0x00000516;            
    dsi_set_cmdq(data_array, 2, 1); 
	
    data_array[0] = 0x00023902;            
    data_array[1] = 0x00000017;             
    dsi_set_cmdq(data_array, 2, 1);     
    data_array[0] = 0x00023902;//12 
    data_array[1] = 0x00001E18;     
    dsi_set_cmdq(data_array, 2, 1);
    data_array[0] = 0x00023902;            
    data_array[1] = 0x0000FF19;            
    dsi_set_cmdq(data_array, 2, 1);       
    data_array[0] = 0x00023902;            
    data_array[1] = 0x0000001A;            
    dsi_set_cmdq(data_array, 2, 1);       
    data_array[0] = 0x00023902;            
    data_array[1] = 0x0000FC1B;            
    dsi_set_cmdq(data_array, 2, 1);       
    data_array[0] = 0x00023902;            
    data_array[1] = 0x0000801C;            
    dsi_set_cmdq(data_array, 2, 1);       
    data_array[0] = 0x00023902;            
    data_array[1] = 0x0000001D; //101D            
    dsi_set_cmdq(data_array, 2, 1);     
    data_array[0] = 0x00023902;
	data_array[1] = 0x0000101E; //011E            
	dsi_set_cmdq(data_array, 2, 1);     
			                                     
	data_array[0] = 0x00023902;          
    data_array[1] = 0x0000771F;          
    dsi_set_cmdq(data_array, 2, 1);  
	data_array[0] = 0x00023902;                                   
    data_array[1] = 0x00000020;          
    dsi_set_cmdq(data_array, 2, 1);     
    data_array[0] = 0x00023902;          
    data_array[1] = 0x00000221;         
    dsi_set_cmdq(data_array, 2, 1);     
    data_array[0] = 0x00023902;          
    data_array[1] = 0x00000022; //5522          
    dsi_set_cmdq(data_array, 2, 1);      
    data_array[0] = 0x00023902;            
    data_array[1] = 0x00000D23;            
    dsi_set_cmdq(data_array, 2, 1);        
    data_array[0] = 0x00023902;//06 
    data_array[1] = 0x0000A031;     
    dsi_set_cmdq(data_array, 2, 1);
    data_array[0] = 0x00023902;     
    data_array[1] = 0x00000032;     
    dsi_set_cmdq(data_array, 2, 1);
    data_array[0] = 0x00023902;     
    data_array[1] = 0x0000B833;         
    dsi_set_cmdq(data_array, 2, 1);
	
    data_array[0] = 0x00023902;            
    data_array[1] = 0x0000BB34;            
    dsi_set_cmdq(data_array, 2, 1);        
    data_array[0] = 0x00023902;             
    data_array[1] = 0x00001135;             
    dsi_set_cmdq(data_array, 2, 1);        
    data_array[0] = 0x00023902;             
    data_array[1] = 0x00000136;             
    dsi_set_cmdq(data_array, 2, 1);        
    data_array[0] = 0x00023902;//0C         
    data_array[1] = 0x00000B37;             
    dsi_set_cmdq(data_array, 2, 1);        
    data_array[0] = 0x00023902; //0D        
    data_array[1] = 0x00000138;             
    dsi_set_cmdq(data_array, 2, 1);        
    data_array[0] = 0x00023902;             
    data_array[1] = 0x00000B39;             
    dsi_set_cmdq(data_array, 2, 1); 	
    data_array[0] = 0x00023902;             
    data_array[1] = 0x00000844;             
    dsi_set_cmdq(data_array, 2, 1);        
    data_array[0] = 0x00023902;             
    data_array[1] = 0x00008045;             
    dsi_set_cmdq(data_array, 2, 1); 
	
    data_array[0] = 0x00023902;                
    data_array[1] = 0x0000CC46;                
    dsi_set_cmdq(data_array, 2, 1);           
    data_array[0] = 0x00023902;//12            
    data_array[1] = 0x00000447;                
    dsi_set_cmdq(data_array, 2, 1);           
    data_array[0] = 0x00023902;                          
    data_array[1] = 0x00000048;                          
    dsi_set_cmdq(data_array, 2, 1);                     
    data_array[0] = 0x00023902;                          
    data_array[1] = 0x00000049;                                 
    dsi_set_cmdq(data_array, 2, 1);                            
    data_array[0] = 0x00023902;                                 
    data_array[1] = 0x0000014A;                                 
    dsi_set_cmdq(data_array, 2, 1);  
	data_array[0] = 0x00023902;                                 
    data_array[1] = 0x0000036C;                                 
    dsi_set_cmdq(data_array, 2, 1);                            
    data_array[0] = 0x00023902;                                 
    data_array[1] = 0x0000036D;                                 
    dsi_set_cmdq(data_array, 2, 1);                            
    data_array[0] = 0x00023902;//18                             
    data_array[1] = 0x00002F6E;                                 
	dsi_set_cmdq(data_array, 2, 1); 		
			
    data_array[0] = 0x00023902; ////
    data_array[1] = 0x00000043;     
    dsi_set_cmdq(data_array, 2, 1);
    data_array[0] = 0x00023902;     
    data_array[1] = 0x0000234B;     
    dsi_set_cmdq(data_array, 2, 1);
    data_array[0] = 0x00023902;     
    data_array[1] = 0x0000014C;     
    dsi_set_cmdq(data_array, 2, 1);
    data_array[0] = 0x00023902;      
    data_array[1] = 0x00002350;      
    dsi_set_cmdq(data_array, 2, 1); 
    data_array[0] = 0x00023902;      
    data_array[1] = 0x00000151;      
    dsi_set_cmdq(data_array, 2, 1); 
    data_array[0] = 0x00023902;//06  
    data_array[1] = 0x00002358;      
    dsi_set_cmdq(data_array, 2, 1); 
    data_array[0] = 0x00023902;      
    data_array[1] = 0x00000159;     
    dsi_set_cmdq(data_array, 2, 1);
    data_array[0] = 0x00023902;     
    data_array[1] = 0x0000235D;     
    dsi_set_cmdq(data_array, 2, 1);
    data_array[0] = 0x00023902;     
    data_array[1] = 0x0000015E;     
    dsi_set_cmdq(data_array, 2, 1);
    data_array[0] = 0x00023902;     
    data_array[1] = 0x00002362;     
    dsi_set_cmdq(data_array, 2, 1);
    data_array[0] = 0x00023902;     
    data_array[1] = 0x00000163;     
    dsi_set_cmdq(data_array, 2, 1);
    data_array[0] = 0x00023902;//0C 
    data_array[1] = 0x00002367;       
    dsi_set_cmdq(data_array, 2, 1); 
    data_array[0] = 0x00023902; //0D
    data_array[1] = 0x00000168;     
    dsi_set_cmdq(data_array, 2, 1);
    data_array[0] = 0x00023902;     
    data_array[1] = 0x00000089;     
    dsi_set_cmdq(data_array, 2, 1);
    data_array[0] = 0x00023902;     
    data_array[1] = 0x0000018D;     
    dsi_set_cmdq(data_array, 2, 1);
    data_array[0] = 0x00023902;     
    data_array[1] = 0x0000648E;
    dsi_set_cmdq(data_array, 2, 1);
	
    data_array[0] = 0x00023902;                       
    data_array[1] = 0x0000208F;                       
    dsi_set_cmdq(data_array, 2, 1); 	
	data_array[0] = 0x00023902;//12                   
    data_array[1] = 0x00008E97;                       
    dsi_set_cmdq(data_array, 2, 1);                  
    data_array[0] = 0x00023902;                                 
    data_array[1] = 0x00008C82;                                 
    dsi_set_cmdq(data_array, 2, 1);                            
    data_array[0] = 0x00023902;                                 
    data_array[1] = 0x00000283;                                 
    dsi_set_cmdq(data_array, 2, 1);                            
    data_array[0] = 0x00023902;                                 
    data_array[1] = 0x00000ABB;                                 
    dsi_set_cmdq(data_array, 2, 1);                            
    data_array[0] = 0x00023902;                                 
    data_array[1] = 0x00000ABC; // 02BC                                
    dsi_set_cmdq(data_array, 2, 1);                            
    data_array[0] = 0x00023902;                                 
    data_array[1] = 0x00002524;                                 
    dsi_set_cmdq(data_array, 2, 1);                            
    data_array[0] = 0x00023902;//18                             
    data_array[1] = 0x00005525;                                 
	dsi_set_cmdq(data_array, 2, 1); 	
			
	data_array[0] = 0x00023902;      
    data_array[1] = 0x00000526;      
    dsi_set_cmdq(data_array, 2, 1); 
    data_array[0] = 0x00023902;      
    data_array[1] = 0x00002327;      
    dsi_set_cmdq(data_array, 2, 1); 
    data_array[0] = 0x00023902;      
    data_array[1] = 0x00000128;      
    dsi_set_cmdq(data_array, 2, 1); 
    data_array[0] = 0x00023902;      
    data_array[1] = 0x00003129; // 0029     
    dsi_set_cmdq(data_array, 2, 1); 
    data_array[0] = 0x00023902;      
    data_array[1] = 0x00005D2A;      
    dsi_set_cmdq(data_array, 2, 1); 
    data_array[0] = 0x00023902;//06 
    data_array[1] = 0x0000012B;     
    dsi_set_cmdq(data_array, 2, 1);
    data_array[0] = 0x00023902;     
    data_array[1] = 0x0000002F;     
    dsi_set_cmdq(data_array, 2, 1);
    data_array[0] = 0x00023902;     
    data_array[1] = 0x00001030; 
    dsi_set_cmdq(data_array, 2, 1); 
	
    data_array[0] = 0x00023902;             
    data_array[1] = 0x000012A7;             
    dsi_set_cmdq(data_array, 2, 1);        
    data_array[0] = 0x00023902;             
    data_array[1] = 0x0000032D;             
    dsi_set_cmdq(data_array, 2, 1);
#endif

    data_array[0] = 0x00023902;////CMD1 
    data_array[1] = 0x000000FF;                 
    dsi_set_cmdq(data_array, 2, 1);    
    data_array[0] = 0x00023902;
    data_array[1] = 0x000001FB;                 
    dsi_set_cmdq(data_array, 2, 1);    
    data_array[0] = 0x00023902;//CMD2,Page0 
    data_array[1] = 0x000001FF;                 
    dsi_set_cmdq(data_array, 2, 1);       
    data_array[0] = 0x00023902;
    data_array[1] = 0x000001FB;                 
    dsi_set_cmdq(data_array, 2, 1);   
    data_array[0] = 0x00023902;//CMD2,Page1 
    data_array[1] = 0x000002FF;                 
    dsi_set_cmdq(data_array, 2, 1);       	
    data_array[0] = 0x00023902;
    data_array[1] = 0x000001FB;                 
    dsi_set_cmdq(data_array, 2, 1);       
    	
    data_array[0] = 0x00023902;//CMD2,Page2 
    data_array[1] = 0x000003FF;                 
    dsi_set_cmdq(data_array, 2, 1);       	
    data_array[0] = 0x00023902;
    data_array[1] = 0x000001FB;                 
    dsi_set_cmdq(data_array, 2, 1);     
    data_array[0] = 0x00023902;//CMD2,Page3
    data_array[1] = 0x000004FF;         
    dsi_set_cmdq(data_array, 2, 1);                                        
    data_array[0] = 0x00023902;         
    data_array[1] = 0x000001FB;         
    dsi_set_cmdq(data_array, 2, 1);    
    data_array[0] = 0x00023902;//CMD2,Page4
    data_array[1] = 0x000005FF;         
    dsi_set_cmdq(data_array, 2, 1);    
    data_array[0] = 0x00023902;         
    data_array[1] = 0x000001FB;         
    dsi_set_cmdq(data_array, 2, 1);
	data_array[0] = 0x00023902;     ////CMD1     
    data_array[1] = 0x000000FF;         
    dsi_set_cmdq(data_array, 2, 1); 

	/*******debug-----start********/
	data_array[0] = 0x00110500;                
    dsi_set_cmdq(data_array, 1, 1); 
    MDELAY(120); 
    	
    data_array[0] = 0x00023902;//not open CABC    
    data_array[1] = 0x0000FF51;         
    dsi_set_cmdq(data_array, 2, 1);    
    	                                    
    data_array[0] = 0x00023902;         
    data_array[1] = 0x00002C53;         
    dsi_set_cmdq(data_array, 2, 1); 
    	
    data_array[0] = 0x00023902;     
    data_array[1] = 0x00000055;         
    dsi_set_cmdq(data_array, 2, 1);  
    	
    data_array[0] = 0x00290500;                
    dsi_set_cmdq(data_array, 1, 1); 
    	
    data_array[0] = 0x00023902;         
    data_array[1] = 0x000000FF;         
    dsi_set_cmdq(data_array, 2, 1); 
    	
    data_array[0] = 0x00023902;     
    data_array[1] = 0x00000035;         
    dsi_set_cmdq(data_array, 2, 1); 
	
	data_array[0] = 0x00033902;
	data_array[1] = (((FRAME_HEIGHT/2)&0xFF) << 16) | (((FRAME_HEIGHT/2)>>8) << 8) | 0x44;
	dsi_set_cmdq(data_array, 2, 1);
	/*******debug-----end********/

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

		params->dbi.te_mode                             = LCM_DBI_TE_MODE_VSYNC_ONLY;
            //params->dbi.te_mode                               = LCM_DBI_TE_MODE_DISABLED;
                params->dbi.te_edge_polarity            = LCM_POLARITY_RISING;
        #if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
        #else
		params->dsi.mode   = SYNC_PULSE_VDO_MODE;//BURST_VDO_MODE;
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
		params->dsi.compatibility_for_nvk=1;

		// Video mode setting		
		params->dsi.intermediat_buffer_num = 2;//because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage

		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
		params->dsi.word_count=720*3;	

		
		params->dsi.vertical_sync_active				= 2;
        params->dsi.vertical_backporch                  = 3; //---3
        params->dsi.vertical_frontporch                 = 20;  //----20
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

        params->dsi.horizontal_sync_active              = 10;  //----10
		params->dsi.horizontal_backporch				= 118; //----118
		params->dsi.horizontal_frontporch				= 118; //----118
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
		//params->dsi.pll_select=1;	//0: MIPI_PLL; 1: LVDS_PLL
		// Bit rate calculation
		//1 Every lane speed
		params->dsi.pll_div1=0;		// div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
		params->dsi.pll_div2=1;		// div2=0,1,2,3;div1_real=1,2,4,4
		params->dsi.fbk_div =18;//15;    // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)

}

static struct LCM_setting_table lcm_initialization_setting[] = {

#if 1
{0xF0,5,{ 0x55, 0xAA, 0x52, 0x08, 0x00}},
{0xB1,2,{ 0x68, 0x21}},
{0xB5,1,{ 0xC8}},
{0xB6,1,{ 0x0F}},
{0xB8,4,{ 0x00, 0x00, 0x0A, 0x00}},
{0xB9,1,{ 0x00}},
{0xBA,1,{ 0x02}},
{0xBB,2,{ 0x63, 0x63}},
//{0xBC,2,{ 0x00, 0x00}}, //Column inversion
{0xBC,2,{ 0x02, 0x02}},	 //	2-dot inversion 

{0xBD,5,{ 0x02, 0x7F, 0x0D, 0x0B, 0x00}},
{0xCC,16,{ 0x41, 0x36, 0x87, 0x54, 0x46, 0x65, 0x10, 0x12, 0x14, 0x10, 0x12, 0x14, 0x40, 0x08, 0x15, 0x05}},
{0xD0,1,{ 0x00}},
{0xD1,16,{ 0x00, 0x04, 0x08, 0x0C, 0x10, 0x14, 0x18, 0x1C, 0x20, 0x24, 0x28, 0x2C, 0x30, 0x34, 0x38, 0x3C}},
{0xD3,1,{ 0x00}},
{0xD6,2,{ 0x44, 0x44}},
{0xD7,12,{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
{0xD8,13,{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
{0xD9,2,{ 0x03, 0x06}},
{0xE5,2,{ 0x00, 0xFF}},
{0xE6,4,{ 0xF3, 0xEC, 0xE7, 0xDF}},
{0xE7,10,{ 0xF3, 0xD9, 0xCC, 0xCD, 0xB3, 0xA6, 0x99, 0x99, 0x99, 0x95}},
{0xE8,10,{ 0xF3, 0xD9, 0xCC, 0xCD, 0xB3, 0xA6, 0x99, 0x99, 0x99, 0x95}},
{0xE9,2,{ 0x00, 0x04}},
{0xEA,1,{ 0x00}},
{0xEE,4,{ 0x87, 0x78, 0x00, 0x00}},
{0xEF,2,{ 0x07, 0xFF}},
{0xF0,5,{ 0x55, 0xAA, 0x52, 0x08, 0x01}},
{0xB0,2,{ 0x0D, 0x0D}}, 
{0xB1,2,{ 0x0D, 0x0D}}, 
{0xB3,2,{ 0x2D, 0x2D}}, 
{0xB4,2,{ 0x19, 0x19}}, 
{0xB5,2,{ 0x04, 0x04}}, 
{0xB6,2,{ 0x05, 0x05}}, 
{0xB7,2,{ 0x05, 0x05}}, 
{0xB8,2,{ 0x05, 0x05}}, 
{0xB9,2,{ 0x33, 0x33}}, 
{0xBA,2,{ 0x16, 0x16}}, 
{0xBC,2,{ 0x50, 0x00}}, 
{0xBD,2,{ 0x50, 0x00}}, 
{0xBE,1,{ 0x26}},   //20~30
//{0xBF,1,{ 0x26}}, //20~30
{0xC0,1,{ 0x04}},  
{0xC1,1,{ 0x00}},  
{0xC2,2,{ 0x19, 0x19}},
{0xC3,2,{ 0x0A, 0x0A}}, 
{0xC4,2,{ 0x23, 0x23}}, 
{0xC7,3,{ 0x00, 0x80, 0x00}},
{0xC9,6,{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
{0xCA,1,{ 0x01}}, 
{0xCB,2,{ 0x0B, 0x53}},
{0xCC,1,{ 0x00}},
{0xCD,3,{ 0x0B, 0x52, 0x53}}, 
{0xCE,1,{ 0x44}},
{0xCF,3,{ 0x00, 0x50, 0x50}}, 
{0xD0,2,{ 0x50, 0x50}},
{0xD1,2,{ 0x50, 0x50}},  
{0xD2,1,{ 0x37}}, 
{0xD3,1,{ 0x39}},

{0xF0,5,{ 0x55, 0xAA, 0x52, 0x08, 0x02}},
{0xb0,16,{ 0x00, 0xac, 0x00, 0xba, 0x00, 0xd9, 0x00, 0xed, 0x01, 0x01, 0x01, 0x1e, 0x01, 0x3a, 0x01, 0x62}},
{0xb1,16,{ 0x01, 0x85, 0x01, 0xb8, 0x01, 0xe4, 0x02, 0x27, 0x02, 0x5b, 0x02, 0x5d, 0x02, 0x8c, 0x02, 0xbe}},
{0xb2,16,{ 0x02, 0xdf, 0x03, 0x0c, 0x03, 0x2a, 0x03, 0x51, 0x03, 0x6d, 0x03, 0x8d, 0x03, 0xa4, 0x03, 0xbe}},
{0xb3,4,{ 0x03, 0xcc, 0x03, 0xcc}},
{0xb4,16,{ 0x00, 0xac, 0x00, 0xba, 0x00, 0xd9, 0x00, 0xed, 0x01, 0x01, 0x01, 0x1e, 0x01, 0x3a, 0x01, 0x62}},
{0xb5,16,{ 0x01, 0x85, 0x01, 0xb8, 0x01, 0xe4, 0x02, 0x27, 0x02, 0x5b, 0x02, 0x5d, 0x02, 0x8c, 0x02, 0xbe}},
{0xb6,16,{ 0x02, 0xdf, 0x03, 0x0c, 0x03, 0x2a, 0x03, 0x51, 0x03, 0x6d, 0x03, 0x8d, 0x03, 0xa4, 0x03, 0xbe}},
{0xb7,4,{ 0x03, 0xcc, 0x03, 0xcc}},		 
{0xb8,16,{ 0x00, 0xac, 0x00, 0xba, 0x00, 0xd9, 0x00, 0xed, 0x01, 0x01, 0x01, 0x1e, 0x01, 0x3a, 0x01, 0x62}},
{0xb9,16,{ 0x01, 0x85, 0x01, 0xb8, 0x01, 0xe4, 0x02, 0x27, 0x02, 0x5b, 0x02, 0x5d, 0x02, 0x8c, 0x02, 0xbe}},
{0xba,16,{ 0x02, 0xdf, 0x03, 0x0c, 0x03, 0x2a, 0x03, 0x51, 0x03, 0x6d, 0x03, 0x8d, 0x03, 0xa4, 0x03, 0xbe}},
{0xbb,4,{ 0x03, 0xcc, 0x03, 0xcc}},	
{0xbc,16,{ 0x00, 0xac, 0x00, 0xba, 0x00, 0xd9, 0x00, 0xed, 0x01, 0x01, 0x01, 0x1e, 0x01, 0x3a, 0x01, 0x62}},
{0xbd,16,{ 0x01, 0x85, 0x01, 0xb8, 0x01, 0xe4, 0x02, 0x27, 0x02, 0x5b, 0x02, 0x5d, 0x02, 0x8c, 0x02, 0xbe}},
{0xbe,16,{ 0x02, 0xdf, 0x03, 0x0c, 0x03, 0x2a, 0x03, 0x51, 0x03, 0x6d, 0x03, 0x8d, 0x03, 0xa4, 0x03, 0xbe}},
{0xbf,4,{ 0x03, 0xcc, 0x03, 0xcc}},	
{0xc0,16,{ 0x00, 0xac, 0x00, 0xba, 0x00, 0xd9, 0x00, 0xed, 0x01, 0x01, 0x01, 0x1e, 0x01, 0x3a, 0x01, 0x62}},
{0xc1,16,{ 0x01, 0x85, 0x01, 0xb8, 0x01, 0xe4, 0x02, 0x27, 0x02, 0x5b, 0x02, 0x5d, 0x02, 0x8c, 0x02, 0xbe}},
{0xc2,16,{ 0x02, 0xdf, 0x03, 0x0c, 0x03, 0x2a, 0x03, 0x51, 0x03, 0x6d, 0x03, 0x8d, 0x03, 0xa4, 0x03, 0xbe}},
{0xc3,4,{ 0x03, 0xcc, 0x03, 0xcc}},	
{0xc4,16,{ 0x00, 0xac, 0x00, 0xba, 0x00, 0xd9, 0x00, 0xed, 0x01, 0x01, 0x01, 0x1e, 0x01, 0x3a, 0x01, 0x62}},
{0xc5,16,{ 0x01, 0x85, 0x01, 0xb8, 0x01, 0xe4, 0x02, 0x27, 0x02, 0x5b, 0x02, 0x5d, 0x02, 0x8c, 0x02, 0xbe}},
{0xc6,16,{ 0x02, 0xdf, 0x03, 0x0c, 0x03, 0x2a, 0x03, 0x51, 0x03, 0x6d, 0x03, 0x8d, 0x03, 0xa4, 0x03, 0xbe}},
{0xc7,4,{ 0x03, 0xcc, 0x03, 0xcc}},	
{0xee,1,{ 0x00}},

{0xF0,5,{ 0x55, 0xAA, 0x52, 0x08, 0x03}},
{0xB0,2,{ 0x00, 0x00}},
{0xB1,2,{ 0x00, 0x00}},
{0xB2,5,{ 0x03, 0x00, 0x00, 0x00, 0x00}},
{0xB3,5,{ 0x03, 0x00, 0x00, 0x00, 0x00}},
{0xB4,5,{ 0x03, 0x00, 0x00, 0x00, 0x00}},
{0xB5,5,{ 0x03, 0x00, 0x00, 0x00, 0x00}},
{0xB6,5,{ 0x03, 0x00, 0x00, 0x00, 0x00}},
{0xB7,5,{ 0x03, 0x00, 0x00, 0x00, 0x00}},
{0xB8,5,{ 0x03, 0x00, 0x00, 0x00, 0x00}},
{0xB9,5,{ 0x03, 0x00, 0x00, 0x00, 0x00}},
{0xBA,5,{ 0x35, 0x10, 0x00, 0x00, 0x00}},
{0xBB,5,{ 0x35, 0x10, 0x00, 0x00, 0x00}},
{0xBC,5,{ 0x35, 0x10, 0x00, 0x00, 0x00}},
{0xBD,5,{ 0x35, 0x10, 0x00, 0x00, 0x00}},
{0xC0,4,{ 0x00, 0x34, 0x00, 0x00}}, 
{0xC1,4,{ 0x00, 0x34, 0x00, 0x00}}, 
{0xC2,4,{ 0x00, 0x34, 0x00, 0x00}}, 
{0xC3,4,{ 0x00, 0x34, 0x00, 0x00}}, 
{0xC4,1,{ 0x40}},
{0xC5,1,{ 0x40}},
{0xC6,1,{ 0x40}},
{0xC7,1,{ 0x40}},
{0xEF,1,{ 0x00}},
{0xF0,5,{ 0x55, 0xAA, 0x52, 0x08, 0x05}},
{0xB0,2,{ 0x1B, 0x10}},
{0xB1,2,{ 0x1B, 0x10}},
{0xB2,2,{ 0x1B, 0x10}},
{0xB3,2,{ 0x1B, 0x10}},
{0xB4,2,{ 0x1B, 0x10}},
{0xB5,2,{ 0x1B, 0x10}},
{0xB6,2,{ 0x1B, 0x10}},
{0xB7,2,{ 0x1B, 0x10}},
{0xB8,1,{ 0x00}},
{0xB9,1,{ 0x00}},
{0xBA,1,{ 0x00}},
{0xBB,1,{ 0x00}},
{0xBC,1,{ 0x00}},
{0xBD,5,{ 0x03, 0x03, 0x03, 0x00, 0x01}},
{0xC0,1,{ 0x03}},
{0xC1,1,{ 0x05}},
{0xC2,1,{ 0x03}},
{0xC3,1,{ 0x05}},
{0xC4,1,{ 0x80}},
{0xC5,1,{ 0xA2}},
{0xC6,1,{ 0x80}},
{0xC7,1,{ 0xA2}},
{0xC8,2,{ 0x01, 0x20}},
{0xC9,2,{ 0x00, 0x20}},
{0xCA,2,{ 0x01, 0x00}},
{0xCB,2,{ 0x00, 0x00}},
{0xCC,3,{ 0x00, 0x00, 0x01}},
{0xCD,3,{ 0x00, 0x00, 0x01}},
{0xCE,3,{ 0x00, 0x00, 0x01}},
{0xCF,3,{ 0x00, 0x00, 0x01}},
{0xD0,1,{ 0x00}},
{0xD1,5,{ 0x03, 0x00, 0x00, 0x07, 0x10}},
{0xD2,5,{ 0x13, 0x00, 0x00, 0x07, 0x11}},
{0xD3,5,{ 0x23, 0x00, 0x00, 0x07, 0x10}},
{0xD4,5,{ 0x33, 0x00, 0x00, 0x07, 0x11}},
{0xE5,1,{ 0x06}},
{0xE6,1,{ 0x06}},
{0xE7,1,{ 0x06}},
{0xE8,1,{ 0x06}},
{0xE9,1,{ 0x06}},
{0xEA,1,{ 0x06}},
{0xEB,1,{ 0x06}},
{0xEC,1,{ 0x06}},
{0xED,1,{ 0x31}},
{0xF0,5,{ 0x55, 0xAA, 0x52, 0x08, 0x06}},
{0xB0,2,{ 0x10, 0x11}}, 
{0xB1,2,{ 0x12, 0x13}}, 
{0xB2,2,{ 0x08, 0x00}}, 
{0xB3,2,{ 0x2D, 0x2D}}, 
{0xB4,2,{ 0x2D, 0x34}}, 
{0xB5,2,{ 0x34, 0x2D}}, 
{0xB6,2,{ 0x2D, 0x34}}, 
{0xB7,2,{ 0x34, 0x34}}, 
{0xB8,2,{ 0x02, 0x0A}}, 
{0xB9,2,{ 0x00, 0x08}}, 
{0xBA,2,{ 0x09, 0x01}}, 
{0xBB,2,{ 0x0B, 0x03}}, 
{0xBC,2,{ 0x34, 0x34}}, 
{0xBD,2,{ 0x34, 0x2D}}, 
{0xBE,2,{ 0x2D, 0x34}}, 
{0xBF,2,{ 0x34, 0x2D}}, 
{0xC0,2,{ 0x2D, 0x2D}}, 
{0xC1,2,{ 0x01, 0x09}}, 
{0xC2,2,{ 0x19, 0x18}}, 
{0xC3,2,{ 0x17, 0x16}}, 
{0xC4,2,{ 0x19, 0x18}}, 
{0xC5,2,{ 0x17, 0x16}}, 
{0xC6,2,{ 0x01, 0x09}}, 
{0xC7,2,{ 0x2D, 0x2D}}, 
{0xC8,2,{ 0x2D, 0x34}}, 
{0xC9,2,{ 0x34, 0x2D}}, 
{0xCA,2,{ 0x2D, 0x34}}, 
{0xCB,2,{ 0x34, 0x34}}, 
{0xCC,2,{ 0x0B, 0x03}}, 
{0xCD,2,{ 0x09, 0x01}}, 
{0xCE,2,{ 0x00, 0x08}}, 
{0xCF,2,{ 0x02, 0x0A}}, 
{0xD0,2,{ 0x34, 0x34}}, 
{0xD1,2,{ 0x34, 0x2D}}, 
{0xD2,2,{ 0x2D, 0x34}}, 
{0xD3,2,{ 0x34, 0x2D}}, 
{0xD4,2,{ 0x2D, 0x2D}}, 
{0xD5,2,{ 0x08, 0x00}}, 
{0xD6,2,{ 0x10, 0x11}}, 
{0xD7,2,{ 0x12, 0x13}}, 
{0xD8,5,{ 0x55, 0x55, 0x55, 0x55, 0x55}},
{0xD9,5,{ 0x55, 0x55, 0x55, 0x55, 0x55}},
{0xE5,2,{ 0x34, 0x34}},
{0xE6,2,{ 0x34, 0x34}},
{0xE7,1,{ 0x05}},      
{0x35,1,{ 0x00}},  

//ccmoff
//ccmrun
    
{0x35,1,{0x00}},  //Tearing Effect On

//{0x44,2,{0x00,0x00}},
                                                   
{0x11,1,{0x00}},  // Sleep Out
{REGFLAG_DELAY, 120, {}},                            
                                                     
{0x29,1,{0x00}}, //Display On             
{REGFLAG_DELAY, 40, {}},                             
   
{0x53,1,{ 0x2C }},     
{0x51,1,{ 0xFF }}, 

{REGFLAG_END_OF_TABLE, 0x00, {}}    
#endif
};

static void lcm_init(void)
{
#ifdef BUILD_LK
        upmu_set_rg_vgp6_vosel(6);
        upmu_set_rg_vgp6_en(1);
#else
        hwPowerOn(MT65XX_POWER_LDO_VGP6, VOL_3300, "LCM");
#endif

        mt_set_gpio_mode(GPIO_LCD_RST_EN, GPIO_MODE_00);
        mt_set_gpio_dir(GPIO_LCD_RST_EN, GPIO_DIR_OUT);
        mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
        MDELAY(10);
        mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ZERO);
	MDELAY(10);
        mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
        MDELAY(120);

   // MDELAY(10);
	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);

}

static void lcm_suspend(void)
{
	//push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
    unsigned int data_array[16];

	data_array[0] = 0x00280500;	// Display Off
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(120);

	data_array[0] = 0x00100500;	// Sleep In
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(120);

/*	mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
	MDELAY(50);
	mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ZERO);
	MDELAY(50);*/
    //mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
    //MDELAY(120);
#ifdef BUILD_LK
    upmu_set_rg_vgp6_en(0);
#else
    hwPowerDown(MT65XX_POWER_LDO_VGP6, "LCM");
#endif

}


static void lcm_resume(void)
{

	lcm_init();
	
}

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




static unsigned int lcm_compare_id(void)
{
	unsigned char  id_high=0;
	unsigned char  id_low=0;
	unsigned int id=0;
	unsigned char buffer[3];
	unsigned int array[16];  
	unsigned int data_array[16];

#ifdef BUILD_LK
    upmu_set_rg_vgp6_vosel(6);
    upmu_set_rg_vgp6_en(1);
#else
    hwPowerOn(MT65XX_POWER_LDO_VGP6, VOL_3000, "LCM");
#endif

	//Do reset here
	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(10);

	SET_RESET_PIN(1);
	MDELAY(10);

    push_table(lcm_compare_id_setting, sizeof(lcm_compare_id_setting) / sizeof(struct LCM_setting_table), 1);
    data_array[0] = 0x00063902;
    data_array[1] = 0x52AA55F0;  
    data_array[2] = 0x00000108;                
    dsi_set_cmdq(&data_array, 3, 1); 

	array[0] = 0x00033700;// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);
	
	read_reg_v2(0xC5, buffer, 3);
	id = buffer[1]; //we only need ID
    #ifdef BUILD_LK
		printf("%s, LK nt35590 debug: nt35590 id = 0x%08x buffer[0]=0x%08x,buffer[1]=0x%08x,buffer[2]=0x%08x\n", __func__, id,buffer[0],buffer[1],buffer[2]);
    #else
		printk("%s, LK nt35590 debug: nt35590 id = 0x%08x buffer[0]=0x%08x,buffer[1]=0x%08x,buffer[2]=0x%08x\n", __func__, id,buffer[0],buffer[1],buffer[2]);
    #endif

    if(id == LCM_ID)
    //if(buffer[0]==0x55 && buffer[1]==0x21)
    	return 1;
    else
        return 0;


}




int err_count = 0;

static unsigned int lcm_esd_check(void)
{
#ifndef BUILD_LK
    unsigned char buffer[8] = {0};

    unsigned int array[4];

    int i =0;

    

    array[0] = 0x00013700;    

    dsi_set_cmdq(array, 1,1);

    read_reg_v2(0x0A, buffer,8);

	printk( "nt35521_JDI lcm_esd_check: buffer[0] = %d,buffer[1] = %d,buffer[2] = %d,buffer[3] = %d,buffer[4] = %d,buffer[5] = %d,buffer[6] = %d,buffer[7] = %d\n",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7]);

    if((buffer[0] != 0x9C))/*LCD work status error,need re-initalize*/

    {

        printk( "nt35521_JDI lcm_esd_check buffer[0] = %d\n",buffer[0]);

        return TRUE;

    }

    else

    {

        if(buffer[3] != 0x02) //error data type is 0x02

        {

             return FALSE;

        }

        else

        {

             if((buffer[4] != 0) || (buffer[5] != 0x80))

             {

                  err_count++;

             }

             else

             {

                  err_count = 0;

             }             

             if(err_count >=2 )

             {

                 err_count = 0;

                 printk( "nt35521_JDI lcm_esd_check buffer[4] = %d , buffer[5] = %d\n",buffer[4],buffer[5]);

                 return TRUE;

             }

        }

        return FALSE;

    }
#endif
	
}

static unsigned int lcm_esd_recover(void)
{
	lcm_init();
	lcm_resume();

	return TRUE;
}

LCM_DRIVER nt35521_jdi50_xyl_hd_lcm_drv = 
{
    .name			= "nt35521_jdi50_xyl_hd",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id     = lcm_compare_id,
//	.esd_check = lcm_esd_check,
 //  .esd_recover = lcm_esd_recover,
#if (LCM_DSI_CMD_MODE)
    .update         = lcm_update,
#endif
    };
