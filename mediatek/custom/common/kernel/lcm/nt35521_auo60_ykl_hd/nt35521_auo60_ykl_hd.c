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
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  (720)
#define FRAME_HEIGHT (1280)

#define LCM_ID_NT35521 (0x5521)

#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif
#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))
static unsigned int lcm_esd_test = FALSE;      ///only for ESD test
#define LCM_DSI_CMD_MODE       0
#define GPIO_LCD_RST_EN      (GPIO131)
#define GPIO_AVEE_EN      (GPIO132)
#define GPIO_AVDD_EN      (GPIO133)
// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))

#define REGFLAG_DELAY             							0xFE
#define REGFLAG_END_OF_TABLE      							0x00   // END OF REGISTERS MARKER


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)									lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)				lcm_util.dsi_write_regs(addr, pdata, byte_nums)
//#define read_reg(cmd)											lcm_util.DSI_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)     

struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};
static void init_lcm_registers(void)
{
	unsigned int data_array[16];
	#if 1 
		data_array[0] = 0x00063902; 
		data_array[1] = 0x52AA55F0;
		data_array[2] = 0x00000108;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);
		data_array[0] = 0x00023902; 
		data_array[1] = 0x000004C0;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);		
	
		data_array[0] = 0x00063902; 						 
		data_array[1] = 0x52AA55F0; 
		data_array[2] = 0x00000008;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00053902; 						 
		data_array[1] = 0xA555AAFF; 
		data_array[2] = 0x00000080;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00033902; 						 
		data_array[1] = 0x002168B1; 
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00053902; 
		data_array[1] = 0x20A301BD;
		data_array[2] = 0x00000110;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00023902;
		data_array[1] = 0x0000026F;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1); 

		data_array[0] = 0x00023902; 
		data_array[1] = 0x000008B8;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);


		data_array[0] = 0x00033902;	
		data_array[1] = 0x001111BB;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x000505BC;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);
        
		data_array[0] = 0x00023902;	
		data_array[1] = 0x000050B5;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);
		
		data_array[0] = 0x00023902;
		data_array[1] = 0x000083C8;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1); 

		///////////PAGE 1
		data_array[0] = 0x00063902; 
		data_array[1] = 0x52AA55F0;
		data_array[2] = 0x00000108;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);
  
        	  
	  
		data_array[0] = 0x00033902;	
		data_array[1] = 0x000050B5;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x000F0FB0;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;
		data_array[1] = 0x000F0FB1;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);	

		data_array[0] = 0x00023902; 
		data_array[1] = 0x000066CE;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x000505B5;///FOR 50198 USE
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00023902;
		data_array[1] = 0x000066BE;// VCOM=-1.3V
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1); 

		data_array[0] = 0x00033902;	
		data_array[1] = 0x001E1EB3;///VGH=15V
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x001818B4;///VGL=-12V
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00033902;
		data_array[1] = 0x003434B9;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);	

		data_array[0] = 0x00033902;	
		data_array[1] = 0x002424BA;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x0003A0BC;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x0003A0BD;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00023902;
		data_array[1] = 0x000000CA;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1); 

		///////////PAGE 2
		data_array[0] = 0x00063902; 
		data_array[1] = 0x52AA55F0;
		data_array[2] = 0x00000208;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00023902;
		data_array[1] = 0x000001EE;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1); 

		data_array[0] = 0x00113902;
		data_array[1] = 0x000000B0;
		data_array[2] = 0x00300012;
		data_array[3] = 0x005F004A;
		data_array[4] = 0x00A40084;
		data_array[5] = 0x000000D8;
		dsi_set_cmdq(data_array, 6, 1);
		MDELAY(1);

		data_array[0] = 0x00113902;
		data_array[1] = 0x010301B1;
		data_array[2] = 0x01800146;
		data_array[3] = 0x022202D9;
		data_array[4] = 0x02650225;
		data_array[5] = 0x000000A8;
		dsi_set_cmdq(data_array, 6, 1);
		MDELAY(1);

		data_array[0] = 0x00113902;
		data_array[1] = 0x02CE02B2;
		data_array[2] = 0x031C03FD;
		data_array[3] = 0x03550340;
		data_array[4] = 0x0372036A;
		data_array[5] = 0x00000078;
		dsi_set_cmdq(data_array, 6, 1);
		MDELAY(1);

		data_array[0] = 0x00053902;
		data_array[1] = 0x038003B3;
		data_array[2] = 0x000000FF;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1); 

		data_array[0] = 0x00023902; 
		data_array[1] = 0x0000026F;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00023902; 
		data_array[1] = 0x000044F8;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00023902; 
		data_array[1] = 0x00000F6F;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00023902; 
		data_array[1] = 0x000014FA;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00023902; 
		data_array[1] = 0x0000026F;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00023902; 
		data_array[1] = 0x0000EFF7;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00023902; 
		data_array[1] = 0x00000A6F;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00023902; 
		data_array[1] = 0x0000D8F4;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);
		
		data_array[0] = 0x00023902;
		data_array[1] = 0x00000E6F;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1); 
		
		data_array[0] = 0x00023902; 
		data_array[1] = 0x000044F4;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);
		
		data_array[0] = 0x00023902; 
		data_array[1] = 0x0000176F;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);
		
		//data_array[0] = 0x00023902; 
		//data_array[1] = 0x000002F7;
		//dsi_set_cmdq(data_array, 2, 1);
		//MDELAY(1);

		//data_array[0] = 0x00023902; 
		//data_array[1] = 0x00000C6F;
		//dsi_set_cmdq(data_array, 2, 1);
		//MDELAY(1);





		

		

		data_array[0] = 0x00023902; 
		data_array[1] = 0x000060F4;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00023902; 
		data_array[1] = 0x0000016F;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00023902;
		data_array[1] = 0x000046F9;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1); 

		data_array[0] = 0x00063902;//////////PAGE 6
		data_array[1] = 0x52AA55F0;
		data_array[2] = 0x00000608;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x002E2EB0;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x002E2EB1;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x002E2EB2;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x00092EB3;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x00110BB4;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x001917B5;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x001D23B6;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x001F25B7;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x000301B8;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x002E2EB9;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x002E2EBA;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x000002BB;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x00241EBC;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x00221CBD;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x001618BE;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x000A10BF;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x002E08C0;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x002E2EC1;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x002E2EC2;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x002E2EC3;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x001213E5;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x002E2EC4;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x002E2EC5;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x002E2EC6;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x00022EC7;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x001800C8;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x001012C9;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x001E24CA;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x001C22CB;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x00080ACC;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x002E2ECD;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x002E2ECE;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x000B09CF;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x00231DD0;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x00251FD1;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x001311D2;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x000119D3;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x002E03D4;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x002E2ED5;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x002E2ED6;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x002E2ED7;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;
		data_array[1] = 0x001716E6;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00063902;
		data_array[1] = 0x000000D8;
		data_array[2] = 0x00000000;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00063902;
		data_array[1] = 0x000000D9;
		data_array[2] = 0x00000000;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00023902; 
		data_array[1] = 0x000000E7;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00063902;/////////PAGE 5
		data_array[1] = 0x52AA55F0;
		data_array[2] = 0x00000508;
		dsi_set_cmdq(data_array, 3, 1); 
		MDELAY(1);

		data_array[0] = 0x00023902; 
		data_array[1] = 0x000030ED;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00063902;
		data_array[1] = 0x52AA55F0;
		data_array[2] = 0x00000308;
		dsi_set_cmdq(data_array, 3, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x000020B0;
		dsi_set_cmdq(data_array, 3, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x000020B1;
		dsi_set_cmdq(data_array, 3, 1); 
		MDELAY(1);

		data_array[0] = 0x00063902;
		data_array[1] = 0x52AA55F0;
		data_array[2] = 0x00000508;
		dsi_set_cmdq(data_array, 3, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;
		data_array[1] = 0x000617B0;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00023902;
		data_array[1] = 0x000000B8;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00063902;
		data_array[1] = 0x03030FBD;
        data_array[2] = 0x00000300;
        dsi_set_cmdq(data_array, 3, 1);         
		
		data_array[0] = 0x00033902;
		data_array[1] = 0x000617B1;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x000300B9;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x000617B2;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x000300BA;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x000617B3;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x000000BB;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x000617B4;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x000617B5;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x000617B6;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x000617B7;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x000100BC;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00023902; 
		data_array[1] = 0x000006E5;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00023902; 
		data_array[1] = 0x000006E6;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00023902; 
		data_array[1] = 0x000006E7;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00023902; 
		data_array[1] = 0x000006E8;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00023902; 
		data_array[1] = 0x00000AE9;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00023902; 
		data_array[1] = 0x000006EA;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00023902; 
		data_array[1] = 0x000006EB;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00023902; 
		data_array[1] = 0x000006EC;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00063902;
		data_array[1] = 0x52AA55F0;
		data_array[2] = 0x00000508;
		dsi_set_cmdq(data_array, 3, 1); 
		MDELAY(1);

		data_array[0] = 0x00023902; 
		data_array[1] = 0x000007C0;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00023902;
		data_array[1] = 0x000005C1;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00063902;////PAGE 3
		data_array[1] = 0x52AA55F0;
		data_array[2] = 0x00000308;
		dsi_set_cmdq(data_array, 3, 1); 
		MDELAY(1);

		data_array[0] = 0x00063902;
		data_array[1] = 0x520004B2;
		data_array[2] = 0x00005101;
		dsi_set_cmdq(data_array, 3, 1); 
		MDELAY(1);

		data_array[0] = 0x00063902;
		data_array[1] = 0x520004B3;
		data_array[2] = 0x00005101;
		dsi_set_cmdq(data_array, 3, 1); 
		MDELAY(1);

		data_array[0] = 0x00063902;////////PAGE 5
		data_array[1] = 0x52AA55F0;
		data_array[2] = 0x00000508;
		dsi_set_cmdq(data_array, 3, 1); 
		MDELAY(1);

		data_array[0] = 0x00023902; 
		data_array[1] = 0x000082C4;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00023902; 
		data_array[1] = 0x000080C5;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00063902;
		data_array[1] = 0x52AA55F0;
		data_array[2] = 0x00000308;
		dsi_set_cmdq(data_array, 3, 1); 
		MDELAY(1);

		data_array[0] = 0x00063902;
		data_array[1] = 0x520004B6;
		data_array[2] = 0x00005101;
		dsi_set_cmdq(data_array, 3, 1); 
		MDELAY(1);

		data_array[0] = 0x00063902;
		data_array[1] = 0x520004B7;
		data_array[2] = 0x00005101;
		dsi_set_cmdq(data_array, 3, 1); 
		MDELAY(1);

		data_array[0] = 0x00063902;////////////
		data_array[1] = 0x52AA55F0;
		data_array[2] = 0x00000508;
		dsi_set_cmdq(data_array, 3, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x002003C8;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x002101C9;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x002003CA;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00033902;	
		data_array[1] = 0x002007CB;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00063902;////////////
		data_array[1] = 0x52AA55F0;
		data_array[2] = 0x00000308;
		dsi_set_cmdq(data_array, 3, 1); 
		MDELAY(1);

		data_array[0] = 0x00023902; 
		data_array[1] = 0x000060C4;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00023902; 
		data_array[1] = 0x000040C5;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00063902;////////////
		data_array[1] = 0x52AA55F0;
		data_array[2] = 0x00000308;
		dsi_set_cmdq(data_array, 3, 1); 
		MDELAY(1);

		data_array[0] = 0x00063902;
		data_array[1] = 0x600044BA;
		data_array[2] = 0x00007201;
		dsi_set_cmdq(data_array, 3, 1); 
		MDELAY(1);

		data_array[0] = 0x00063902;
		data_array[1] = 0x600044BB;
		data_array[2] = 0x00007201;
		dsi_set_cmdq(data_array, 3, 1); 
		MDELAY(1);

		data_array[0] = 0x00063902;
		data_array[1] = 0x030053BC;
		data_array[2] = 0x00004800;
		dsi_set_cmdq(data_array, 3, 1); 
		MDELAY(1);

		data_array[0] = 0x00063902;
		data_array[1] = 0x030053BD;
		data_array[2] = 0x00004800;
		dsi_set_cmdq(data_array, 3, 1); 
		MDELAY(1);

		data_array[0] = 0x00063902;/////////////
		data_array[1] = 0x52AA55F0;
		data_array[2] = 0x00000508;
		dsi_set_cmdq(data_array, 3, 1); 
		MDELAY(1);

		data_array[0] = 0x000C3902;
		data_array[1] = 0x000503D1;
		data_array[2] = 0x00000005;
		data_array[3] = 0x00000000;
		dsi_set_cmdq(data_array, 4, 1); 
		MDELAY(1);

		data_array[0] = 0x000C3902;
		data_array[1] = 0x000503D2;
		data_array[2] = 0x00000003;
		data_array[3] = 0x00000000;
		dsi_set_cmdq(data_array, 4, 1); 
		MDELAY(1);

		data_array[0] = 0x000C3902;
		data_array[1] = 0x040500D3;
		data_array[2] = 0x00000005;
		data_array[3] = 0x00000000;
		dsi_set_cmdq(data_array, 4, 1); 
		MDELAY(1);

		data_array[0] = 0x000C3902;
		data_array[1] = 0x040500D4;
		data_array[2] = 0x00000003;
		data_array[3] = 0x00000000;
		dsi_set_cmdq(data_array, 4, 1); 
		MDELAY(1);

		data_array[0] = 0x00063902;/////////
		data_array[1] = 0x52AA55F0;
		data_array[2] = 0x00000308;
		dsi_set_cmdq(data_array, 3, 1); 
		MDELAY(1);

		data_array[0] = 0x00023902; 
		data_array[1] = 0x000040C4;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00023902; 
		data_array[1] = 0x000040C5;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00063902;////////////
		data_array[1] = 0x52AA55F0;
        data_array[2] = 0x00000008;
		dsi_set_cmdq(data_array, 3, 1); 
		MDELAY(1);

		data_array[0] = 0x00023902;
		data_array[1] = 0x0000086F;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00023902; 
		data_array[1] = 0x000070F2;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00023902; 
		data_array[1] = 0x0000026F;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00023902; 
		data_array[1] = 0x000008B8;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00023902; 
		data_array[1] = 0x0000076F;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00023902;
		data_array[1] = 0x0000F0F4;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00063902;////////PAGE 1
		data_array[1] = 0x52AA55F0;
		data_array[2] = 0x00000108;
		dsi_set_cmdq(data_array, 3, 1); 
		MDELAY(1);

		data_array[0] = 0x00023902; 
		data_array[1] = 0x000092CB;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00023902; 
		data_array[1] = 0x000008CD;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00023902; 
		data_array[1] = 0x0000016F;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00023902; 
		data_array[1] = 0x000001CD;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00023902; 
		data_array[1] = 0x0000016F;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00023902; 
		data_array[1] = 0x00008ACB;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00023902;
		data_array[1] = 0x000060BF;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00023902; 
		data_array[1] = 0x000040D3;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00023902;
		data_array[1] = 0x0000116F;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00023902; 
		data_array[1] = 0x000001F3;		
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);


		data_array[0] = 0x00023902;
		data_array[1] = 0x00000035;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);
        
		data_array[0] = 0x00023902;
		data_array[1] = 0x000000BF;
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);
		
		data_array[0] = 0x00110500; // Sleep Out
		dsi_set_cmdq(data_array, 1, 1);
		MDELAY(120);
		
		data_array[0] = 0x00290500; // Display On
		dsi_set_cmdq(data_array, 1, 1);
		MDELAY(50);

	#endif
		#if 0
		data_array[0] = 0x00063902; 						 
		data_array[1] = 0x52AA55F0; 
		data_array[2] = 0x00000008;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);
		
		data_array[0] = 0x00053902; 						 
		data_array[1] = 0xA555AAFF; 
		data_array[2] = 0x00000080;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);
	
		
		data_array[0] = 0x00023902; 						 
		data_array[1] = 0x0000176F; 
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);
		
		data_array[0] = 0x00023902; 						 
		data_array[1] = 0x000070F4;  
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);
		 
		data_array[0] = 0x00023902; 						 
		data_array[1] = 0x0000066F; 
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);
		 
		data_array[0] = 0x00023902; 						 
		data_array[1] = 0x000020F7;  
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);
		
		data_array[0] = 0x00023902; 						 
		data_array[1] = 0x000000F7;  
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);
		
		data_array[0] = 0x00063902; 						 
		data_array[1] = 0x050000FC; 
		data_array[2] = 0x00000B08;  
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);
		
		data_array[0] = 0x00023902; 						 
		data_array[1] = 0x00001F6F; 
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);	
		
		data_array[0] = 0x00023902; 						 
		data_array[1] = 0x000090FA; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00023902; 						 
		data_array[1] = 0x0000216F; 
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);
	
		data_array[0] = 0x00023902; 						 
		data_array[1] = 0x000018FA; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00023902; 						 
		data_array[1] = 0x0000026F; 
		dsi_set_cmdq(data_array, 2, 1); 
		MDELAY(1);

		data_array[0] = 0x00023902; 						 
		data_array[1] = 0x000003F8; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00023902; 						 
		data_array[1] = 0x00000A6F; 
		dsi_set_cmdq(data_array, 2, 1); 

		data_array[0] = 0x00023902; 						 
		data_array[1] = 0x000032F8; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00023902; 						 
		data_array[1] = 0x00000B6F; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00023902; 						 
		data_array[1] = 0x000020F8; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00023902; 						 
		data_array[1] = 0x00000F6F; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);
		
		data_array[0] = 0x00023902; 						 
		data_array[1] = 0x000013FA; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00023902; 						 
		data_array[1] = 0x0000026F; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00023902; 						 
		data_array[1] = 0x0000EFF7; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00063902; 						 
		data_array[1] = 0x20C201BD;
		data_array[2] = 0x00000110;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00023902;  //add 						 
		data_array[1] = 0x0000016F; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00023902; 						 
		data_array[1] = 0x0000026F; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00023902; 						 
		data_array[1] = 0x000008B8; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00033902; 						 
		data_array[1] = 0x004474BB; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00063902; 						 
		data_array[1] = 0x52AA55F0;
		data_array[2] = 0x00000108;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00033902; 						 
		data_array[1] = 0x003434B9; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00033902; 						 
		data_array[1] = 0x000078BC; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00033902; 						 
		data_array[1] = 0x000078BD; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00023902; 						 
		data_array[1] = 0x000004B5; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00023902; 						 
		data_array[1] = 0x000000CA; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00023902; 						 
		data_array[1] = 0x00000AB0; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00023902; 						 
		data_array[1] = 0x00000AB1; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00023902; 						 
		data_array[1] = 0x00000C6F; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00023902; 						 
		data_array[1] = 0x0000A7F4; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00023902; 						 
		data_array[1] = 0x00000E6F; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00023902; 						 
		data_array[1] = 0x000043F4; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00023902; 						 
		data_array[1] = 0x000066CE; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00023902; 						 
		data_array[1] = 0x00000CC0; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00063902; 						 
		data_array[1] = 0x52AA55F0;
		data_array[2] = 0x00000608;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00063902; 						 
		data_array[1] = 0x000000D8;
		data_array[2] = 0x00002000;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00063902; 						 
		data_array[1] = 0x000008D9;
		data_array[2] = 0x00000000;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00023902; 						 
		data_array[1] = 0x000000E7; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00023902; 						 
		data_array[1] = 0x0000116F; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00023902; 						 
		data_array[1] = 0x000001F3; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00063902;
		data_array[1] = 0x52AA55F0;
		data_array[2] = 0x00000008;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00023902; 						 
		data_array[1] = 0x000005BC; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00063902;
		data_array[1] = 0x52AA55F0;
		data_array[2] = 0x00000108;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00023902;	 
		data_array[1] = 0x000000CA; 
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00063902; 						 
		data_array[1] = 0x52AA55F0;
		data_array[2] = 0x00000308;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00063902;
		data_array[1] = 0x7D0004B2;
		data_array[2] = 0x0000EB00;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00063902;
		data_array[1] = 0x7D0004B3;
		data_array[2] = 0x0000EB00;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00063902;
		data_array[1] = 0x0A0003B4;
		data_array[2] = 0x00000000;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00063902;
		data_array[1] = 0x0A0003B5;
		data_array[2] = 0x00000000;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00063902;
		data_array[1] = 0x7D0004B6;
		data_array[2] = 0x0000EB00;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00063902;
		data_array[1] = 0x7D0004B7;
		data_array[2] = 0x0000EB00;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00063902;
		data_array[1] = 0x190002B8;
		data_array[2] = 0x00000000;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00063902;
		data_array[1] = 0x190002B9;
		data_array[2] = 0x00000000;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00063902;
		data_array[1] = 0x0F0044BA;
		data_array[2] = 0x0000EB00;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00063902;
		data_array[1] = 0x0F0044BB;
		data_array[2] = 0x0000EB00;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00063902;
		data_array[1] = 0x4F0088BC;
		data_array[2] = 0x00005F01;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00063902;
		data_array[1] = 0x4F0088BD;
		data_array[2] = 0x00005F01;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00063902;
		data_array[1] = 0x52AA55F0;
		data_array[2] = 0x00000508;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00053902;
		data_array[1] = 0x790001D1;
		data_array[2] = 0x00000005;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00053902;
		data_array[1] = 0x790001D2;
		data_array[2] = 0x00000003;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00053902;
		data_array[1] = 0x790001D3;
		data_array[2] = 0x00000001;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00053902;
		data_array[1] = 0x780001D4;
		data_array[2] = 0x00000009;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00033902;
		data_array[1] = 0x003003C8;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00033902;
		data_array[1] = 0x003103C9;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00033902;
		data_array[1] = 0x003007CA;
		dsi_set_cmdq(data_array, 2, 1);

		data_array[0] = 0x00033902;
		data_array[1] = 0x003101CB;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00063902;
		data_array[1] = 0x52AA55F0;
		data_array[2] = 0x00000608;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00063902;
		data_array[1] = 0x000000D8;
		data_array[2] = 0x00002000;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00063902;
		data_array[1] = 0x000008D9;
		data_array[2] = 0x00000000;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00023902;
		data_array[1] = 0x000000E7;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);

		data_array[0] = 0x00063902;
		data_array[1] = 0x52AA55F0;
		data_array[2] = 0x00000000;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00033902;  //add for test flip
		data_array[1] = 0x0000C036;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);
		
		data_array[0] = 0x00110500; // Sleep Out
		dsi_set_cmdq(data_array, 1, 1);
		MDELAY(120);
		
		data_array[0] = 0x00290500; // Display On
		dsi_set_cmdq(data_array, 1, 1);

#if 0 // for test
		data_array[0] = 0x00280500; // Sleep Out
		dsi_set_cmdq(data_array, 1, 1);
		
		data_array[0] = 0x00100500; // Display On
		dsi_set_cmdq(data_array, 1, 1);
		MDELAY(120);

		data_array[0] = 0x00063902;
		data_array[1] = 0x52AA55F0;
		data_array[2] = 0x00000008;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00053902;
		data_array[1] = 0x027887EE;
		data_array[2] = 0x00000040;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(1);

		data_array[0] = 0x00033902;
		data_array[1] = 0x00FF07EF;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(1);
#endif
#endif

}
static struct LCM_setting_table lcm_initialization_setting[] = {
     {0xf0,5,{0x55,0xaa,0x52,0x08,0x01}},
	{0xC0,1,{0x04}},
	{REGFLAG_DELAY, 120, {}},         

	{0xf0,5,{0x55,0xaa,0x52,0x08,0x00}},
	{0xff,5,{0xaa,0x55,0xa5,0x80,0x00}},
	{0xb1,2,{0x68,0x21}},	
	{0xbd,5,{0x01,0xa3,0x20,0x10,0x01}},	
	{0x6f,1,{0x02}},	
	{0xb8,1,{0x08}},	
	{0xbb,2,{0x11,0x11}},	
	{0xbc,2,{0x05,0x05}},	
	//{0xb5,1,{0x50}},	
	{0xc8,1,{0x83}},	
	{0xf0,5,{0x55,0xaa,0x52,0x08,0x01}},
	{0xb0,2,{0x0f,0x0f}},	
	{0xb1,2,{0x0f,0x0f}},
	{0xce,1,{0x66}},
	{0xb5,2,{0x05,0x05}},
	{0xbe,1,{0x45}},
	{0xb3,2,{0x1e,0x1e}},
	{0xb4,2,{0x18,0x18}},
	{0xb9,2,{0x34,0x34}},
	{0xba,2,{0x24,0x24}},
	{0xbc,2,{0xa0,0x03}},	
	{0xbd,2,{0xa0,0x03}},	
	{0xca,1,{0x00}},	
	{0xf0,5,{0x55,0xaa,0x52,0x08,0x02}},
	{0xee,1,{0x01}},	
	{0xb0,16,{0x00,0x00,0x00,0x12,0x00,0x30,0x00,0x4a,0x00,0x5f,0x00,0x84,0x00,0xa4,0x00,0xd8}},
	{0xb1,16,{0x01,0x03,0x01,0x46,0x01,0x80,0x01,0xd9,0x02,0x22,0x02,0x25,0x02,0x65,0x02,0xa8}},
	{0xb2,16,{0x02,0xce,0x02,0xfd,0x03,0x1c,0x03,0x40,0x03,0x55,0x03,0x6a,0x03,0x72,0x03,0x78}},
	{0xb3,4,{0x03,0x80,0x03,0xff}},	
	{0x6f,1,{0x02}},	
	{0xf8,1,{0x44}},	
	{0x6f,1,{0x0f}},	
	{0xfa,1,{0x14}},	
	{0x6f,1,{0x02}},	
	{0xf7,1,{0xef}},	
	{0x6f,1,{0x0a}},	
	{0xf4,1,{0xd8}},	
	{0x6f,1,{0x0e}},	
	{0xf4,1,{0x44}},	
	{0x6f,1,{0x17}},	
	{0xf4,1,{0x60}},	
	{0x6f,1,{0x01}},	
	{0xf9,1,{0x46}},	
	{0xf0,5,{0x55,0xaa,0x52,0x08,0x06}},
	{0xb0,2,{0x2e,0x2e}},	
	{0xb1,2,{0x2e,0x2e}},
	{0xb2,2,{0x2e,0x2e}},
	{0xb3,2,{0x2e,0x09}},
	{0xb4,2,{0x0b,0x11}},
	{0xb5,2,{0x17,0x19}},
	{0xb6,2,{0x23,0x1d}},
	{0xb7,2,{0x25,0x1f}},
	{0xb8,2,{0x01,0x03}},
	{0xb9,2,{0x2e,0x2e}},
	{0xba,2,{0x2e,0x2e}},
	{0xbb,2,{0x02,0x00}},
	{0xbc,2,{0x1e,0x24}},
	{0xbd,2,{0x1c,0x22}},
	{0xbe,2,{0x18,0x16}},
	{0xbf,2,{0x10,0x0a}},
	{0xc0,2,{0x08,0x2e}},	
	{0xc1,2,{0x2e,0x2e}},	
	{0xc2,2,{0x2e,0x2e}},	
	{0xc3,2,{0x2e,0x2e}},	
	{0xe5,2,{0x13,0x12}},	
	{0xc4,2,{0x2e,0x2e}},	
	{0xc5,2,{0x2e,0x2e}},	
	{0xc6,2,{0x2e,0x2e}},	
	{0xc7,2,{0x2e,0x02}},		
	{0xc8,2,{0x00,0x18}},	
	{0xc9,2,{0x12,0x10}},	
	{0xca,2,{0x24,0x1e}},	
	{0xcb,2,{0x22,0x1c}},	
	{0xcc,2,{0x0a,0x08}},	
	{0xcd,2,{0x2e,0x2e}},	
	{0xce,2,{0x2e,0x2e}},	
	{0xcf,2,{0x09,0x0b}},	
	{0xd0,2,{0x1d,0x23}},	
	{0xd1,2,{0x1f,0x25}},	
	{0xd2,2,{0x11,0x13}},	
	{0xd3,2,{0x19,0x01}},
	{0xd4,2,{0x03,0x2e}},	
	{0xd5,2,{0x2e,0x2e}},	
	{0xd6,2,{0x2e,0x2e}},	
	{0xd7,2,{0x2e,0x2e}},
	{0xe6,2,{0x16,0x17}},
	{0xd8,5,{0x00,0x00,0x00,0x00,0x00}},
	{0xd9,5,{0x00,0x00,0x00,0x00,0x00}},
	{0xe7,1,{0x00}},	
	{0xf0,5,{0x55,0xaa,0x52,0x08,0x05}},
	{0xed,1,{0x30}},	
	{0xf0,5,{0x55,0xaa,0x52,0x08,0x03}},
	{0xb0,2,{0x20,0x00}},	
	{0xb1,2,{0x20,0x00}},
	{0xf0,5,{0x55,0xaa,0x52,0x08,0x05}},
	{0xb0,2,{0x17,0x06}},	
	{0xb8,1,{0x00}},	
	{0xbd,5,{0x0f,0x03,0x03,0x00,0x03}},
	{0xb1,2,{0x17,0x06}},	
	{0xb9,2,{0x00,0x03}},	
	{0xb2,2,{0x17,0x06}},	
	{0xba,2,{0x00,0x03}},	
	{0xb3,2,{0x17,0x06}},
	{0xbb,2,{0x00,0x00}},
	{0xb4,2,{0x17,0x06}},
	{0xb5,2,{0x17,0x06}},
	{0xb6,2,{0x17,0x06}},
	{0xb7,2,{0x17,0x06}},
	{0xbc,2,{0x00,0x01}},
	{0xe5,1,{0x06}},
	{0xe6,1,{0x06}},
	{0xe7,1,{0x06}},
	{0xe8,1,{0x06}},
	{0xe9,1,{0x0a}},
	{0xea,1,{0x06}},
	{0xeb,1,{0x06}},
	{0xec,1,{0x06}},
	{0xf0,5,{0x55,0xaa,0x52,0x08,0x05}},
	{0xc0,1,{0x07}},
	{0xc1,1,{0x05}},
	{0xf0,5,{0x55,0xaa,0x52,0x08,0x03}},
	{0xb2,5,{0x04,0x00,0x52,0x01,0x51}},
	{0xb3,5,{0x04,0x00,0x52,0x01,0x51}},
	{0xf0,5,{0x55,0xaa,0x52,0x08,0x05}},
	{0xc4,1,{0x82}},
	{0xc5,1,{0x80}},	
	{0xf0,5,{0x55,0xaa,0x52,0x08,0x03}},
	{0xb6,5,{0x04,0x00,0x52,0x01,0x51}},
	{0xb7,5,{0x04,0x00,0x52,0x01,0x51}},
	{0xf0,5,{0x55,0xaa,0x52,0x08,0x05}},
	{0xc8,2,{0x03,0x20}},
	{0xc9,2,{0x01,0x21}},
	{0xca,2,{0x03,0x20}},
	{0xcb,2,{0x07,0x20}},
	{0xf0,5,{0x55,0xaa,0x52,0x08,0x03}},
	{0xc4,1,{0x60}},
	{0xc5,1,{0x40}},	
	{0xf0,5,{0x55,0xaa,0x52,0x08,0x03}},
	{0xba,5,{0x44,0x00,0x60,0x01,0x72}},
	{0xbb,5,{0x44,0x00,0x60,0x01,0x72}},
	{0xbc,5,{0x53,0x00,0x03,0x00,0x48}},
	{0xbd,5,{0x53,0x00,0x03,0x00,0x48}},
	{0xf0,5,{0x55,0xaa,0x52,0x08,0x05}},
	{0xd1,11,{0x03,0x05,0x00,0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},	
	{0xd2,11,{0x03,0x05,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},	
	{0xd3,11,{0x00,0x05,0x04,0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},	
	{0xd4,11,{0x00,0x05,0x04,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},	
	{0xf0,5,{0x55,0xaa,0x52,0x08,0x03}},
	{0xc4,1,{0x40}},
	{0xc5,1,{0x40}},	
	{0xf0,5,{0x55,0xaa,0x52,0x08,0x00}},
	{0x6f,1,{0x08}},
	{0xf2,1,{0x70}},
	{0x6f,1,{0x02}},
	{0xb8,1,{0x08}},
	{0x6f,1,{0x07}},
	{0xf4,1,{0xf0}},
	{0xf0,5,{0x55,0xaa,0x52,0x08,0x01}},
	{0xcb,1,{0x92}},
	{0xcd,1,{0x08}},
	{0x6f,1,{0x01}},
	{0xcd,1,{0x01}},
	{0x6f,1,{0x01}},
	{0xcb,1,{0x8a}},
	{0xbf,1,{0x60}},
	{0xd3,1,{0x40}},
	{0x6f,1,{0x11}},
	{0xf3,1,{0x01}},
	
	{0x35,1,{0x00}},
	{0xbf,1,{0x00}},
	{0x11,1,{0x00}},
	{REGFLAG_DELAY, 200, {}},  
	{0xbf,1,{0x00}},
	{0x29,1,{0x00}},
	{REGFLAG_DELAY, 120, {}},  
	
	{REGFLAG_END_OF_TABLE, 0x00, {}}	
};

static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
	{0x11, 0, {0x00}},
    {REGFLAG_DELAY, 120, {}},

    // Display ON
	{0x29, 0, {0x00}},
	{REGFLAG_DELAY, 180, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_sleep_mode_in_setting[] = {
	// Display off sequence
	{0x28, 0, {0x00}},
	{REGFLAG_DELAY, 120, {}},

    // Sleep Mode On
	{0x10, 0, {0x00}},
	{REGFLAG_DELAY, 120, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_compare_id_setting[] = {
	// Display off sequence
	{0xF0,	5,	{0x55, 0xaa, 0x52,0x08,0x00}},
	{REGFLAG_DELAY, 10, {}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

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
			// MDELAY(2);
       	}
    }
	
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
	params->dbi.te_mode 				= LCM_DBI_TE_MODE_DISABLED;//LCM_DBI_TE_MODE_VSYNC_ONLY;
	params->dbi.data_format.format = 3;

	//params->dsi.mode   = SYNC_PULSE_VDO_MODE;
	params->dsi.mode   = BURST_VDO_MODE;
	//params->dsi.mode   = SYNC_EVENT_VDO_MODE;
	
	// DSI
	/* Command mode setting */
	params->dbi.io_driving_current      = LCM_DRIVING_CURRENT_6575_16MA;
	params->dsi.LANE_NUM				= LCM_FOUR_LANE;
	//params->dsi.LANE_NUM				= LCM_THREE_LANE;
	//The following defined the fomat for data coming from LCD engine.
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

	// Video mode setting		
	params->dsi.intermediat_buffer_num = 0;

	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

#if 0
	params->dsi.vertical_sync_active				= 0x03;
	params->dsi.vertical_backporch					= 0x04;
	params->dsi.vertical_frontporch					= 0x08;
	params->dsi.vertical_active_line				= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active				= 0x0B;
	params->dsi.horizontal_backporch				= 0x40;
	params->dsi.horizontal_frontporch				= 0x40;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
#else
	params->dsi.vertical_sync_active				= 8;
	params->dsi.vertical_backporch					= 16;
	params->dsi.vertical_frontporch					= 15;
	params->dsi.vertical_active_line				= FRAME_HEIGHT;

		params->dsi.horizontal_sync_active				= 5;
		params->dsi.horizontal_backporch				= 80;
		params->dsi.horizontal_frontporch				= 80;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

#endif
	// Bit rate calculation
	params->dsi.pll_div1=0;		// div1=0,1,2,3;div1_real=1,2,4,4
	params->dsi.pll_div2=1;		// div2=0,1,2,3;div1_real=1,2,4,4
	params->dsi.fbk_div =12;	//15	// fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)		
}
extern void DSI_clk_HS_mode(bool enter);
static void lcm_init(void)
{
    DSI_clk_HS_mode(0);
	mt_set_gpio_mode(GPIO_AVEE_EN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_AVEE_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_AVEE_EN, GPIO_OUT_ONE);
	mt_set_gpio_mode(GPIO_AVDD_EN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_AVDD_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_AVDD_EN, GPIO_OUT_ONE);
	// Enable EN_PWR for NT50198 PMIC
#ifdef BUILD_LK
    upmu_set_rg_vgp6_vosel(6);
    upmu_set_rg_vgp6_en(1);
#else
    hwPowerOn(MT65XX_POWER_LDO_VGP6, VOL_2800, "LCM");
#endif
    mt_set_gpio_mode(GPIO_LCD_RST_EN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LCD_RST_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
	MDELAY(10);
	mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ZERO);
	MDELAY(10);
	mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
	MDELAY(120);
    DSI_clk_HS_mode(1);
    MDELAY(10);
   // init_lcm_registers();
    push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}


 void lcm_suspend(void)
{
	unsigned int data_array[16];

	data_array[0] = 0x00280500;	// Display Off
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(120);

	data_array[0] = 0x00100500;	// Sleep In
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(120);

#if 1
	mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
	MDELAY(50);
	mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ZERO);
	MDELAY(50);
	mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
	MDELAY(120);

#ifdef BUILD_LK
	upmu_set_rg_vgp6_en(0);
#else
	hwPowerDown(MT65XX_POWER_LDO_VGP6, "LCM");
#endif
	mt_set_gpio_out(GPIO_AVEE_EN, GPIO_OUT_ZERO);
	mt_set_gpio_out(GPIO_AVDD_EN, GPIO_OUT_ZERO);
#endif
	
}


static void lcm_resume(void)
{
#ifndef BUILD_LK
	lcm_init();
#endif
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
	unsigned int   id=0;
	unsigned char buffer[2];
	unsigned int array[16];  

	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(1);
	
	SET_RESET_PIN(1);
	MDELAY(20); 

//*************Enable CMD2 Page1  *******************//
	array[0]=0x00063902;
	array[1]=0x52AA55F0;
	array[2]=0x00000108;
	dsi_set_cmdq(array, 3, 1);

	array[0] = 0x00023700;// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);
	
	read_reg_v2(0xC5, buffer, 2);

	id_high = buffer[0];            //should be 0x96
	id_low  = buffer[1];            //should be 0x01
	id      = (id_high<<8)|id_low;  //should be 0x9601

    #ifdef BUILD_LK
		printf("%s, LK nt35521 debug: nt35521 id = 0x%08x\n", __func__, id);
    #else
		printk("%s, kernel nt35521 horse debug: nt35521 id = 0x%08x\n", __func__, id);
    #endif

    if(id == LCM_ID_NT35521)
    	return 1;
    else
        return 0;


}


static unsigned int lcm_esd_check(void)
{
  #ifndef BUILD_LK
	char  buffer[3];
	int   array[4];

	if(lcm_esd_test)
	{
		lcm_esd_test = FALSE;
		return TRUE;
	}

	array[0] = 0x00013700;
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0x36, buffer, 1);
	if(buffer[0]==0x90)
	{
		return FALSE;
	}
	else
	{			 
		return TRUE;
	}
 #endif

}

static unsigned int lcm_esd_recover(void)
{
	lcm_init();
	lcm_resume();

	return TRUE;
}



LCM_DRIVER nt35521_auo60_ykl_hd_lcm_drv = 
{
    .name			= "nt35521_auo60_ykl_hd",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id     = lcm_compare_id,
	//.esd_check = lcm_esd_check,
	//.esd_recover = lcm_esd_recover,
#if (LCM_DSI_CMD_MODE)
    .update         = lcm_update,
#endif
    };
