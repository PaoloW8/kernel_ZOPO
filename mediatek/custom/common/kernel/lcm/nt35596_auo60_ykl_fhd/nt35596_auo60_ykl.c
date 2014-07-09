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

#define REGFLAG_DELAY             							0xEE
#define REGFLAG_END_OF_TABLE      							0xEF   // END OF REGISTERS MARKER

#define LCM_LG_TRULY_ID_NT35596 (0x96)
#define GPIO_LCD_RST_EN    (GPIO131)
#define GPIO_AVEE_EN      (GPIO132)
#define GPIO_AVDD_EN      (GPIO133)

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))

static void lcm_compare_id(void);

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

static struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};

static struct LCM_setting_table lcm_initialization_setting[] = {
{0xFF,1,{0xEE}},                                               
{0xFB,1,{0x01}},                                               
{0x24,1,{0x4F}},                                               
{0x38,1,{0xC8}},                                               
{0x39,1,{0x27}},//Modified VP_HSSI Voltage                       
{0x1E,1,{0x77}},//Modified VDD Voltage                           
{0x1D,1,{0x0F}},                                               
{0x7E,1,{0x71}},//Modified VDD Voltage                           
{0x7C,1,{0x31}},//Source Pull GND                                
                                               
{0xFF,1,{0x01}},                                               
{0xFB,1,{0x01}},                                               
{0x00,1,{0x01}},                                               
{0x01,1,{0x55}},                                               
{0x02,1,{0x40}},                                               
{0x05,1,{0x50}},                                               
{0x06,1,{0x4A}},//Modified VGH Level                             
{0x07,1,{0x3C}},//H497==>29                                      
{0x08,1,{0x0C}},                                               
{0x0B,1,{0x9B}},//H497==>87                                      
{0x0C,1,{0x9B}},//H497==>87                                      
{0x0E,1,{0xB6}},//H497==>B0                                      
{0x0F,1,{0xB5}},//H497==>B3                                      
{0x11,1,{0x28}},//VCOM DC,no need to issue for OTP LCM H497==> 10
{0x12,1,{0x28}},//H497==>10                                      
{0x13,1,{0x03}},//VCOM for  Forward and Backward Scan            
{0x14,1,{0x4A}},                                               
{0x15,1,{0x18}},//H497==>12                                      
{0x16,1,{0x18}},//H497==>12                                      
{0x18,1,{0x00}},                                               
{0x19,1,{0x77}},                                               
{0x1A,1,{0x55}},                                               
{0x1B,1,{0x13}},                                               
{0x1C,1,{0x00}},                                               
{0x1D,1,{0x00}},                                               
{0x1E,1,{0x00}},                                               
{0x1F,1,{0x00}},                                               
{0x58,1,{0x82}},                                               
{0x59,1,{0x02}},                                               
{0x5A,1,{0x02}},                                               
{0x5B,1,{0x02}},                                               
{0x5C,1,{0x82}},                                               
{0x5D,1,{0x82}},                                               
{0x5E,1,{0x02}},                                               
{0x5F,1,{0x02}},                                               
{0x66,1,{0x00}},                                               
{0x6D,1,{0x66}},                                               
{0x72,1,{0x31}},                                               
                                               
{0xFF,1,{0x05}},                                               
{0xFB,1,{0x01}},                                               
{0x00,1,{0x01}},                                               
{0x01,1,{0x0B}},                                               
{0x02,1,{0x0C}},                                               
{0x03,1,{0x09}},                                               
{0x04,1,{0x0A}},                                               
{0x05,1,{0x00}},                                               
{0x06,1,{0x0F}},                                               
{0x07,1,{0x10}},                                               
{0x08,1,{0x00}},                                               
{0x09,1,{0x00}},                                               
{0x0A,1,{0x00}},                                               
{0x0B,1,{0x00}},                                               
{0x0C,1,{0x00}},                                               
{0x0D,1,{0x13}},                                               
{0x0E,1,{0x15}},                                               
{0x0F,1,{0x17}},                                               
{0x10,1,{0x01}},                                               
{0x11,1,{0x0B}},                                               
{0x12,1,{0x0C}},                                               
{0x13,1,{0x09}},                                               
{0x14,1,{0x0A}},                                               
{0x15,1,{0x00}},                                               
{0x16,1,{0x0F}},                                               
{0x17,1,{0x10}},                                               
{0x18,1,{0x00}},                                               
{0x19,1,{0x00}},                                               
{0x1A,1,{0x00}},                                               
{0x1B,1,{0x00}},                                               
{0x1C,1,{0x00}},                                               
{0x1D,1,{0x13}},                                               
{0x1E,1,{0x15}},                                               
{0x1F,1,{0x17}},                                               
{0x20,1,{0x00}},                                               
{0x21,1,{0x03}},                                               
{0x22,1,{0x01}},                                               
{0x23,1,{0x40}},                                               
{0x24,1,{0x40}},                                               
{0x25,1,{0xED}},                                               
{0x29,1,{0x58}},                                               
{0x2A,1,{0x12}},                                               
{0x2B,1,{0x01}},                                               
{0x4B,1,{0x04}},                                               
{0x4C,1,{0x12}},                                               
{0x4D,1,{0x02}},                                               
{0x4E,1,{0x20}},                                               
{0x4F,1,{0x20}},                                               
{0x50,1,{0x02}},                                               
{0x51,1,{0x61}},                                               
{0x52,1,{0x01}},                                               
{0x53,1,{0x63}},                                               
{0x54,1,{0x75}},//H497==>77                                      
{0x55,1,{0xED}},                                               
{0x5B,1,{0x00}},                                               
{0x5C,1,{0x00}},                                               
{0x5D,1,{0x00}},                                               
{0x5E,1,{0x00}},                                               
{0x5F,1,{0x15}},                                               
{0x60,1,{0x75}},                                               
{0x61,1,{0x00}},                                               
{0x62,1,{0x00}},                                               
{0x63,1,{0x00}},                                               
{0x64,1,{0x00}},                                               
{0x67,1,{0x00}},                                               
{0x68,1,{0x04}},                                               
{0x6C,1,{0x40}},                                               
{0x75,1,{0x01}},                                               
{0x76,1,{0x01}},                                               
{0x7A,1,{0x80}},                                               
{0x7B,1,{0xA3}},                                               
{0x7C,1,{0xD8}},                                               
{0x7D,1,{0x60}},                                               
{0x7F,1,{0x15}},                                               
{0x80,1,{0x00}},                                               
{0x83,1,{0x00}},                                               
{0x93,1,{0x08}},                                               
{0x94,1,{0x0A}},//H497==>08                                      
{0x8A,1,{0x00}},                                               
{0x9B,1,{0x0F}},                                               
{0x9D,1,{0xB0}},                                               
 /*                                              
{0xFF,1,{0x01}},                                               
{0xFB,1,{0x01}},                                               
{0x75,1,{0x00}},//Gamma R+                                             
{0x76,1,{0x2E}},                                               
{0x77,1,{0x00}},                                               
{0x78,1,{0x5F}},                                               
{0x79,1,{0x00}},                                               
{0x7A,1,{0x8B}},                                               
{0x7B,1,{0x00}},                                               
{0x7C,1,{0xA8}},                                               
{0x7D,1,{0x00}},                                               
{0x7E,1,{0xBF}},                                               
{0x7F,1,{0x00}},                                               
{0x80,1,{0xD3}},                                               
{0x81,1,{0x00}},                                               
{0x82,1,{0xE3}},                                               
{0x83,1,{0x00}},                                               
{0x84,1,{0xF2}},                                               
{0x85,1,{0x01}},                                               
{0x86,1,{0x00}},                                               
{0x87,1,{0x01}},                                               
{0x88,1,{0x2C}},                                               
{0x89,1,{0x01}},                                               
{0x8A,1,{0x4F}},                                               
{0x8B,1,{0x01}},                                               
{0x8C,1,{0x86}},                                               
{0x8D,1,{0x01}},                                               
{0x8E,1,{0xB1}},                                               
{0x8F,1,{0x01}},                                               
{0x90,1,{0xF4}},                                               
{0x91,1,{0x02}},                                               
{0x92,1,{0x2A}},                                               
{0x93,1,{0x02}},                                               
{0x94,1,{0x2B}},                                               
{0x95,1,{0x02}},                                               
{0x96,1,{0x5E}},                                               
{0x97,1,{0x02}},                                               
{0x98,1,{0x97}},                                               
{0x99,1,{0x02}},                                               
{0x9A,1,{0xBC}},                                               
{0x9B,1,{0x02}},                                               
{0x9C,1,{0xEE}},                                               
{0x9D,1,{0x03}},                                               
{0x9E,1,{0x11}},                                               
{0x9F,1,{0x03}},                                               
{0xA0,1,{0x3E}},                                               
{0xA2,1,{0x03}},                                               
{0xA3,1,{0x4B}},                                               
{0xA4,1,{0x03}},                                               
{0xA5,1,{0x5A}},                                               
{0xA6,1,{0x03}},                                               
{0xA7,1,{0x6A}},                                               
{0xA9,1,{0x03}},                                               
{0xAA,1,{0x7C}},                                               
{0xAB,1,{0x03}},                                               
{0xAC,1,{0x8F}},                                               
{0xAD,1,{0x03}},                                               
{0xAE,1,{0xA3}},                                               
{0xAF,1,{0x03}},                                               
{0xB0,1,{0xB4}},                                               
{0xB1,1,{0x03}},                                               
{0xB2,1,{0xFF}},                                               
{0xB3,1,{0x00}},//Gamma R-                                               
{0xB4,1,{0x2E}},                                               
{0xB5,1,{0x00}},                                               
{0xB6,1,{0x5F}},                                               
{0xB7,1,{0x00}},                                               
{0xB8,1,{0x8B}},                                               
{0xB9,1,{0x00}},                                               
{0xBA,1,{0xA8}},                                               
{0xBB,1,{0x00}},                                               
{0xBC,1,{0xBF}},                                               
{0xBD,1,{0x00}},                                               
{0xBE,1,{0xD3}},                                               
{0xBF,1,{0x00}},                                               
{0xC0,1,{0xE3}},                                               
{0xC1,1,{0x00}},                                               
{0xC2,1,{0xF2}},                                               
{0xC3,1,{0x01}},                                               
{0xC4,1,{0x00}},                                               
{0xC5,1,{0x01}},                                               
{0xC6,1,{0x2C}},                                               
{0xC7,1,{0x01}},                                               
{0xC8,1,{0x4F}},                                               
{0xC9,1,{0x01}},                                               
{0xCA,1,{0x86}},                                               
{0xCB,1,{0x01}},                                               
{0xCC,1,{0xB1}},                                               
{0xCD,1,{0x01}},                                               
{0xCE,1,{0xF4}},                                               
{0xCF,1,{0x02}},                                               
{0xD0,1,{0x2A}},                                               
{0xD1,1,{0x02}},                                               
{0xD2,1,{0x2B}},                                               
{0xD3,1,{0x02}},                                               
{0xD4,1,{0x5E}},                                               
{0xD5,1,{0x02}},                                               
{0xD6,1,{0x97}},                                               
{0xD7,1,{0x02}},                                               
{0xD8,1,{0xBC}},                                               
{0xD9,1,{0x02}},                                               
{0xDA,1,{0xEE}},                                               
{0xDB,1,{0x03}},                                               
{0xDC,1,{0x11}},                                               
{0xDD,1,{0x03}},                                               
{0xDE,1,{0x3E}},                                               
{0xDF,1,{0x03}},                                               
{0xE0,1,{0x4B}},                                               
{0xE1,1,{0x03}},                                               
{0xE2,1,{0x5A}},                                               
{0xE3,1,{0x03}},                                               
{0xE4,1,{0x6A}},                                               
{0xE5,1,{0x03}},                                               
{0xE6,1,{0x7C}},                                               
{0xE7,1,{0x03}},                                               
{0xE8,1,{0x8F}},                                               
{0xE9,1,{0x03}},                                               
{0xEA,1,{0xA3}},                                               
{0xEB,1,{0x03}},                                               
{0xEC,1,{0xB4}},                                               
{0xED,1,{0x03}},                                               
{0xEE,1,{0xFF}},                                               
{0xEF,1,{0x00}},//Gamma G+                                               
{0xF0,1,{0x2E}},                                               
{0xF1,1,{0x00}},                                               
{0xF2,1,{0x5F}},                                               
{0xF3,1,{0x00}},                                               
{0xF4,1,{0x8B}},                                               
{0xF5,1,{0x00}},                                               
{0xF6,1,{0xA8}},                                               
{0xF7,1,{0x00}},                                               
{0xF8,1,{0xBF}},                                               
{0xF9,1,{0x00}},                                               
{0xFA,1,{0xD3}},                                               
{0xFF,1,{0x02}},                                               
{0xFB,1,{0x01}},                                               
{0x00,1,{0x00}},                                               
{0x01,1,{0xE3}},                                               
{0x02,1,{0x00}},                                               
{0x03,1,{0xF2}},                                               
{0x04,1,{0x01}},                                               
{0x05,1,{0x00}},                                               
{0x06,1,{0x01}},                                               
{0x07,1,{0x2C}},                                               
{0x08,1,{0x01}},                                               
{0x09,1,{0x4F}},                                               
{0x0A,1,{0x01}},                                               
{0x0B,1,{0x86}},                                               
{0x0C,1,{0x01}},                                               
{0x0D,1,{0xB1}},                                               
{0x0E,1,{0x01}},                                               
{0x0F,1,{0xF4}},                                               
{0x10,1,{0x02}},                                               
{0x11,1,{0x2A}},                                               
{0x12,1,{0x02}},                                               
{0x13,1,{0x2B}},                                               
{0x14,1,{0x02}},                                               
{0x15,1,{0x5E}},                                               
{0x16,1,{0x02}},                                               
{0x17,1,{0x97}},                                               
{0x18,1,{0x02}},                                               
{0x19,1,{0xBC}},                                               
{0x1A,1,{0x02}},                                               
{0x1B,1,{0xEE}},                                               
{0x1C,1,{0x03}},                                               
{0x1D,1,{0x11}},                                               
{0x1E,1,{0x03}},                                               
{0x1F,1,{0x3E}},                                               
{0x20,1,{0x03}},                                               
{0x21,1,{0x4B}},                                               
{0x22,1,{0x03}},                                               
{0x23,1,{0x5A}},                                               
{0x24,1,{0x03}},                                               
{0x25,1,{0x6A}},                                               
{0x26,1,{0x03}},                                               
{0x27,1,{0x7C}},                                               
{0x28,1,{0x03}},                                               
{0x29,1,{0x8F}},                                               
{0x2A,1,{0x03}},                                               
{0x2B,1,{0xA3}},                                               
{0x2D,1,{0x03}},                                               
{0x2F,1,{0xB4}},                                               
{0x30,1,{0x03}},                                               
{0x31,1,{0xFF}},                                               
{0x32,1,{0x00}},//Gamma G-                                               
{0x33,1,{0x2E}},                                               
{0x34,1,{0x00}},                                               
{0x35,1,{0x5F}},                                               
{0x36,1,{0x00}},                                               
{0x37,1,{0x8B}},                                               
{0x38,1,{0x00}},                                               
{0x39,1,{0xA8}},                                               
{0x3A,1,{0x00}},                                               
{0x3B,1,{0xBF}},                                               
{0x3D,1,{0x00}},                                               
{0x3F,1,{0xD3}},                                               
{0x40,1,{0x00}},                                               
{0x41,1,{0xE3}},                                               
{0x42,1,{0x00}},                                               
{0x43,1,{0xF2}},                                               
{0x44,1,{0x01}},                                               
{0x45,1,{0x00}},                                               
{0x46,1,{0x01}},                                               
{0x47,1,{0x2C}},                                               
{0x48,1,{0x01}},                                               
{0x49,1,{0x4F}},                                               
{0x4A,1,{0x01}},                                               
{0x4B,1,{0x86}},                                               
{0x4C,1,{0x01}},                                               
{0x4D,1,{0xB1}},                                               
{0x4E,1,{0x01}},                                               
{0x4F,1,{0xF4}},                                               
{0x50,1,{0x02}},                                               
{0x51,1,{0x2A}},                                               
{0x52,1,{0x02}},                                               
{0x53,1,{0x2B}},                                               
{0x54,1,{0x02}},                                               
{0x55,1,{0x5E}},                                               
{0x56,1,{0x02}},                                               
{0x58,1,{0x97}},                                               
{0x59,1,{0x02}},                                               
{0x5A,1,{0xBC}},                                               
{0x5B,1,{0x02}},                                               
{0x5C,1,{0xEE}},                                               
{0x5D,1,{0x03}},                                               
{0x5E,1,{0x11}},                                               
{0x5F,1,{0x03}},                                               
{0x60,1,{0x3E}},                                               
{0x61,1,{0x03}},                                               
{0x62,1,{0x4B}},                                               
{0x63,1,{0x03}},                                               
{0x64,1,{0x5A}},                                               
{0x65,1,{0x03}},                                               
{0x66,1,{0x6A}},                                               
{0x67,1,{0x03}},                                               
{0x68,1,{0x7C}},                                               
{0x69,1,{0x03}},                                               
{0x6A,1,{0x8F}},                                               
{0x6B,1,{0x03}},                                               
{0x6C,1,{0xA3}},                                               
{0x6D,1,{0x03}},                                               
{0x6E,1,{0xB4}},                                               
{0x6F,1,{0x03}},                                               
{0x70,1,{0xFF}},                                               
{0x71,1,{0x00}},//Gamma B+                                               
{0x72,1,{0x2E}},                                               
{0x73,1,{0x00}},                                               
{0x74,1,{0x45}},                                               
{0x75,1,{0x00}},                                               
{0x76,1,{0x88}},                                               
{0x77,1,{0x00}},                                               
{0x78,1,{0xA4}},                                               
{0x79,1,{0x00}},                                               
{0x7A,1,{0xBB}},                                               
{0x7B,1,{0x00}},                                               
{0x7C,1,{0xCE}},                                               
{0x7D,1,{0x00}},                                               
{0x7E,1,{0xDE}},                                               
{0x7F,1,{0x00}},                                               
{0x80,1,{0xED}},                                               
{0x81,1,{0x00}},                                               
{0x82,1,{0xFA}},                                               
{0x83,1,{0x01}},                                               
{0x84,1,{0x26}},                                               
{0x85,1,{0x01}},                                               
{0x86,1,{0x49}},                                               
{0x87,1,{0x01}},                                               
{0x88,1,{0x7F}},                                               
{0x89,1,{0x01}},                                               
{0x8A,1,{0xAA}},                                               
{0x8B,1,{0x01}},                                               
{0x8C,1,{0xED}},                                               
{0x8D,1,{0x02}},                                               
{0x8E,1,{0x23}},                                               
{0x8F,1,{0x02}},                                               
{0x90,1,{0x25}},                                               
{0x91,1,{0x02}},                                               
{0x92,1,{0x57}},                                               
{0x93,1,{0x02}},                                               
{0x94,1,{0x8E}},                                               
{0x95,1,{0x02}},                                               
{0x96,1,{0xB2}},                                               
{0x97,1,{0x02}},                                               
{0x98,1,{0xE3}},                                               
{0x99,1,{0x03}},                                               
{0x9A,1,{0x05}},                                               
{0x9B,1,{0x03}},                                               
{0x9C,1,{0x31}},                                               
{0x9D,1,{0x03}},                                               
{0x9E,1,{0x3E}},                                               
{0x9F,1,{0x03}},                                               
{0xA0,1,{0x4E}},                                               
{0xA2,1,{0x03}},                                               
{0xA3,1,{0x5F}},                                               
{0xA4,1,{0x03}},                                               
{0xA5,1,{0x71}},                                               
{0xA6,1,{0x03}},                                               
{0xA7,1,{0x88}},                                               
{0xA9,1,{0x03}},                                               
{0xAA,1,{0xA4}},                                               
{0xAB,1,{0x03}},                                               
{0xAC,1,{0xBA}},                                               
{0xAD,1,{0x03}},                                               
{0xAE,1,{0xFF}},                                               
{0xAF,1,{0x00}},//Gamma B-                                               
{0xB0,1,{0x2E}},                                               
{0xB1,1,{0x00}},                                               
{0xB2,1,{0x45}},                                               
{0xB3,1,{0x00}},                                               
{0xB4,1,{0x88}},                                               
{0xB5,1,{0x00}},                                               
{0xB6,1,{0xA4}},                                               
{0xB7,1,{0x00}},                                               
{0xB8,1,{0xBB}},                                               
{0xB9,1,{0x00}},                                               
{0xBA,1,{0xCE}},                                               
{0xBB,1,{0x00}},                                               
{0xBC,1,{0xDE}},                                               
{0xBD,1,{0x00}},                                               
{0xBE,1,{0xED}},                                               
{0xBF,1,{0x00}},                                               
{0xC0,1,{0xFA}},                                               
{0xC1,1,{0x01}},                                               
{0xC2,1,{0x26}},                                               
{0xC3,1,{0x01}},                                               
{0xC4,1,{0x49}},                                               
{0xC5,1,{0x01}},                                               
{0xC6,1,{0x7F}},                                               
{0xC7,1,{0x01}},                                               
{0xC8,1,{0xAA}},                                               
{0xC9,1,{0x01}},                                               
{0xCA,1,{0xED}},                                               
{0xCB,1,{0x02}},                                               
{0xCC,1,{0x23}},                                               
{0xCD,1,{0x02}},                                               
{0xCE,1,{0x25}},                                               
{0xCF,1,{0x02}},                                               
{0xD0,1,{0x57}},                                               
{0xD1,1,{0x02}},                                               
{0xD2,1,{0x8E}},                                               
{0xD3,1,{0x02}},                                               
{0xD4,1,{0xB2}},                                               
{0xD5,1,{0x02}},                                               
{0xD6,1,{0xE3}},                                               
{0xD7,1,{0x03}},                                               
{0xD8,1,{0x05}},                                               
{0xD9,1,{0x03}},                                               
{0xDA,1,{0x31}},                                               
{0xDB,1,{0x03}},                                               
{0xDC,1,{0x3E}},                                               
{0xDD,1,{0x03}},                                               
{0xDE,1,{0x4E}},                                               
{0xDF,1,{0x03}},                                               
{0xE0,1,{0x5F}},                                               
{0xE1,1,{0x03}},                                               
{0xE2,1,{0x71}},                                               
{0xE3,1,{0x03}},                                               
{0xE4,1,{0x88}},                                               
{0xE5,1,{0x03}},                                               
{0xE6,1,{0xA4}},                                               
{0xE7,1,{0x03}},                                               
{0xE8,1,{0xBA}},                                               
{0xE9,1,{0x03}},                                               
{0xEA,1,{0xFF}},                                               
 */                                              
                                               
{0xFF,1,{0x01}},                                               
{0xFB,1,{0x01}},                                               
{0xFF,1,{0x02}},                                               
{0xFB,1,{0x01}},                                               
{0xFF,1,{0x04}},                                               
{0xFB,1,{0x01}},                                               
{0xFF,1,{0x00}},                                               
                                               
{0xD3,1,{0x05}},//05 04
{0xD4,1,{0x04}},

{0xFF,1,{0x00}},                                               
{0x35,1,{0x00}}, 
{0x11,1,{0x00}},//SLEEP OUT
{REGFLAG_DELAY,120,{}},                                                              				                                                                                
{0x29,1,{0x00}},//Display ON 
{REGFLAG_DELAY,100,{}},	

};

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
		params->dsi.vertical_backporch					= 3;//2 8
		params->dsi.vertical_frontporch					= 4;//2 8
		params->dsi.vertical_active_line				= FRAME_HEIGHT;

		params->dsi.horizontal_sync_active			= 4;//12 8
		params->dsi.horizontal_backporch				= 50;//72 40
		params->dsi.horizontal_frontporch				= 50;//120 140
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
		//params->dsi.pll_select=1;	//0: MIPI_PLL; 1: LVDS_PLL
		// Bit rate calculation
		//1 Every lane speed
		/*
		params->dsi.pll_div1=0;		// div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
		params->dsi.pll_div2=0;		// div2=0,1,2,3;div1_real=1,2,4,4
		params->dsi.fbk_div =15;  //0x12  // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)
	*/
		params->dsi.pll_div1=0;		// div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
		params->dsi.pll_div2=0;		// div2=0,1,2,3;div1_real=1,2,4,4
		params->dsi.fbk_div =16;  //0x12  // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)
}

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

    push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
    mt_set_gpio_mode(GPIO_LCD_RST_EN, GPIO_MODE_01);
	
	#if defined(BUILD_LK)
	printf("lcm nt35596==in uboot=== lcm_init\n");
#else
	printk("lcm nt35596==in kernel=== lcm_init\n");
#endif
	
}

static void lcm_suspend(void)
{
	unsigned int data_array[16];
	//unsigned char buffer[2];
/*
	mt_set_gpio_mode(GPIO_LCD_RST_EN, GPIO_MODE_00);
	mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
	MDELAY(10);
	mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ZERO);
	MDELAY(20);
	mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
	MDELAY(20);
*/
	data_array[0]=0x00280500; // Display Off
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(20);

	data_array[0] = 0x00100500; // Sleep In
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(60);
	

	mt_set_gpio_mode(GPIO_LCD_RST_EN, GPIO_MODE_00);
	
	mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ZERO);
	MDELAY(120);
	
	mt_set_gpio_mode(GPIO_LCD_RST_EN, GPIO_MODE_01);
#ifdef BUILD_LK
        upmu_set_rg_vgp6_en(0);
#else
        hwPowerDown(MT65XX_POWER_LDO_VGP6, "LCM");
#endif

#if defined(BUILD_LK)
	printf("lcm nt35596==in uboot=== lcm_suspend\n");
#else
	printk("lcm nt35596==in kernel=== lcm_suspend\n");
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
	//lcm_compare_id();
#if defined(BUILD_LK)
	printf("lcm nt35596==in uboot=== lcm_resume\n");
#else
	printk("lcm nt35596==in kernel=== lcm_resume\n");
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

static void lcm_compare_id(void)
{
	unsigned int data_array[16];
	unsigned char buffer[5];
	unsigned int temp;
	unsigned int array[16];
	unsigned int id=0x0;

	mt_set_gpio_mode(GPIO_AVEE_EN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_AVEE_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_AVEE_EN, GPIO_OUT_ONE);
	mt_set_gpio_mode(GPIO_AVDD_EN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_AVDD_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_AVDD_EN, GPIO_OUT_ONE);

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
	MDELAY(10);
	mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ZERO);
	MDELAY(10);
	mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
	MDELAY(100);
	//mt_set_gpio_mode(GPIO_LCD_RST_EN, GPIO_MODE_01);
#if 0
	data_array[0] = 0x00023902;
	data_array[1] = 0x000020FF;
	dsi_set_cmdq(array, 2, 1);
#endif
	
	//data_array[0] = 0x00023902;
	//data_array[1] = 0x000001FB;
	//dsi_set_cmdq(array, 2, 1);
//	data_array[2] = 0x00000108;
	
	array[0] = 0x00033700;	// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0xF4, buffer, 3);
	id = buffer[0];		//we only need ID
	//id = buffer[0];
#ifdef BUILD_LK
	printf("%s, LK nt35596 debug: nt35596 id = 0x%08x, id2 = 0x%08x\n", __func__,
	       id,buffer[1]);
#else
	printk("%s, kernel nt35596 horse debug: nt35596 id = 0x%08x, id2 = 0x%08x\n",
	       __func__, id,buffer[1]);
#endif


	if( LCM_LG_TRULY_ID_NT35596==id)
	{
		return 1;
	}
	else
	{	return 0;
	}
	//return ((LCM_LG_TRULY_ID_NT35596 == id) ? 1:0);
}

LCM_DRIVER nt35596_auo60_ykl_fhd_lcm_drv = 
{
    .name			= "nt35596_auo60_ykl_fhd",
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
