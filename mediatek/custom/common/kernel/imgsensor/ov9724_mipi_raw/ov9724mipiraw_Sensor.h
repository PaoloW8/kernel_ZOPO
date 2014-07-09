/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2005
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

/*****************************************************************************
 *
 * Filename:
 * ---------
 *   sensor.h
 *
 * Project:
 * --------
 *   DUMA
 *
 * Description:
 * ------------
 *   CMOS sensor header file
 *
 ****************************************************************************/
#ifndef _OV9724MIPI_SENSOR_H
#define _OV9724MIPI_SENSOR_H

#define OV9724MIPI_DEBUG
#define OV9724MIPI_DRIVER_TRACE
//#define OV9724MIPI_TEST_PATTEM
#ifdef OV9724MIPI_DEBUG
//#define SENSORDB printk
#else
//#define SENSORDB(x,...)
#endif

#define OV9724MIPI_FACTORY_START_ADDR 0
#define OV9724MIPI_ENGINEER_START_ADDR 10

//#define MIPI_INTERFACE

 
typedef enum OV9724MIPI_group_enum
{
  OV9724MIPI_PRE_GAIN = 0,
  OV9724MIPI_CMMCLK_CURRENT,
  OV9724MIPI_FRAME_RATE_LIMITATION,
  OV9724MIPI_REGISTER_EDITOR,
  OV9724MIPI_GROUP_TOTAL_NUMS
} OV9724MIPI_FACTORY_GROUP_ENUM;

typedef enum OV9724MIPI_register_index
{
  OV9724MIPI_SENSOR_BASEGAIN = OV9724MIPI_FACTORY_START_ADDR,
  OV9724MIPI_PRE_GAIN_R_INDEX,
  OV9724MIPI_PRE_GAIN_Gr_INDEX,
  OV9724MIPI_PRE_GAIN_Gb_INDEX,
  OV9724MIPI_PRE_GAIN_B_INDEX,
  OV9724MIPI_FACTORY_END_ADDR
} OV9724MIPI_FACTORY_REGISTER_INDEX;

typedef enum OV9724MIPI_engineer_index
{
  OV9724MIPI_CMMCLK_CURRENT_INDEX = OV9724MIPI_ENGINEER_START_ADDR,
  OV9724MIPI_ENGINEER_END
} OV9724MIPI_FACTORY_ENGINEER_INDEX;

typedef struct _sensor_data_struct
{
  SENSOR_REG_STRUCT reg[OV9724MIPI_ENGINEER_END];
  SENSOR_REG_STRUCT cct[OV9724MIPI_FACTORY_END_ADDR];
} sensor_data_struct;
typedef enum {
    OV9724MIPI_SENSOR_MODE_INIT = 0,
    OV9724MIPI_SENSOR_MODE_PREVIEW,
    OV9724MIPI_SENSOR_MODE_VIDEO,
    OV9724MIPI_SENSOR_MODE_CAPTURE
} OV9724MIPI_SENSOR_MODE;


/* SENSOR PREVIEW/CAPTURE VT CLOCK */
//#define OV9724MIPI_PREVIEW_CLK                     69333333  //48100000
//#define OV9724MIPI_CAPTURE_CLK                     69333333  //48100000

#define OV9724MIPI_COLOR_FORMAT                    SENSOR_OUTPUT_FORMAT_RAW_B //SENSOR_OUTPUT_FORMAT_RAW_R

#define OV9724MIPI_MIN_ANALOG_GAIN				1	/* 1x */
#define OV9724MIPI_MAX_ANALOG_GAIN				8	/* 32x */


/* FRAME RATE UNIT */
#define OV9724MIPI_FPS(x)                          (10 * (x))

/* SENSOR PIXEL/LINE NUMBERS IN ONE PERIOD */
//#define OV9724MIPI_FULL_PERIOD_PIXEL_NUMS          2700 /* 9 fps */

//#define OV9724MIPI_DEBUG_SETTING
//#define OV9724_BINNING_SUM//binning: enable  for sum, disable for vertical averag	
//#define OV9724MIPI_4LANE



#define OV9724MIPI_FULL_PERIOD_PIXEL_NUMS          3000  //3055 /* 8 fps */
#define OV9724MIPI_FULL_PERIOD_LINE_NUMS           900//2642//2586 //2484   //1968
#define OV9724MIPI_PV_PERIOD_PIXEL_NUMS            3000 //1630 /* 30 fps */
#define OV9724MIPI_PV_PERIOD_LINE_NUMS             900//1260 //984
#define OV9724MIPI_VIDEO_PERIOD_PIXEL_NUMS            3000 //1630 /* 30 fps */
#define OV9724MIPI_VIDEO_PERIOD_LINE_NUMS             900//1260 //984

/* SENSOR START/END POSITION */
#define OV9724MIPI_FULL_X_START                    2
#define OV9724MIPI_FULL_Y_START                    2
#define OV9724MIPI_IMAGE_SENSOR_FULL_WIDTH         (1296 - 16) /* 2560 */
#define OV9724MIPI_IMAGE_SENSOR_FULL_HEIGHT        (736 - 16) /* 1920 */
#define OV9724MIPI_PV_X_START                      2
#define OV9724MIPI_PV_Y_START                      2
#define OV9724MIPI_IMAGE_SENSOR_PV_WIDTH           (1296 - 16)  //    (1280 - 16) /* 1264 */
#define OV9724MIPI_IMAGE_SENSOR_PV_HEIGHT          (736 - 16)  //(960 - 12) /* 948 */

#define OV9724MIPI_VIDEO_X_START                    2
#define OV9724MIPI_VIDEO_Y_START                    2
#define OV9724MIPI_IMAGE_SENSOR_VIDEO_WIDTH         (1296 - 16)//(3264 - 64) /* 2560 */
#define OV9724MIPI_IMAGE_SENSOR_VIDEO_HEIGHT        (736 - 16)//(2448 - 48) /* 1920 */


/* SENSOR READ/WRITE ID */
#define OV9724MIPI_SLAVE_WRITE_ID_1   (0x6c)
#define OV9724MIPI_SLAVE_WRITE_ID_2   (0x20)

#define OV9724MIPI_WRITE_ID   (0x20)  //(0x6c)
#define OV9724MIPI_READ_ID    (0x21)  //(0x6d)

/* SENSOR ID */
//#define OV9724MIPI_SENSOR_ID						(0x5647)


//added by mandrave
//#define OV9724MIPI_USE_OTP

#if defined(OV9724MIPI_USE_OTP)

struct imx188mipi_otp_struct
{
    kal_uint16 customer_id;
	kal_uint16 module_integrator_id;
	kal_uint16 lens_id;
	kal_uint16 rg_ratio;
	kal_uint16 bg_ratio;
	kal_uint16 user_data[5];
	kal_uint16 lenc[63];

};

#define RG_TYPICAL 0x51
#define BG_TYPICAL 0x57


#endif




/* SENSOR PRIVATE STRUCT */
typedef struct OV9724MIPI_sensor_STRUCT
{
  MSDK_SENSOR_CONFIG_STRUCT cfg_data;
  sensor_data_struct eng; /* engineer mode */
  MSDK_SENSOR_ENG_INFO_STRUCT eng_info;
  kal_uint8 mirror;
  kal_bool pv_mode;
  kal_bool video_mode;
  kal_bool is_zsd;
  kal_bool is_zsd_cap;
  kal_bool is_autofliker;
  kal_uint16 normal_fps; /* video normal mode max fps */
  kal_uint16 night_fps; /* video night mode max fps */
  kal_uint16 shutter;
  kal_uint16 gain;
  kal_uint32 pv_pclk;
  kal_uint32 cap_pclk;
  kal_uint32 video_pclk;
  kal_uint32 pclk;
  kal_uint16 frame_length;
  kal_uint16 line_length;  
  kal_uint16 write_id;
  kal_uint16 read_id;
  kal_uint16 dummy_pixels;
  kal_uint16 dummy_lines;
  kal_uint32 max_exposure_lines;
  
  OV9724MIPI_SENSOR_MODE sensorMode;
} OV9724MIPI_sensor_struct;

//export functions
UINT32 OV9724MIPIOpen(void);
UINT32 OV9724MIPIControl(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow, MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
UINT32 OV9724MIPIFeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId, UINT8 *pFeaturePara,UINT32 *pFeatureParaLen);
UINT32 OV9724MIPIGetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_INFO_STRUCT *pSensorInfo, MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
UINT32 OV9724MIPIGetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution);
UINT32 OV9724MIPIClose(void);

#define Sleep(ms) mdelay(ms)

#endif 
