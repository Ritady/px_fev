/*
 * @Author: xiaojia
 * @email: limu.xiao@tuya.com
 * @LastEditors:
 * @file name: app_common.h
 * @Description: common function include file
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2021.8.30 17:28:01
 * @LastEditTime: 2021.8.30 17:28:01
 */

#ifndef  __APP_COMMON_H__
#define  __APP_COMMON_H__

#include "tuya_zigbee_stack.h"
#include "light_types.h"
#include "tuya_mcu_os.h"
#include "tuya_zigbee_modules.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */


#define PRINT_LEVEL_ERR    0 // print error information
#define PRINT_LEVEL_NOTICE 0 // print important information
#define PRINT_LEVEL_DEBUG  0 // print realtime information

  
#if (PRINT_LEVEL_ERR)
#define PR_ERR(fmt,...) app_print("ERROR "": "fmt"\r\n", ##__VA_ARGS__)

#else
#define PR_ERR(...)   
#endif

#if (PRINT_LEVEL_DEBUG)
#define PR_DEBUG(fmt,...) app_print("DEBUG ""%s(%d)-<%s>: "fmt"\r\n",__FILE__, __LINE__, __FUNCTION__,##__VA_ARGS__)

#else
#define PR_DEBUG(...)
#endif
  
#if (PRINT_LEVEL_NOTICE)
#define PR_NOTICE(fmt,...) app_print("NOTICE ""%s(%d)-<%s>: "fmt"\r\n",__FILE__, __LINE__, __FUNCTION__,##__VA_ARGS__)


#else
#define PR_NOTICE(...)
#endif


/*hart beat, report app version to coordinator, every 150s ~ 180s */
#define ZIGBEE_JOIN_MAX_TIMEOUT                 3*60*1000  //ms
#define ZIGBEE_HEAR_BEAT_DURATION               150*1000   //150s ~180s
#define NETWORK_JOIN_START_DELAY_TIME           1000       //ms
#define APP_CMD_HANDLE_DELAY_TIME               100        //ms

#define ZG_JOIN_TYPE_ZLL                        3

/*hardware timer id used by this app layer*/
#define HW_SCENE_SYNC_TIMER_ID                  6


#define PIN_NUM_ERROR       0xFF
#define PWM_MAX_FREQ        20000           ///< PWM MAX Frequency 20K
#define PWM_MIN_FREQ        100             ///< PWM MIN Frequency 100
#define PWM_MAX_DUTY        1000            ///< PWM MAX Duty 1000 --> Precision 0.1%

#define LIGHT_SCENE_MAX_LENGTH    210

#define get_array_len(x) (sizeof(x)/sizeof(x[0])) ///< get array number

/*  flash related macro definitions    */
#define FLASH_DATA_START_ADDRESS   0
#define FLASH_DATA_SIZE_MAX        250
#define FLASH_DATA_MAX_ADDRESS     (FLASH_DATA_START_ADDRESS + FLASH_DATA_SIZE_MAX)

#define R_NUM  9   //PB1
#define G_NUM  5   //PA4
#define B_NUM  1   //PA0

/**
 * @brief pwm init data structure
 * pwm send data structure
 */
typedef struct
{
    USHORT_T usFreq;            ///< PWM Frequency
    USHORT_T usDuty;            ///< PWM Init duty
    UCHAR_T ucList[3];          ///< GPIO List
    UCHAR_T ucChannel_num;      ///< GPIO List length
    BOOL_T bPolarity;           ///< PWM output polarity
    UCHAR_T ucCTRL_IO;          ///< CTRL pin parm
    BOOL_T  bCTRL_LV;           ///< Enable level
}user_pwm_init_t;

typedef enum {
    NETWORK_STATE_OFFLINE = 0,
    NETWORK_STATE_ONLINE,
}NETWORK_STATE_E;             /* Network join option */

/**
 * @brief software timer use id enum
 */
typedef enum {
    CLEAR_RESET_CNT_SW_TIMER = 35,
    SHADE_TIMER ,
    BLINK_SW_TIMER ,
    AUTOSAVE_SW_TIMER ,
    COUNTDOWN_SW_TIMER ,
    NETWORK_JOIN_START_DELAY_TIMER_ID , 
    APP_CMD_HANDLE_DELAY_TIMER_ID ,
    DELAY_REPORT_TIMER_ID ,
}SW_TIMER_ID_E;

typedef struct{
    UCHAR_T onoff;
    UCHAR_T mode;
    USHORT_T H;
    USHORT_T S;
    USHORT_T V;
    USHORT_T B;
    USHORT_T T;
}app_data_t;

typedef enum {
  NO_LEAVE = 0,
  LEAVE_ONLY = 1,
  LEAVE_AND_CLEAN = 2,
}remote_leave_t;

typedef struct{
    USHORT_T H;
    USHORT_T S;
    USHORT_T V;
}app_hsv_t;

typedef struct{
    UCHAR_T h;
    UCHAR_T s;
    UCHAR_T v;
}zigbee_hsv_t;

typedef enum
{
    LIGHT_MODE_WHITE = 0,
    LIGHT_MODE_COLOR,
    LIGHT_MODE_SCENE,
    LIGHT_MODE_MUSIC,
    LIGHT_MODE_MAX,
}LIGHT_MODE_T;

typedef struct
{
    USHORT_T Red;         /* color R setting */
    USHORT_T Green;
    USHORT_T Blue;
}color_rgb_t;

typedef struct 
{
    USHORT_T Hue;         /* color H setting */
    USHORT_T Saturation;  /* color S setting */
    USHORT_T Value;       /* color V setting */
}color_hsv_t;

/**
 * @brief Light save data structure
 */
typedef struct 
{
    BOOL_T          bPower;
    LIGHT_MODE_T    Mode;
    USHORT_T        usBright;
    USHORT_T        usTemper;
    color_rgb_t     Color;
    color_hsv_t     color_hsv;
}light_app_data_flash_t;

typedef struct {
    char*   string;
    UCHAR_T module;
}module_table_t; 


extern UCHAR_T scene_sync_ready_flag; //Is it time to send a synchronization command
extern UCHAR_T is_master_device; //it is the master device
extern USHORT_T g_join_network_time;//10 min
extern UCHAR_T  g_nwk_type;
extern UCHAR_T mf_test_flag;
extern NETWORK_STATE_E g_network_state;
extern UCHAR_T  cmd_control_shade_flag;

extern UCHAR_T WRITE_DATA[FLASH_DATA_SIZE_MAX];
extern UCHAR_T READ_DATA[FLASH_DATA_SIZE_MAX];


/**
 * @description: write app data attribute value,app data = onoff(1) + mode(1) + H(2) + S(2) + V(2) + B(2) + T(2)= 12 Bytes
 * @param {OUT app_data_t* app_data} data value to be write
 * @return: success:1 / faild:0
 */
bool_t write_app_data_attr(app_data_t *app_data);

/**
 * @description: read app data attribute value,app data = onoff(1) + mode(1) + H(2) + S(2) + V(2) + B(2) + T(2)= 12 Bytes
 * @param {OUT app_data_t* app_data} readout data value
 * @return: success:1 / faild:0
 */
bool_t read_app_data_attr(app_data_t* app_data);

/**
 * @description: app command received and process now
 * @param {type} none
 * @return: none
 */
void app_cmd_handle_delay_timer_cb(void);

/**
 * @description: load app data from Flash,and init the attributes
 * @param {type} none
 * @return: none
 */
void dev_load_attr_data(void);

/**
 * @description: load app data from Flash,and init the attributes
 * @param {type} none
 * @return: none
 */
void dev_read_attr_data(void);

/**
 * @description: get the final option,single control command is effective when the light is off,
 * need to combine the option attribute value and the current command's optionMask+optionOverride.
 * .Currently, only case of optionMask=0000 0001, Update the function when the alliance standard has expanded
 * @param {cluter} cluster id
 * @param {optionMask} Used to indicate which bits of the option attribute need to be rewritten
 * @param {optionOverride} to show the Bit of optionMask,which the rewritten value
 * @return: the final option value
 */
UCHAR_T get_option_value(CLUSTER_ID_T cluter, UCHAR_T option_mask, UCHAR_T option_override);


/**
 * @description: Turning on lights caused by non-switching commands
 * @param {IN bool_t onoff} onoff value
 * @param {IN SEND_QOS_T Qos} QOS value
 * @param {IN UINT_T delay_ms} delay report
 * @return: none
 */
OPERATE_RET op_set_onoff_with_countdown_cmd(bool_t onoff, SEND_QOS_T Qos, UINT_T delay_ms);

/**
 * @description: join start delay timer hander, join now
 * @param {none}
 * @return: none
 */
void network_join_start_delay_timer_callback(void);

/**
 * @berief: read light light ctrl data
 * @param {OUT UCHAR_T *data -> reset cnt}
 * @return: OPERATE_RET
 * @retval: none
 */
OPERATE_RET op_user_flash_read_light_ctrl_data(light_app_data_flash_t *data);

/**
 * @berief: read light reset cnt
 * @param {OUT UCHAR_T *data -> reset cnt}
 * @return: OPERATE_RET
 * @retval: none
 */
OPERATE_RET op_user_flash_read_reset_cnt(UCHAR_T *data);

/**
 * @berief: save light reset cnt
 * @param {IN UCHAR_T data -> save reset cnt to flash}
 * @return: OPERATE_RET
 * @retval: none
 */
OPERATE_RET op_user_flash_write_reset_cnt(UCHAR_T *data);


/**********************************************************************
*       light init function
**********************************************************************/
/**
 * @berief: pwm init
 * @param {user_pwm_init_t *light_config -> gpio set config}
 * @return: OPERATE_RET
 * @retval: OPERATE_RET
 */
OPERATE_RET light_pwm_init(user_pwm_init_t *light_config);

/**
 * @berief: Light hardware reboot judge & proc
 *          process detail:
 *                  1. hardware reset judge;
 *                  2. load reboot cnt data;
 *                  3. reboot cnt data increase;
 *                  4. start software time to clear reboot cnt;
 * @param {none}
 * @retval: none
 */
void dev_hw_reboot_proc(void);

/**
 * @berief: Light reset to re-distribute proc
 * @param {none}
 * @attention: this func will call light_sys_reset_cnt_over_callback()
 *              light_sys_reset_cnt_over_callback() need to implement by system
 * @retval: OPERATE_RET
 */
OPERATE_RET dev_reset_cnt_proc(void);


/**
 * @berief: read the flash data and set the light_ctrl_data
 * @param {none} 
 * @return: OPERATE_RET
 * @retval: none
 */
void remember_light_proc(void);

/**
 * @berief: light init
 * @param {none} 
 * @return: OPERATE_RET
 * @retval: none
 */
OPERATE_RET dev_light_init(void);




#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif