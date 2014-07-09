/* 
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
/*
 * Definitions for CM3232 als/ps sensor chip.
 */
#ifndef __CM3232_H__
#define __CM3232_H__

#include <linux/ioctl.h>

/*cm3232 als/ps sensor register related macro*/
#define CM3232_REG_ALS_CONF 		0X00
#define CM3232_REG_ALS_THDH 		0X01
#define CM3232_REG_ALS_THDL 		0X02
#define CM3232_REG_PS_CONF1_2		0X03
#define CM3232_REG_PS_CONF3_MS		0X04
#define CM3232_REG_PS_CANC			0X05
#define CM3232_REG_PS_THD			0X06
#define CM3232_REG_PS_DATA			0X08
#define CM3232_REG_ALS_DATA		0X50
#define CM3232_REG_INT_FLAG		0X0B
#define CM3232_REG_ID_MODE			0X0C

/*CM3232 related driver tag macro*/
#define CM3232_SUCCESS				 		 0
#define CM3232_ERR_I2C						-1
#define CM3232_ERR_STATUS					-3
#define CM3232_ERR_SETUP_FAILURE			-4
#define CM3232_ERR_GETGSENSORDATA			-5
#define CM3232_ERR_IDENTIFICATION			-6


#endif

