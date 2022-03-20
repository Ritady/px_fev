/*
 * @Author: xiao jia
 * @email: limu.xiao@tuya.com
 * @Date: 2021-08-31 20:32:45
 * @file name: light_control.h
 * @Description:
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 */

#ifndef __LIHGT_CONTROL_H__
#define __LIHGT_CONTROL_H__


#include "light_types.h"
#include "tuya_zigbee_stack.h"
#include "app_common.h"
#include "light_tools.h"
#include "hal_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

  /// hardware timer cycle (unit:ms)
#define HW_TIMER_CYCLE_MS           HW_TIMER_CYCLE_US/1000

/// Control calculate range 0 ~ 360
#define CTRL_HSV_H_MAX              360
/// Control calculate range 0 ~ 1000
#define CTRL_CAL_VALUE_RANGE        1000
/// Control cw calc max bright value
#define CTRL_CW_BRIGHT_VALUE_MAX    CTRL_CAL_VALUE_RANGE
/// Control cw calc min bright value (max value * 0.01)
#define CTRL_CW_BRIGHT_VALUE_MIN    (CTRL_CW_BRIGHT_VALUE_MAX * 0.01)
/// Control RGB calc max bright value
#define CTRL_RGB_BRIGHT_VALUE_MAX   CTRL_CAL_VALUE_RANGE
/// Control RGB calc min bright value
#define CTRL_RGB_BRIGHT_VALUE_MIN   (CTRL_RGB_BRIGHT_VALUE_MAX * 0.01)

/// hardware timer cycle (unit:us)
#define HW_TIMER_CYCLE_US           5000

/// light shade change cycle (unit:ms)
#define LIGHT_SHADE_CYCLE           5

// CIE XY max value
#define MAX_CIE_XY_VALUE           0xfeff

/// shade change max time(the worst situation),uint:ms
#define SHADE_CHANG_MAX_TIME        1000

/// scene head(scene Num) length
#define SCNE_HRAD_LENGTH            2
/// scene unit length
#define SCENE_UNIT_LENGTH           26
/// scene max unit number
#define SCENE_MAX_UNIT              8
/// scene data min length, formu -> 2+ 26 = 28
#define SCENE_MIN_LENGTH            (SCNE_HRAD_LENGTH + SCENE_UNIT_LENGTH)
/// scene data max length, formu -> 2 + 26*8  = 210
#define SCENE_MAX_LENGTH            (SCNE_HRAD_LENGTH + SCENE_UNIT_LENGTH * SCENE_MAX_UNIT)

/// scene C default scene ctrl data
#define SCENE_DATA_DEFAULT_C        "000e0d00002e03e8000000c803e8"
/// scene CW default scene ctrl data
#define SCENE_DATA_DEFAULT_CW       "000d0d0000000000000000c80000"
/// scene RGB/RGBC/RGBCW default scene ctrl data
#define SCENE_DATA_DEFAULT_RGB      "000e0d00002e03e802cc00000000"
  
#define LOWPOWER_TIMER_CYCLE_MS     3000 //ms

#define RESET_CNT_OFFSET          0
#define LIGHT_APP_DATA_OFFSET     (RESET_CNT_OFFSET + 2)
#define PROD_TEST_DATA_OFFSET     (LIGHT_APP_DATA_OFFSET + sizeof(light_app_data_flash_t) + 1)
  
/**
 * @brief commond type
 * CMD_TYPE_SINGLE -> single commond
 * CMD_TYPE_GROUP -> group commond
 * 
 */
typedef enum{
	CMD_TYPE_SINGLE = 0,
	CMD_TYPE_GROUP
}CMD_TYPE_T;

/**
 * @brief Light way type enum
 *          1~5 ways
 */
typedef enum {
    LIGHT_C = 0,
    LIGHT_CW,
    LIGHT_RGB,
    LIGHT_RGBC,
    LIGHT_RGBCW,
    LIGHT_MAX,
}CTRL_LIGHT_WAY_E;

typedef enum {
    DEF_COLOR_C = 0,
    DEF_COLOR_W,
    DEF_COLOR_R,
    DEF_COLOR_G,
    DEF_COLOR_B,
    DEF_COLOR_RGB,
    DEF_COLOR_MAX,
}CTRL_DEF_COLOR_E;

/**
 * @brief commond type
 * CMD_TYPE_SINGLE -> single commond
 * CMD_TYPE_GROUP -> group commond
 *
 */
typedef enum{
	ZIGBEE_CMD_SINGLE = 0,
	ZIGBEE_CMD_GROUP
}ZIGBEE_CMD_T;

/**
 * @brief Light control switch change mode enum
 *          SWITCH_GRADUAL -> turn on/off gradually
 *          SWITCH_DIRECT  -> turn on/off directly
 */
typedef enum {
    SWITCH_GRADUAL = 0,
    SWITCH_DIRECT,
    SWITCH_MAX,
}CTRL_SWITCH_MODE_E;

/**
 * @brief Light control drive mode enum
 *          BRIGHT_MODE_CW  -> CW drive by pwm totally
 *          BRIGHT_MODE_CCT -> C value is bright setting essentially , w is the scale of C&W .
 *                          the light will send warm, when w is set by zero.
 */
typedef enum {
    BRIGHT_MODE_CW = 0,
    BRIGHT_MODE_CCT,
    BRIGHT_MODE_MAX,
}CTRL_BRIGHT_MODE_E;

/**
 * @brief Light control scene change mode enum
 *          SCENE_STATIC    -> scene hold on no change
 *          SCENE_JUMP      -> scene change by directly
 *          SCENE_SHADOW    -> scene change by gradually
 */
typedef enum {
    SCENE_STATIC = 0,
    SCENE_JUMP,
    SCENE_SHADE,
    SCENE_MAX,
}CTRL_SCENE_MODE_E;

typedef enum {
    RESET_MOD_CNT = 0,
}CTRL_RESET_MODE_E;             /* this mode can't choose in bulb! */


/**
 * @brief Light control need configuration sturct
 * the configuration need to set firstly ,when use the control proccess
 */
typedef struct
{
    CTRL_LIGHT_WAY_E    light_way;
    CTRL_DEF_COLOR_E    net_color;
    USHORT_T            net_bright;    /* blink bright in connect proc */
    USHORT_T            net_temper;    /* blink temper in connect proc */
    CTRL_DEF_COLOR_E    def_color;
    USHORT_T            def_bright;    /* default bright */
    USHORT_T            def_temper;    /* default temper */
    CTRL_SWITCH_MODE_E  switch_mode;     /* turn on/off mode */
    CTRL_BRIGHT_MODE_E  bright_mode;     /* CCT&CW drive mode */
    UCHAR_T             power_gain;    /* power gain(amplification) */
    UCHAR_T             limit_cw_max;   /* CW limit Max value */
    UCHAR_T             limit_cw_min;   /* CW limit Min value */
    UCHAR_T             limit_rgb_max;
    UCHAR_T             limit_rgb_min;
    UCHAR_T             reset_cnt;     /* re distibute cnt */
    UCHAR_T             connect_mode;  /* connect cfg mode */
    BOOL_T              memory;        /* ifnot save flag  */
}light_ctrl_cfg_t;

/// light ctrl configuration
extern light_ctrl_cfg_t light_cfg_data;

/**
 * @brief Light control data structure
 * storage the light control data(normal issued by app)
 */
typedef enum
{
    WHITE_MODE = 0,
    COLOR_MODE,
    SCENE_MODE,
    MUSIC_MODE,
    MODE_MAX,
}LIGHT_MODE_E;

/**
 * @brief Light control color(RGB) data structure
 * storage the light control color(RGB) data(normal issued by app)
 */
typedef struct
{
    USHORT_T Red;         /* color R setting */
    USHORT_T Green;
    USHORT_T Blue;
}COLOR_RGB_T;

typedef enum
{
    COLOR_MODE_RGB = 0,
    COLOR_MODE_HSV,
    COLOR_MODE_XY,
}COLOR_MODE_E;

/**
 * @brief Light control color(XY) data structure
 * storage the light control color(XY) data(normal issued by Philips Hue app)
 */
typedef struct 
{
    USHORT_T usX;         /* color R setting */
    USHORT_T usY;
}COLOR_XY_T;

/**
 * @brief Light control color(HSV) data structure
 * storage the light control color(HSV) data(normal issued by app)
 */
typedef struct
{
    USHORT_T Hue;         /* color H setting */
    USHORT_T Saturation;  /* color S setting */
    USHORT_T Value;       /* color V setting */
}COLOR_HSV_T;

/**
 * @brief Light control real time control change mode enum
 */
typedef enum {
    REALTIME_CHANGE_JUMP = 0,
    REALTIME_CHANGE_SHADE,
}REALTIME_CHANGE_E;

/**
 * @brief Light control data structure
 * storage the light control data(normal issued by app)
 */
typedef struct
{
    BOOL_T onoff_status;             /* on off setting */
    LIGHT_MODE_E Mode;
    COLOR_MODE_E color_mode;
    COLOR_RGB_T Color;
    COLOR_HSV_T color_hsv;       // HSV value, for Zibgee3.0 command, V = usBright
    UINT_T count_down;
    UCHAR_T real_time_data[22]; // Tuya realtime value
    UCHAR_T ucRealTimeFlag;
}LIGHT_CTRL_DATA_T;

/**
 * @brief shade mode
 */
typedef enum
{
    MOVE_UP = 0,
    MOVE_DOWM,
}MOVE_MODE_E;

typedef enum {
    MOVE_MODE_UP = 0,
    MOVE_MODE_DOWN,
    MOVE_MODE_MAX,
}MOVE_MODE_T;

/**
 * @brief Bright 5ways value structure
 * Used in light gradually change calculation process
 */
typedef struct
{
    USHORT_T Red;
    USHORT_T Green;
    USHORT_T Blue;
    USHORT_T White;
    USHORT_T Warm;
}light_data_t;

/**
 * @brief whitch param to be shade
 */
typedef enum {
    SHADE_PARAM_RGBCW = 0,
    SHADE_PARAM_B,
    SHADE_PARAM_T,
    SHADE_PARAM_H,
    SHADE_PARAM_S,
    SHADE_PARAM_V,
    SHADE_PARAM_H_LOOP,
    SHADE_PARAM_RGBCW2,
}SHADE_PARAM_E;

/**
 * @brief Light gradually change structure
 * Used in light gradually change calculation process
 */
typedef struct
{
    BOOL_T first_change;    /* first change flag, need to calculate change step */
    SHADE_PARAM_E shade_param;   // What param of light to be shade
    FLOAT_T slope;
    light_data_t target_val;    // Target RGBCW output
    light_data_t current_val;
    //Used for HSV change shade
    COLOR_HSV_T current_hsv;     // Current hsv value,Used for H, S, V shade when HSV change(color mode = HSV)
    COLOR_HSV_T target_hsv;     // Target hsv value,Used for H, S, V shade when HSV change(color mode = HSV)
}LIGHT_CTRL_HANDLE_T;


typedef enum{
    MODE_C = 0,
    MODE_CW = 1,
    MODE_RGB = 2,
}PWM_LIGHT_MODE_E;


/**
 * @brief pwm send data structure
 * pwm send data structure
 */
typedef struct
{
    USHORT_T usR_Value;       ///< R value,rang from 0 to 1000
    USHORT_T usG_Value;       ///< G value,rang from 0 to 1000
    USHORT_T usB_Value;       ///< B value,rang from 0 to 1000
    USHORT_T usC_Value;       ///< C value,rang from 0 to 1000
    USHORT_T usW_Value;       ///< W value,rang from 0 to 1000
    PWM_LIGHT_MODE_E usmode;
}user_pwm_color_t;

typedef struct{
    UCHAR_T num;
    GPIO_PORT_T port;
    GPIO_PIN_T pin;
}num2pin_t;

/// light ctrl data(status)
extern LIGHT_CTRL_DATA_T light_ctrl_data;

/// light ctrl handle(process)
extern LIGHT_CTRL_HANDLE_T light_ctrl_handle_data;

extern BOOL_T   level_down_to_off;


/****************************************************************************
* Functions
*
* These APIs used to be implemented at the application layer or other file
* now declare and definiction in the light_control file
*
****************************************************************************/

/**
 * @note: set light count down cnt
 * @param [in] {uint32_t}count
 * @return: none
 */
void op_light_count_down_cnt_set(UINT_T count);

/**
 * @note: return app_light_count_down_cnt_get
 * @param [in] none
 * @return: app_light_count_down_cnt_get
 */
uint32_t app_light_count_down_cnt_get(void);

/**
 * @description: light control callback,Notify the application layer of the current countdown remaining time
 * @param {RemainTimeSec} Countdown remaining time
 * @return: none
 */
void light_data_count_down_response(UINT_T remain_time_sec);

/**
 * @description: light system callback of reset count clear
 * @param {type} none
 * @return: none
 */
void light_sys_reset_cnt_clear_callback(void);

/**
 * @berief: Light reset cnt clear timercallback
 * @param {none}
 * @return: none
 * @retval: none
 */
void light_ctrl_reset_cnt_clear_timer_callback(void);

/**
 * @description: system reboot as hardware mode jude proc
 * @param {type} none
 * @return: TRUE:reset reason is a extern reset, FALSE: not a extern reset
 */
BOOL_T b_light_sys_hw_reboot_jude(void);

/**
 * @description: reset's times exceed callback function, the apl should start connect network
 * @param {none}
 * @return: OPERATE_RET
 */
OPERATE_RET light_sys_reset_cnt_over_callback(void);

/**
 * @berief: light send control data
 * @param {usR_value} red color, range 0~1000
 * @param {usG_Value} Green color, range 0~1000
 * @param {usB_Value} Blue color, range 0~1000
 * @param {usC_Value} cold white color, range 0~1000
 * @param {usW_Value} warm white color, range 0~1000
 * @return: OPERATE_RET
 * @retval: OPERATE_RET
 */
OPERATE_RET op_light_set_rgb(USHORT_T usR_value, USHORT_T usG_value, USHORT_T usB_value);
/**********************************************************************
*      end  Functions
**********************************************************************/


/**
 * @berief: calculate the RGB Light_Handle.TargetVal according to
 *          the Light_Data value.
 * @param {IN COLOR_RGB_T *Color -> RGB ctrl data}
 * @param {IN light_data_t *Result -> Result handle data}
 * @retval: none
 */
void light_ctrl_data_calc_rgb(COLOR_RGB_T *Color, light_data_t *Result);


void light_set_reset_handle_data(light_data_t data);

/**
 * @berief: set light ctrl data to default according light_cfg_data
 * @param {none}
 * @retval: none
 */
void light_ctrl_data_reset(void);

/**
 * @berief: Light system control hardware timer callback
 * @param {none}
 * @attention: this function need to implement by system,
 *              decide how to call vLightCtrlShadeGradually function.
 * @retval: none
 */
void light_sys_hardware_timer_callback(void);

/**
 * @berief: Light control hardware timer callback
 * @param {none}
 * @attention: light_sys_hardware_timer_callback() func need to implement by system
 * @retval: none
 */
void light_ctrl_hardware_timer_callback(void);

/**
 * @berief: set light switch data, adapte control data issued by system
 *          to control data format.
 * @param {IN BOOL_T bONOFF -> switch data, TRUE will turn on}
 * @retval: OPERATE_RET -> OPRT_OK meaning need to call op_light_ctrl_proc() function!
 */
OPERATE_RET op_light_ctrl_data_switch_set(BOOL_T bONOFF);

/**
 * @berief: reponse mode property process,
 *          this need to implement by system.
 * @param {OUT LIGHT_MODE_E Mode}
 * @retval: none
 */
void light_ctrl_data_mode_response(LIGHT_MODE_E Mode);

/**
 * @berief: set light mode data
 * @param {IN LIGHT_MODE_E Mode}
 * @attention:Mode value is below:
 *                                  WHITE_MODE = 0,
 *                                  COLOR_MODE = 1,
 *                                  SCENE_MODE = 2,
 *                                  MUSIC_MODE = 3,
 * @retval: OPERATE_RET -> OPRT_OK meaning need to call op_light_ctrl_proc() function!
 */
OPERATE_RET op_light_ctrl_data_mode_set(LIGHT_MODE_E Mode);

/**
 * @berief: Set Bright and HSV->V both
 * @param {Level}  Bright and HSV->V value: 0~1000
 * @param {with_onoff}  TRUE: turn on immediately if level is not 0 or shade to off if level is 0;
 *                     FALSE: don't change onoff,just shade to level.
 * @retval: OPERATE_RET: OPRT_OK / OPRT_INVALID_PARM
 */
OPERATE_RET op_light_ctrl_data_level_set(USHORT_T Level, USHORT_T trans_time);

/**
 * @berief: set light RGB data
 * @param {IN COLOR_RGB_T *Color}
 * @param {IN COLOR_ORIGINAL_S *ColorOrigin -> color origin data save}
 * @attention: acceptable format is RGB module, R,G,B range:0~1000
 * @retval: OPERATE_RET -> OPRT_OK meaning need to call opLightCtrlProc() function!
 */
OPERATE_RET op_light_ctrl_data_rgb_set(COLOR_RGB_T *color_rgb);

/**
 * @berief: set light HSV data
 * @param {OUT COLOR_HSV_T *color_hsv}
 * @attention: acceptable format is HSV module, H:0~360, S:0~1000, V:0~1000
 * @retval: OPERATE_RET -> OPRT_OK meaning need to call opLightCtrlProc() function!
 */
OPERATE_RET op_light_ctrl_data_hsv_set(COLOR_HSV_T *Color);

/**
 * @description: Move to hue and saturation with transition time handler
 * @param {Hue} Target Hue
 * @param {Saturation} Target Saturation
 * @param {trans_time} Transtion time in 100ms
 * @return: OPERATE_RET: OPRT_OK / OPRT_INVALID_PARM
 */
OPERATE_RET op_light_ctrl_data_hs_set(USHORT_T Hue, USHORT_T Saturation, USHORT_T trans_time);

/**
 * @description: Move to Hue with direction and transition time handler
 * @param {Hue} Target Hue:0~360, other value has no effect.
 * @param {move_mode} UP / DOWN
 * @param {trans_time} Transtion time in 100ms,0:
*  @return: OPERATE_RET: OPRT_OK / OPRT_INVALID_PARM
 */
OPERATE_RET op_light_ctrl_data_hue_set(USHORT_T Hue, MOVE_MODE_T move_mode, USHORT_T trans_time);   

/**
 * @description: Move to Saturation with transition time handler
 * @param {Saturation} Target Saturation
 * @param {trans_time} Transtion time in 100ms
*  @return: OPERATE_RET: OPRT_OK / OPRT_INVALID_PARM
 */
OPERATE_RET op_light_ctrl_data_saturation_set(USHORT_T Saturation, USHORT_T trans_time);

/**
 * @description: Bright and HSV->V move up or move down
 * @param {move_mode} level move mode :up / down
 * @param {Rate} move rate,if 0x00/0xFF: 1/ms, else Rate/S
 * @param {with_onoff} move mode :up / down
 * @return: none
 */
OPERATE_RET op_light_ctrl_data_level_move(MOVE_MODE_T move_mode, USHORT_T Rate, BOOL_T with_onoff);

/**
 * @berief: get light switch data
 * @param {OUT BOOL_T *onpONOFFoff -> switch data return}
 * @retval: OPERATE_RET
 */
OPERATE_RET op_light_ctrl_data_switch_get(BOOL_T *onoff);


/**
 * @berief: get light mode data
 * @param {OUT LIGHT_MODE_E *Mode -> mode data return}
 * @retval: OPERATE_RET
 */
OPERATE_RET op_light_ctrl_data_mode_get(LIGHT_MODE_E *Mode);

/**
 * @berief: get light color mode
 * @param {OUT COLOR_MODE_E *Mode -> mode data return}
 * @retval: OPERATE_RET
 */
OPERATE_RET op_light_ctrl_data_color_mode_get(COLOR_MODE_E *Mode);

/**
 * @berief: get light RGB data
 * @param {OUT COLOR_RGB_T *Color -> color original data return}
 * @retval: OPERATE_RET
 */
OPERATE_RET op_light_ctrl_data_rgb_get(COLOR_RGB_T *color_rgb);

/**
 * @berief: Get light HSV data
 * @param {OUT COLOR_HSV_T *color_hsv}  HSV data
 * @retval: OPERATE_RET: OPRT_OK / OPRT_INVALID_PARM
 */
OPERATE_RET op_light_ctrl_data_hsv_get(COLOR_HSV_T *color_hsv);

/**
 * @berief: get control ctrl data
 * @param {none}
 * @retval: LIGHT_CTRL_DATA_T
 */
LIGHT_CTRL_DATA_T* op_light_ctrl_data_get(void);

/**
 * @berief: Light control proc
 * @param {none}
 * @retval: OPERATE_RET
 */
OPERATE_RET op_light_ctrl_proc(void);

/**
 * @description: Move to off when shade stop
 * @param {none}
 * @return: OPRT_OK
 */
void light_ctrl_data_move_to_off_set(void);

/**
 * @berief: set light countdown value
 * @param {IN INT_T CountDownSec -> unit:second}
 * @attention: countdown lave time will return with
 *              calling light_data_count_down_response function every minutes.
 *              switch status will return with calling
 *              light_ctrl_data_switch_response function when countdown active.
 * @retval: OPERATE_RET -> OPRT_OK set countdown OK.
 */
OPERATE_RET op_light_ctrl_data_count_down_set(INT_T CountDownSec);

/**
 * @description: Move Hue with rate handler,Hue move will not stop until stop move reaceived.
 * @param {move_mode} move up / move down
 * @param {Rate} move uint per second,if rate = 0,this api has no effect
 * @return: OPERATE_RET: OPRT_OK / OPRT_INVALID_PARM
 */
OPERATE_RET op_light_ctrl_data_hue_move(MOVE_MODE_T move_mode, USHORT_T Rate);

/**
 * @berief: Light ctrl data save
 * @param {none}
 * @attention: this function directly save ctrl data.
 * @retval: none
 */
OPERATE_RET op_light_ctrl_data_auto_save(void);

/*
* @berief: network joined,light turn off 200ms,then turn on
* @param {none}
* @attention:
* @retval: none
*/
OPERATE_RET op_light_ctrl_nwk_join_ok_notify(void);

/**
 * @description: identify cmd handler
 * @param {identifyTimes} identify times
 * @return: none
 */
void light_ctrl_identify_handler(UCHAR_T identifyTimes);

/**
 * @description: start blink
 * @param {IN UINT_T BlinkTimeMs -> blink phase}
 * @attention: blink display will as the parm
 *             -- net_color, net_bright in configuration.
 * @retval: none
 */
OPERATE_RET op_light_ctrl_blink_start(UINT_T BlinkTimeMs);

/**
 * @description: stop blink
 * @param {type} none
 * @attention: blink stop will directly go to normal status display
 *              normal status will bright as default bright parm
 *              -- def_bright,def_temper,def_color in configuration.
 * @retval: none
 */
OPERATE_RET op_light_ctrl_blink_stop(void);

/**
 * @description: stop Hue,Saturation, XY moving
 * @param {type} none
 * @return: none
 */
OPERATE_RET op_light_ctrl_data_move_stop(void);

/**
 * @description: set scene uint and start scene change
 * @param {uint_num} scene uint
 * @return: none
 */
void light_ctrl_scene_change_start_with_uint(UCHAR_T uint_num);

/**
 * @berief: set light realtime control data
 * @param {IN BOOL_T bMusicFlag -> music control data}
 * @param {IN UCHAR_T *RealTimeData}
 * @attention: data format: please reference to DP protocol
 * @retval: OPERATE_RET -> OPRT_OK need to call opLightRealTimeCtrlProc function.
 */
OPERATE_RET op_light_ctrl_data_realtime_adjust_set(BOOL_T bMusicFlag, UCHAR_T *RealTimeData);

/**
 * @berief: Light realtime ctrl process
 * @param {none}
 * @retval: OPERATE_RET
 */
OPERATE_RET op_light_realtime_ctrl_proc(void);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif  /* __LIHGT_CONTROL_H__ */
