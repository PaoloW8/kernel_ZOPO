#ifndef EKTH3K_H__
#define EKTH3K_H__

/* Pre-defined definition */
#define TPD_TYPE_CAPACITIVE
#define TPD_TYPE_RESISTIVE
#define TPD_POWER_SOURCE         
#define TPD_I2C_NUMBER           0
#define TPD_WAKEUP_TRIAL         60
#define TPD_WAKEUP_DELAY         100

#define ELAN_X_MAX 	 832//960//576  
#define ELAN_Y_MAX	 1408//1856 //1792//960 

/*-------------------------------------------------------*/
#define TPD_TYPE_CAPACITIVE
#define TPD_TYPE_RESISTIVE
#if (defined(DCT_V5)||defined(DCT_E5))
#define TPD_POWER_SOURCE_CUSTOM MT65XX_POWER_LDO_VGP5
#define TPD_POWER_SOURCE_1800 MT65XX_POWER_LDO_VMC1
#elif (defined(DCT_V936) || defined(DCT_K7T) || defined(DCT_K7W))
#else
#define TPD_POWER_SOURCE_1800 MT65XX_POWER_LDO_VGP5
#endif
#define TPD_I2C_NUMBER           0
#define TPD_WAKEUP_TRIAL         60
#define TPD_WAKEUP_DELAY         100

/*-------------------------------------------------------*/

#define LCM_X_MAX	1080//simple_strtoul(LCM_WIDTH, NULL, 0)//896
#define LCM_Y_MAX	1920//simple_strtoul(LCM_HEIGHT, NULL, 0)//1728

#define ELAN_KEY_BACK	0x81 //Elan Key's define
#define ELAN_KEY_HOME	0x41
#define ELAN_KEY_MENU	0x21
//#define ELAN_KEY_SEARCH	0x11
#define ELAN_KTF2K_NAME "elan-ktf2k"

struct elan_ktf2k_i2c_platform_data {
	uint16_t version;
	int abs_x_min;
	int abs_x_max;
	int abs_y_min;
	int abs_y_max;
	int intr_gpio;
	int (*power)(int on);
};

//softkey is reported as AXIS
//#define SOFTKEY_AXIS_VER

//Orig. point at upper-right, reverse it.
//#define REVERSE_X_AXIS
struct osd_offset{
	int left_x;
	int right_x;
	unsigned int key_event;
};

//Elan add for OSD bar coordinate
static struct osd_offset OSD_mapping[] = {
  {35,  99,  KEY_MENU},	//menu_left_x, menu_right_x, KEY_MENU
  {203, 267, KEY_HOME},	//home_left_x, home_right_x, KEY_HOME
  {373, 437, KEY_BACK},	//back_left_x, back_right_x, KEY_BACK
  {541, 605, KEY_SEARCH},	//search_left_x, search_right_x, KEY_SEARCH
};

static int key_pressed = -1;


#define TPD_DELAY                (2*HZ/100)

#define TPD_CALIBRATION_MATRIX  {962,0,0,0,1600,0,0,0};


#define TPD_HAVE_BUTTON
#define TPD_BUTTON_HEIGH        (100)

#define TPD_KEY_COUNT           3
#define TPD_KEYS                { KEY_MENU, KEY_HOME, KEY_BACK}
#define TPD_KEYS_DIM            {{60,1360,120,TPD_BUTTON_HEIGH},{180,1360,120,TPD_BUTTON_HEIGH},{180,1360,120,TPD_BUTTON_HEIGH}}

#endif /* TOUCHPANEL_H__ */

