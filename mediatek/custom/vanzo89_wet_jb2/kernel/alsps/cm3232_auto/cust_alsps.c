#include <linux/types.h>
#include <mach/mt_pm_ldo.h>
#include <cust_alsps.h>

static struct alsps_hw cust_alsps_hw = {
    .i2c_num    = 3,
	.polling_mode_ps =0,
	.polling_mode_als =1,
    .power_id   = MT65XX_POWER_NONE,    /*LDO is not used*/
    .power_vol  = VOL_DEFAULT,          /*LDO is not used*/
    //.i2c_addr   = {0x0C, 0x48, 0x78, 0x00},
   .als_level  = { 0,  1,  7,   15,  30,  60,  100, 300, 1500,  3000,  6000, 10000, 14000, 18000, 20000},
    .als_value  = {20, 40, 60,  90, 120, 160,  225,  320,  640,  1280,  1280,  2600,  2600, 2600,  10240, 10240},
  //  .als_level  = { 4, 40,  80,   120,   160, 250,  400, 800, 1200,  1600, 2000, 3000, 5000, 10000, 65535},
 //   .als_value  = {40, 40,90, 160, 225, 320,  800,  800, 800,  800,  800,  2000,  5000, 9000,  10240, 10240},
    .ps_threshold_high = 53,
    .ps_threshold_low = 46,
};
struct alsps_hw *get_cust_alsps_hw(void) {
    return &cust_alsps_hw;
}

