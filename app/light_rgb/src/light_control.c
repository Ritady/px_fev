/*
 * @file name: light_control.c
 * @Description: light control process
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2021-08-29 10:40:15
 * @LastEditTime: 2021-08-31 20:32:56
 */

#include "light_control.h"
#include "tuya_zigbee_stack.h"
#include "light_types.h"
#include "tuya_zigbee_sdk.h"
#include "dev_register.h"
#include "onoff_cluster.h"
#include "level_cluster.h"
#include "color_control_cluster.h"


/// control power gain (100 --> gain = 1 times )
#define CTRL_POEWER_GAIN_MIN        100
/// control power gain (200 --> gain = 2 times )
#define CTRL_POEWER_GAIN_MAX        200

/// control CW&RGB bright limit max
#define CTRL_BRIGHT_LIMIT_MAX       100
/// control CW&RGB bright limit min
#define CTRL_BRIGHT_LIMIT_MIN       0

/// control CW&RGB temper limit max
#define CTRL_TEMPER_LIMIT_MAX       100
/// control CW&RGB temper limit min
#define CTRL_TEMPER_LIMIT_MIN       0

#define LIGHT_SHADE_CYCLE           5 // light shade cycle (unit:ms)
#define SHADE_STEP_GAIN_DEFAULT     5 // light shade step per cycle
#define SHADE_SLOPE_DEFAULT         SHADE_STEP_GAIN_DEFAULT/LIGHT_SHADE_CYCLE

#define COLOR_RED_X  45874
#define COLOR_RED_Y  19659

#define COLOR_GREEN_X  7208
#define COLOR_GREEN_Y  53737

#define COLOR_BLUE_X  7865
#define COLOR_BLUE_Y  5242

#define BREATH_TIMER_CYCLE_MS        10 //ms
#define BREATH_STEP_VALUE            CTRL_CAL_VALUE_RANGE/(uiBreathTimeMs/BREATH_TIMER_CYCLE_MS)

#define NETWORK_JOIN_IDENTIFY_MS     200 //ms



/**
 * @brief hardware timer drive actively control structure
 */
typedef struct
{
    UINT_T target_cnt;     /* shade time per one change */
    UINT_T cnt;           /* hardware time */
    BOOL_T enable;         /* hardware time deal with shade change flag */
}HW_TIMER_PARAM_S;

/// hardware time set param
static HW_TIMER_PARAM_S hw_timer_param = {
    .target_cnt = 0xFFFFFF,
    .cnt = 0,
    .enable = FALSE,
};

typedef struct{
    UCHAR_T  driver_type;
    UCHAR_T  light_type;    //1way~5way
    UCHAR_T  light_mix_type;

    UCHAR_T  power_pro_type;
    USHORT_T  rgb_max;
    UCHAR_T  cw_max;

}LIGHT_POWER_GAIN_T;

typedef struct{
  UCHAR_T module_name_str[15];
  UCHAR_T module_name_len;
}module_name_t;

light_ctrl_cfg_t light_cfg_data;

/// light ctrl data(status)
LIGHT_CTRL_DATA_T light_ctrl_data;

/// light ctrl handle(process)
LIGHT_CTRL_HANDLE_T light_ctrl_handle_data;

static uint32_t blink_time_ms = 0;

static uint32_t g_step_gain = SHADE_STEP_GAIN_DEFAULT;

static FLOAT_T step_gain_float;

static USHORT_T g_shade_off_time = 1000;//default

BOOL_T   level_down_to_off = FALSE;

BOOL_T   mode_change = FALSE;

static MOVE_MODE_T hue_move_mode = MOVE_MODE_UP; //Hue move , UP / DOWN 
static BOOL_T      Hue_loop = FALSE; //Hue loop flag

static USHORT_T    trans_time_100ms = 0; 
static USHORT_T    remain_time_100ms = 0; 

volatile static UCHAR_T    identify_times = 0;
volatile static BOOL_T     identify_processing_flag = FALSE;
volatile static BOOL_T     identify_first_time_flag = TRUE;

extern bool_t user_pwm_init_flag; 
extern bool_t g_pwm_bPolarity;


/**
 * @berief: set shade change step gain
 * @param {IN UINT_T uiGain -> gain}
 * @retval: none
 */
static void light_ctrl_shade_gain_set(UINT_T uiGain)
{
    g_step_gain = uiGain;
    if (g_step_gain < 1) {     /* avoid gain calc too small */
        g_step_gain = 1;
    }
}  

void light_control_stop(void)
{
    remain_time_100ms = 0;
    Hue_loop = TRUE;
    dev_timer_stop(SHADE_TIMER);

    app_print("\r\n Shade STOP  RGB : %d, %d, %d  ",light_ctrl_handle_data.current_val.Red, \
                                            light_ctrl_handle_data.current_val.Green, \
                                            light_ctrl_handle_data.current_val.Blue);
}

/**
 * @berief: set light ctrl data to default according light_cfg_data
 * @param {none}
 * @retval: none
 */
void light_ctrl_data_reset(void)
{
    SEND_QOS_T QOS = QOS_VIP_0;
    USHORT_T delay_ms = 0;
    app_hsv_t report_value;

    memset(&light_ctrl_data, 0, SIZEOF(LIGHT_CTRL_DATA_T));

    light_ctrl_data.onoff_status = TRUE;
    light_ctrl_data.Mode = COLOR_MODE;

    light_ctrl_data.color_hsv.Hue = 500;
    light_ctrl_data.color_hsv.Saturation = 200;
    light_ctrl_data.color_hsv.Value = 300; 

    report_value.H = light_ctrl_data.color_hsv.Hue;
    report_value.S = light_ctrl_data.color_hsv.Saturation;
    report_value.V = light_ctrl_data.color_hsv.Value;

    report_onoff_value(QOS, delay_ms, light_ctrl_data.onoff_status);
    report_mode_value(QOS, delay_ms, light_ctrl_data.Mode);
    report_hsv_value(QOS, delay_ms, &report_value);
    op_light_ctrl_proc();
}

/**
 * @note: set light count down cnt
 * @param [in] {uint32_t}count
 * @return: none
 */
void op_light_count_down_cnt_set(UINT_T count)
{
    light_ctrl_data.count_down = count;
}

/**
 * @note: return op_light_count_down_cnt_get
 * @param [in] none
 * @return: app_light_count_down_cnt_get
 */
uint32_t op_light_count_down_cnt_get(void)
{
    return light_ctrl_data.count_down;
}

/**
 * @description: light control callback,Notify the application layer of the current countdown remaining time
 * @param {remain_time_sec} Countdown remaining time
 * @return: none
 */
void light_data_count_down_response(UINT_T remain_time_sec)
{
    UCHAR_T onoff = 0;
    report_count_down_data(QOS_1, 0 , remain_time_sec);
    
    app_print(" report count down %d ", remain_time_sec);

    if (remain_time_sec != 0) {
        report_count_down_data(QOS_1, 0 , (USHORT_T)remain_time_sec);
    } else {
        op_light_ctrl_data_switch_get(&onoff);
        report_onoff_value(QOS_0, 0 , onoff);
        dev_zigbee_write_attribute(1,\
                                CLUSTER_ON_OFF_CLUSTER_ID,\
                                ATTR_ON_OFF_ATTRIBUTE_ID,\
                                &onoff,\
                                ATTR_INT8U_ATTRIBUTE_TYPE);
    }
    dev_zigbee_write_attribute(1,\
                            CLUSTER_ON_OFF_CLUSTER_ID,\
                            TY_ATTR_COUNT_DOWN_ATTR_ID,\
                            &remain_time_sec,\
                            ATTR_INT32U_ATTRIBUTE_TYPE);                 
}

/**
 * @description: Reset times clear callback, turn on self-recovery
 * @param {type} none
 * @return: none
 */
void light_sys_reset_cnt_clear_callback(void)
{
    nwk_enable_self_recovery_once();
}

/**
 * @description: Determine whether it is a extern reset (power on reset/extern rest pin,et.)
 * @param {type} none
 * @return: TRUE:reset reason is a extern reset, FALSE: not a extern reset
 */
BOOL_T b_light_sys_hw_reboot_jude(void)
{
    return TRUE;
}

/**
 * @description: reset's times exceed callback function, the apl should start connect network
 * @param {none}
 * @return: OPERATE_RET
 */
OPERATE_RET light_sys_reset_cnt_over_callback(void)
{
    PR_ERR("reset cnt over!");
    dev_timer_start_with_callback(NETWORK_JOIN_START_DELAY_TIMER_ID,\
                             NETWORK_JOIN_START_DELAY_TIME,\
                            (timer_func_t)network_join_start_delay_timer_callback);
    return OPRT_OK;
}

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
OPERATE_RET op_light_set_rgb(USHORT_T R_value, USHORT_T G_value, USHORT_T B_value)
{
    if(user_pwm_init_flag != TRUE){
        app_print("user pwm not init!");
        return OPRT_INVALID_PARM;
    }

    if ((R_value > PWM_MAX_DUTY) || (G_value> PWM_MAX_DUTY) || (B_value > PWM_MAX_DUTY)) {
        app_print("USER PWM data is invalid!");
        return OPRT_INVALID_PARM;
    }

    if ( g_pwm_bPolarity == FALSE ) {
        R_value = PWM_MAX_DUTY - R_value;
        G_value = PWM_MAX_DUTY - G_value;
        B_value = PWM_MAX_DUTY - B_value;
    }
    
    if (!set_pwm_duty(0, R_value, PWM_MAX_DUTY)) {
        app_print("PWM_R SET DUTY ERR");
    }
    if (!set_pwm_duty(1, G_value, PWM_MAX_DUTY)) {
        app_print("PWM_G SET DUTY ERR");
    }
    if (!set_pwm_duty(2, B_value, PWM_MAX_DUTY)) {
        app_print("PWM_B SET DUTY ERR");
    }
    
    return OPRT_OK;
}

/**
 * @berief: save light application data
 * @param {IN light_app_data_flash_t *data -> save data}
 * @return: OPERATE_RET
 * @retval: none
 */
static OPERATE_RET op_user_flash_write_app_data(light_app_data_flash_t *data)
{
    UCHAR_T ret = 0;
    USHORT_T i=0;

    USHORT_T len = sizeof(light_app_data_flash_t);
    UINT_T addr = LIGHT_APP_DATA_OFFSET;
    UCHAR_T *flash_data = (UCHAR_T *)data;

    //check space
    if ((addr + len) >= FLASH_DATA_MAX_ADDRESS || addr < FLASH_DATA_START_ADDRESS) {
        app_print("write data size out range !");
        return FALSE;
    }
    //read out firt
    user_flash_data_read(WRITE_DATA, sizeof(WRITE_DATA));

    //set data valid flag
    WRITE_DATA[addr] = 0x5A;

    //fill write buffer from addr + 1
    for(i = 0; i < len; i++) {
        WRITE_DATA[addr + i + 1] = *(flash_data + i);
    }
    ret = user_flash_data_write(WRITE_DATA, sizeof(WRITE_DATA));
    if (ret != sizeof(WRITE_DATA)) {
        app_print("write data error !");
        return OPRT_COM_ERROR;
    }
    return OPRT_OK;
}

/**
 * @berief: Light system control hardware timer callback
 * @param {none}
 * @attention: this function need to implement by system,
 *              decide how to call vLightCtrlShadeGradually function.
 * @retval: none
 */
void light_sys_hardware_timer_callback(void)
{
    PR_ERR("shade tick 5ms...");
}

/**
 * @berief: Light control hardware timer callback
 * @param {none}
 * @attention: light_sys_hardware_timer_callback() func need to implement by system
 * @retval: none
 */
void light_ctrl_hardware_timer_callback(void)
{
    if (hw_timer_param.enable != TRUE) {
        return;
    }

    hw_timer_param.cnt += HW_TIMER_CYCLE_MS;
    if (hw_timer_param.cnt >= hw_timer_param.target_cnt) {
        light_sys_hardware_timer_callback();
        hw_timer_param.cnt = 0;
    }
}

/**
 * @berief: set shade change step gain
 * @param {IN UINT_T uiGain -> gain}
 * @retval: none
 */
static void light_ctrl_shade_step_gain_set(UINT_T uiGain) {
    g_step_gain = uiGain;
    if (g_step_gain < 1) {
        g_step_gain = 1;
    }
}

/**
 * @brief: calculate HSV limit v.
 * @param {IN UINT_T value -> HSV, value}
 * @retval: limit adjust value
 */
static USHORT_T light_ctrl_data_hsv_limit(UINT_T value)
{
    USHORT_T min = 10;        //light hsv -> v the min is 10    -> 1
    USHORT_T max = 1000;      //light hsv -> v the max is 1000  -> 100
    USHORT_T result = 0;

    if(value < CTRL_RGB_BRIGHT_VALUE_MIN){
        value = CTRL_RGB_BRIGHT_VALUE_MIN;
    }

    result = (USHORT_T)(value - CTRL_RGB_BRIGHT_VALUE_MIN) * (max - min) / (CTRL_RGB_BRIGHT_VALUE_MAX - CTRL_RGB_BRIGHT_VALUE_MIN) + min;

    return result;
}


/**
 * @berief: calculate the RGB Light_Handle.TargetVal according to
 *          the Light_Data value.
 * @param {IN COLOR_RGB_T *Color -> RGB ctrl data}
 * @param {IN light_data_t *Result -> Result handle data}
 * @retval: none
 */
void light_ctrl_data_calc_rgb(COLOR_RGB_T *Color, light_data_t *Result)
{
    Result->Red = Color->Red;
    Result->Green = Color->Green;
    Result->Blue = Color->Blue;

    //  output max limit
    if((Color->Red + Color->Green + Color->Blue) > 1000){
		FLOAT_T ratio = 1000 / (FLOAT_T)(Color->Red + Color->Green + Color->Blue);
		FLOAT_T precison_compensate_value = 0.499;//float error compensate

		Result->Red   = (USHORT_T)((Color->Red)*ratio + precison_compensate_value);
		Result->Green = (USHORT_T)((Color->Green)*ratio + precison_compensate_value);
		Result->Blue  = (USHORT_T)((Color->Blue)*ratio + precison_compensate_value);
	}
	app_print("after limit rgb:%d,%d,%d",Result->Red, Result->Green, Result->Blue);
}

/**
 * @berief: set light switch data, adapte control data issued by system
 *          to control data format.
 * @param {IN BOOL_T on_off -> switch data, TRUE will turn on}
 * @retval: OPERATE_RET -> OPRT_OK meaning need to call op_light_ctrl_proc() function!
 */
OPERATE_RET op_light_ctrl_data_switch_set(BOOL_T on_off)
{
    BOOL_T last_status ;

    last_status = light_ctrl_data.onoff_status;

    if ( TRUE == on_off ) {
        light_ctrl_data.onoff_status = TRUE;
    } else {
        light_ctrl_data.onoff_status = FALSE;
    }

    if (on_off == last_status) {
        // PR_ERR("the same switch set");
        return OPRT_INVALID_PARM;
    }

    return OPRT_OK;
}

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
OPERATE_RET op_light_ctrl_data_mode_set(LIGHT_MODE_E Mode)
{
    if ( Mode > MODE_MAX) {
        PR_ERR("mode is illegal,set error");
        return OPRT_INVALID_PARM;
    }

    light_ctrl_data.Mode = Mode;

    return OPRT_OK;
}

/**
 * @description: calculate Gain Step by target_value and Transtion time
 * @param [IN] {target_value} target value from current to target
 * @param [IN] {trans_time_ms} Transition time from current to target
 * @return: Shade gain step
 */
static void shade_gain_and_slope_set(UINT_T target_value, UINT_T trans_time_ms)
{
    if (target_value == 0 || trans_time_ms == 0 || trans_time_ms == 0xFFFF) {
        g_step_gain = SHADE_STEP_GAIN_DEFAULT;
        light_ctrl_handle_data.slope = SHADE_SLOPE_DEFAULT;
        step_gain_float = g_step_gain;

    }else if (target_value <= (trans_time_ms/LIGHT_SHADE_CYCLE)) {
        app_print("target_value less than transtime/cycle");
        g_step_gain = 1;
        light_ctrl_handle_data.slope =  target_value /1.0/ trans_time_ms;
        step_gain_float = 1.0;
    } else {
        step_gain_float = 1.0 * target_value * LIGHT_SHADE_CYCLE / trans_time_ms ;
        g_step_gain = (uint32_t)step_gain_float + 1;
        light_ctrl_handle_data.slope = target_value /1.0/ trans_time_ms;
    }
    if(g_step_gain < 1){
        g_step_gain = 1;
    }
}

/**
 * @berief: Set Bright and HSV->V both
 * @param {Level}  Bright and HSV->V value: 0~1000
 * @param {with_onoff}  TRUE: turn on immediately if level is not 0 or shade to off if level is 0;
 *                     FALSE: don't change onoff,just shade to level.
 * @retval: OPERATE_RET: OPRT_OK / OPRT_INVALID_PARM
 */
OPERATE_RET op_light_ctrl_data_level_set(USHORT_T Level, USHORT_T trans_time)
{   
    STATIC USHORT_T Diff = 0;
    USHORT_T last_Level;

    trans_time_100ms = trans_time;

    last_Level = light_ctrl_handle_data.current_hsv.Value;

    if(Level < CTRL_CW_BRIGHT_VALUE_MIN) {
        app_print("bright value is exceed range,set error");
        return OPRT_INVALID_PARM;
    }

    if(Level > CTRL_CW_BRIGHT_VALUE_MAX) {
        app_print("bright value is exceed range,set error");
        return OPRT_INVALID_PARM;
    }

    light_ctrl_data.color_hsv.Value = Level;
    // light_ctrl_handle_data.target_hsv.Value = Level;
        
    if(Level == last_Level) {
        app_print("the same bright set");
        return OPRT_INVALID_PARM;
    }
    Diff = (USHORT_T)light_tool_get_abs_value(Level - last_Level);
    light_ctrl_handle_data.shade_param = SHADE_PARAM_V;
    
    if(remain_time_100ms != 0 && light_ctrl_handle_data.shade_param != SHADE_PARAM_V){
        light_ctrl_handle_data.shade_param = SHADE_PARAM_RGBCW2;
    }else{
        light_ctrl_handle_data.shade_param = SHADE_PARAM_V;
    }

    USHORT_T value_limited = light_ctrl_data_hsv_limit(light_ctrl_data.color_hsv.Value);
    light_tool_HSV_2_RGB(light_ctrl_data.color_hsv.Hue,\
                    light_ctrl_data.color_hsv.Saturation,\
                    value_limited,\
                    &light_ctrl_data.Color.Red,\
                    &light_ctrl_data.Color.Green,\
                    &light_ctrl_data.Color.Blue);

    shade_gain_and_slope_set(Diff, trans_time*100);

    return OPRT_OK;
}

/**
 * @berief: set light RGB data
 * @param {IN COLOR_RGB_T *Color}
 * @param {IN COLOR_ORIGINAL_S *ColorOrigin -> color origin data save}
 * @attention: acceptable format is RGB module, R,G,B range:0~1000
 * @retval: OPERATE_RET -> OPRT_OK meaning need to call opLightCtrlProc() function!
 */
OPERATE_RET op_light_ctrl_data_rgb_set(COLOR_RGB_T *color_rgb)
{
    if ((color_rgb ->Red > CTRL_CAL_VALUE_RANGE) ||
        (color_rgb ->Green > CTRL_CAL_VALUE_RANGE) ||
        (color_rgb ->Blue > CTRL_CAL_VALUE_RANGE)) {
        PR_ERR("RGB color_rgb value is exceed range,set error");
        return OPRT_INVALID_PARM;
    }
    light_ctrl_data.color_mode = COLOR_MODE_RGB;
    memcpy(&light_ctrl_data.Color, color_rgb, SIZEOF(COLOR_RGB_T));

    return OPRT_OK;
}


/**
 * @berief: set light RGB data
 * @param {IN COLOR_RGB_T *Color}
 * @param {IN COLOR_ORIGINAL_S *ColorOrigin -> color origin data save}
 * @attention: acceptable format is RGB module, R,G,B range:0~1000
 * @retval: OPERATE_RET -> OPRT_OK meaning need to call opLightCtrlProc() function!
 */
OPERATE_RET op_light_ctrl_data_hsv_set(COLOR_HSV_T *color_hsv)
{
    COLOR_HSV_T hsv_value;
    USHORT_T limit_value = 0;

    if ( color_hsv == NULL ) {
        return OPRT_INVALID_PARM;
    }

    hsv_value.Hue = color_hsv->Hue;
    hsv_value.Saturation = color_hsv->Saturation;
    hsv_value.Value = color_hsv->Value;
    // PR_ERR("it's the hsv_set , H:%d, S:%d, V:%d\r\n", hsv_value.Hue, hsv_value.Saturation, hsv_value.Value);

    if (hsv_value.Hue > CTRL_HSV_H_MAX) {
        hsv_value.Hue = CTRL_HSV_H_MAX;
        app_print("usHue bigger than CTRL_HSV_H_MAX");
    }

    if (hsv_value.Saturation > CTRL_CAL_VALUE_RANGE) {
        hsv_value.Saturation = CTRL_CAL_VALUE_RANGE;
        app_print("usSaturation bigger than CTRL_CAL_VALUE_RANGE");
    }

    if (hsv_value.Value > CTRL_CAL_VALUE_RANGE) {
        hsv_value.Value = CTRL_CAL_VALUE_RANGE;
        app_print("usValue bigger than CTRL_CAL_VALUE_RANGE");
    }

    if (hsv_value.Value < CTRL_RGB_BRIGHT_VALUE_MIN) {
        app_print("The HSV->value is not support");
        return OPRT_INVALID_PARM;
    }
    memcpy(&light_ctrl_data.color_hsv, &hsv_value, SIZEOF(COLOR_HSV_T));
    light_tool_HSV_2_RGB(light_ctrl_data.color_hsv.Hue,\
                          light_ctrl_data.color_hsv.Saturation,\
                          limit_value,\
                          &light_ctrl_data.Color.Red,\
                          &light_ctrl_data.Color.Green,\
                          &light_ctrl_data.Color.Blue);

    app_print("light_tool_HSV_2_RGB H:%d, S:%d, V:%d\r\n", light_ctrl_data.color_hsv.Hue, light_ctrl_data.color_hsv.Saturation, light_ctrl_data.color_hsv.Value);
    light_ctrl_data.color_mode = COLOR_MODE_HSV;

    if (memcmp(&hsv_value, &light_ctrl_handle_data.current_hsv,sizeof(COLOR_HSV_T)) == 0 ) {
        app_print("The same HSV set");
        return OPRT_INVALID_PARM;
    }

    if (light_ctrl_data.color_hsv.Hue == hsv_value.Hue && \
       light_ctrl_data.color_hsv.Saturation == hsv_value.Saturation && \
       light_ctrl_data.color_hsv.Value != hsv_value.Value)
    {
        light_ctrl_handle_data.shade_param = SHADE_PARAM_V;
        app_print("shade_param >>> V ");

    } else {
        light_ctrl_handle_data.shade_param = SHADE_PARAM_RGBCW;
        app_print("shade_param >>> RGBCW");
    }

    USHORT_T Diff = (USHORT_T)light_tool_get_abs_value(hsv_value.Value - light_ctrl_handle_data.current_hsv.Value);
    shade_gain_and_slope_set(Diff, 0);
    
    limit_value = light_ctrl_data_hsv_limit(hsv_value.Value);
    light_tool_HSV_2_RGB(light_ctrl_data.color_hsv.Hue,\
                          light_ctrl_data.color_hsv.Saturation,\
                          limit_value,\
                          &light_ctrl_data.Color.Red,\
                          &light_ctrl_data.Color.Green,\
                          &light_ctrl_data.Color.Blue);
    app_print("the value of RGB: R:%d, G:%d, B:%d", light_ctrl_data.Color.Red, light_ctrl_data.Color.Green, light_ctrl_data.Color.Blue);
    return OPRT_OK;
}


/**
 * @description: Move to hue and saturation with transition time handler
 * @param {Hue} Target Hue
 * @param {Saturation} Target Saturation
 * @param {trans_time}  Transtion time in 100ms
 * @return: OPERATE_RET: OPRT_OK / OPRT_INVALID_PARM
 */
OPERATE_RET op_light_ctrl_data_hs_set(USHORT_T Hue, USHORT_T Saturation, USHORT_T trans_time)
{
    USHORT_T sat_save = Saturation;

    trans_time_100ms = trans_time;

    light_ctrl_data.color_hsv.Hue = Hue;
    light_ctrl_data.color_hsv.Saturation = sat_save;

    if (Hue == light_ctrl_handle_data.current_hsv.Hue && Saturation == light_ctrl_handle_data.current_hsv.Saturation) {
        app_print("The same Hue and Saturation set");
        return OPRT_INVALID_PARM;
    }
    if (sat_save > CTRL_CAL_VALUE_RANGE) {
        sat_save = CTRL_CAL_VALUE_RANGE;
    }

    Hue_loop = FALSE;
    light_ctrl_data.Mode = COLOR_MODE;
    light_ctrl_data.color_mode = COLOR_MODE_HSV;
    light_ctrl_handle_data.target_hsv.Hue = Hue;
    light_ctrl_handle_data.target_hsv.Saturation = sat_save;

    mode_change = TRUE;
    light_ctrl_handle_data.shade_param = SHADE_PARAM_RGBCW2;
    // light_ctrl_handle_data.shade_param = SHADE_PARAM_RGBCW;
    return OPRT_OK;
}

/**
 * @description: Move to Hue with direction and transition time handler
 * @param {Hue} Target Hue:0~360, other value has no effect.
 * @param {Direction} UP / DOWN
 * @param {trans_time} Transtion time in 100ms,0:
*  @return: OPERATE_RET: OPRT_OK / OPRT_INVALID_PARM
 */
OPERATE_RET op_light_ctrl_data_hue_set(USHORT_T Hue, MOVE_MODE_T move_mode, USHORT_T trans_time)
{
    USHORT_T Diff;

    trans_time_100ms = trans_time;

    light_ctrl_data.color_hsv.Hue = Hue;
    light_ctrl_data.color_mode = COLOR_MODE_HSV;

     if (Hue == light_ctrl_handle_data.current_hsv.Hue && light_ctrl_data.color_mode == COLOR_MODE_HSV) {
        app_print("Hue is same");
        return OPRT_INVALID_PARM;
    }
    if (light_ctrl_handle_data.current_hsv.Hue == 360) {
        light_ctrl_handle_data.current_hsv.Hue = 0;
    }

    if (move_mode == MOVE_MODE_UP) {
        if (Hue > light_ctrl_handle_data.current_hsv.Hue) {
            Diff = Hue - light_ctrl_handle_data.current_hsv.Hue;
        } else {
            Diff = 360 - (light_ctrl_handle_data.current_hsv.Hue - Hue);
        }
    } else {
        if (Hue > light_ctrl_handle_data.current_hsv.Hue) {
            Diff = 360 - (Hue - light_ctrl_handle_data.current_hsv.Hue);
        } else {
            Diff = light_ctrl_handle_data.current_hsv.Hue - Hue;
        }
    }
    Hue_loop = FALSE;
    light_ctrl_data.color_hsv.Hue = Hue;
    light_ctrl_data.color_mode = COLOR_MODE_HSV;

    light_ctrl_handle_data.target_hsv.Hue = Hue;
    light_ctrl_handle_data.shade_param = SHADE_PARAM_H_LOOP;
    hue_move_mode = move_mode;
    
    shade_gain_and_slope_set(light_tool_get_abs_value(Diff), trans_time);

    return OPRT_OK;
}

/**
 * @description: Move to Saturation with transition time handler
 * @param {Saturation} Target Saturation
 * @param {trans_time} Transtion time in 100ms
*  @return: OPERATE_RET: OPRT_OK / OPRT_INVALID_PARM
 */
OPERATE_RET op_light_ctrl_data_saturation_set(USHORT_T Saturation, USHORT_T trans_time)
{
    USHORT_T sat_save = Saturation;

    trans_time_100ms = trans_time;

    light_ctrl_data.color_hsv.Saturation = sat_save;
    light_ctrl_data.color_mode = COLOR_MODE_HSV;

    if (sat_save == light_ctrl_handle_data.current_hsv.Saturation && light_ctrl_data.color_mode == COLOR_MODE_HSV) {
        PR_ERR("Saturation is same");
        return OPRT_INVALID_PARM;
    }

    if (sat_save > CTRL_CAL_VALUE_RANGE) {
        sat_save = CTRL_CAL_VALUE_RANGE;
    }

    USHORT_T Diff = (USHORT_T)light_tool_get_abs_value(sat_save - light_ctrl_handle_data.current_hsv.Saturation);
    shade_gain_and_slope_set(Diff, trans_time*100);

    light_ctrl_handle_data.target_hsv.Saturation = sat_save;
    light_ctrl_handle_data.shade_param = SHADE_PARAM_S;

    return OPRT_OK;
}


/**
 * @description: Bright and HSV->V move up or move down
 * @param {move_mode} level move mode :up / down
 * @param {Rate} move rate,if 0x00/0xFF: 1/ms, else Rate/S
 * @param {with_onoff} move mode :up / down
 * @return: none
 */
OPERATE_RET op_light_ctrl_data_level_move(MOVE_MODE_T move_mode, USHORT_T Rate, BOOL_T with_onoff)
{
    USHORT_T set_level;
    USHORT_T Diff;
    USHORT_T trans_time_100ms;
    USHORT_T move_rate = Rate;

    if (Rate == 0 || Rate >= 1000) {
        move_rate = 1000 * SHADE_STEP_GAIN_DEFAULT / LIGHT_SHADE_CYCLE;
        if (move_rate == 0) {
            move_rate = 1000;
        }
    }

    if (move_mode == MOVE_MODE_UP) {
        set_level = 1000;
        level_down_to_off = FALSE;
        if (with_onoff) {
            if (light_ctrl_data.onoff_status != TRUE) {
                light_ctrl_data.onoff_status = TRUE;
            }
        }
    } else {
        set_level = 10;
        if (with_onoff) {
            level_down_to_off = TRUE;
        } else {
            level_down_to_off = FALSE;
        }
    }

    if (light_ctrl_data.Mode == COLOR_MODE) {
        Diff = (USHORT_T)light_tool_get_abs_value(set_level- light_ctrl_handle_data.current_hsv.Value);
    }

    trans_time_100ms = Diff * 10 / move_rate;
    op_light_ctrl_data_level_set(set_level, trans_time_100ms);
    
    return OPRT_OK;
}

/**
 * @berief: get light switch data
 * @param {OUT BOOL_T *onpONOFFoff -> switch data return}
 * @retval: OPERATE_RET
 */
OPERATE_RET op_light_ctrl_data_switch_get(BOOL_T *pONOFF)
{
    *pONOFF = light_ctrl_data.onoff_status;

    return OPRT_OK;
}

/**
 * @berief: get light RGB data
 * @param {OUT COLOR_RGB_T *Color -> color original data return}
 * @retval: OPERATE_RET
 */
OPERATE_RET op_light_ctrl_data_rgb_get(COLOR_RGB_T *color_rgb)
{
    if (color_rgb == NULL) {
        PR_ERR("RGB get param error");
        return OPRT_COM_ERROR;
    }
    memcpy(color_rgb, &light_ctrl_data.Color , SIZEOF(COLOR_RGB_T));
    return OPRT_OK;
}
/**
 * @berief: get light mode data
 * @param {OUT LIGHT_MODE_E *Mode -> mode data return}
 * @retval: OPERATE_RET
 */
OPERATE_RET op_light_ctrl_data_mode_get(LIGHT_MODE_E *Mode)
{
    *Mode = light_ctrl_data.Mode;

    return OPRT_OK;
}

/**
 * @berief: get light color mode
 * @param {OUT COLOR_MODE_E *Mode -> mode data return}
 * @retval: OPERATE_RET
 */
OPERATE_RET op_light_ctrl_data_color_mode_get(COLOR_MODE_E *Mode)
{
    *Mode = light_ctrl_data.color_mode;

    return OPRT_OK;
}

/**
 * @berief: Get light HSV data
 * @param {OUT COLOR_HSV_T *color_hsv}  HSV data
 * @retval: OPERATE_RET: OPRT_OK / OPRT_INVALID_PARM
 */
OPERATE_RET op_light_ctrl_data_hsv_get(COLOR_HSV_T *color_hsv)
{
    if (color_hsv == NULL) {
        PR_ERR("HSV get param error");
        return OPRT_COM_ERROR;
    }
    memcpy(color_hsv, &light_ctrl_data.color_hsv , SIZEOF(COLOR_HSV_T));
    return OPRT_OK;
}

/**
 * @berief: get control ctrl data
 * @param {none}
 * @retval: LIGHT_CTRL_DATA_T
 */
LIGHT_CTRL_DATA_T* op_light_ctrl_data_get(void)
{

    return &light_ctrl_data;
}

/**
 * @berief: get change gradually process the max error of 5ways
 *          this func will calc the error between LightCtrlHandle.Target
 *          and LightCtrlHandle.Current, and change.
 * @param {none}
 * @attention: this func need to called by period
 * @retval: none
 */
static void light_ctrl_shade_gradually(void)
{
    int diff_Red = 0, diff_Green = 0, diff_Blue = 0;
    
    USHORT_T diff_max = 0;

    static float fscale_Red = 0;
    static float fscale_Green = 0;
    static float fscale_Blue = 0;

    UINT_T step_Red = 0;
    UINT_T step_Green = 0;
    UINT_T step_Blue = 0;

    UINT_T step_gain = 0;
    UCHAR_T udata = 0;

    dev_timer_start_with_callback(SHADE_TIMER, LIGHT_SHADE_CYCLE, (timer_func_t)light_ctrl_shade_gradually);

    step_gain = g_step_gain;

    if (memcmp(&light_ctrl_handle_data.target_val, &light_ctrl_handle_data.current_val, SIZEOF(light_data_t)) != 0) {
        diff_Red = light_ctrl_handle_data.target_val.Red - light_ctrl_handle_data.current_val.Red;
        diff_Green = light_ctrl_handle_data.target_val.Green - light_ctrl_handle_data.current_val.Green;
        diff_Blue = light_ctrl_handle_data.target_val.Blue - light_ctrl_handle_data.current_val.Blue;

        diff_max = (USHORT_T)light_tool_get_max_value(\
                    light_tool_get_abs_value(light_ctrl_handle_data.target_val.Red - light_ctrl_handle_data.current_val.Red),\
                    light_tool_get_abs_value(light_ctrl_handle_data.target_val.Red - light_ctrl_handle_data.current_val.Red),\
                    light_tool_get_abs_value(light_ctrl_handle_data.target_val.Red - light_ctrl_handle_data.current_val.Red));
        if( TRUE == light_ctrl_handle_data.first_change ) {
            fscale_Red = light_tool_get_abs_value(diff_Red) / 1.0 / diff_max;
            fscale_Green = light_tool_get_abs_value(diff_Green) / 1.0 / diff_max;
            fscale_Blue = light_tool_get_abs_value(diff_Blue) / 1.0 / diff_max;
            light_ctrl_handle_data.first_change = FALSE;
        }
        remain_time_100ms = 3 * light_tool_get_abs_value(diff_max) / step_gain /100 ;

        if( diff_max == light_tool_get_abs_value(diff_Red) ) {
            step_Red = step_gain;
        } else {
            udata = ( light_tool_get_abs_value(diff_Red) - diff_max * fscale_Red );
            (udata < step_gain) ? (step_Red = 0) : (step_Red = step_gain);
        }

        if( diff_max == light_tool_get_abs_value(diff_Green) ) {
            step_Green = step_gain;
        } else {
            udata = ( light_tool_get_abs_value(diff_Green) - diff_max * fscale_Green );
            (udata < step_gain) ? (step_Green = 0) : (step_Green = step_gain);
        }

        if( diff_max == light_tool_get_abs_value(diff_Blue) ) {
            step_Blue = step_gain;
        } else {
            udata = ( light_tool_get_abs_value(diff_Blue) - diff_max * fscale_Blue );
            (udata < step_gain) ? (step_Blue = 0) : (step_Blue = step_gain);
        }

        if( diff_Red != 0 ) {
            if( light_tool_get_abs_value(diff_Red) < step_Red ) {
                light_ctrl_handle_data.current_val.Red += diff_Red;
            } else {
                if( diff_Red < 0 ){
                    light_ctrl_handle_data.current_val.Red -= step_Red;
                } else {
                    light_ctrl_handle_data.current_val.Red += step_Red;
                }
            }
        }

        if( diff_Green != 0 ) {
            if( light_tool_get_abs_value(diff_Green) < step_Green ) {
                light_ctrl_handle_data.current_val.Green += diff_Green;
            } else {
                if( diff_Green < 0 ){
                    light_ctrl_handle_data.current_val.Green -= step_Green;
                } else {
                    light_ctrl_handle_data.current_val.Green += step_Green;
                }
            }
        }

        if( diff_Blue != 0 ) {
            if( light_tool_get_abs_value(diff_Blue) < step_Blue ) {
                light_ctrl_handle_data.current_val.Blue += diff_Blue;
            } else {
                if( diff_Blue < 0 ){
                    light_ctrl_handle_data.current_val.Blue -= step_Blue;
                } else {
                    light_ctrl_handle_data.current_val.Blue += step_Blue;
                }
            }
        }
        op_light_set_rgb(light_ctrl_handle_data.current_val.Red,\
                         light_ctrl_handle_data.current_val.Green,\
                         light_ctrl_handle_data.current_val.Blue);
    } else {
        light_control_stop();
        memcpy(&light_ctrl_handle_data.current_val, &light_ctrl_handle_data.target_val, sizeof(light_data_t));
        memcpy(&light_ctrl_handle_data.current_hsv, &light_ctrl_handle_data.target_hsv,sizeof(COLOR_HSV_T));

        op_light_set_rgb(light_ctrl_handle_data.target_val.Red,\
                         light_ctrl_handle_data.target_val.Green,\
                         light_ctrl_handle_data.target_val.Blue);
        op_light_ctrl_data_auto_save();
    }
}


static void light_control_shade_param(void)
{
    OPERATE_RET opRet = -1;
    COLOR_RGB_T rgb;
    light_data_t shade_rgbcw_data;
    static uint32_t shade_cnt = 0;
    static uint32_t shade_limit_cnt = 0;
    static uint32_t step_cnt = 0;

    static uint32_t current_shade_value = 0;
    static FLOAT_T current_shade_value_float = 0;
    static uint32_t target_shade_value = 0;

    static FLOAT_T target_gain = 0;
    USHORT_T actual_gain = 0;
    int16_t level_error = 0;

    dev_timer_start_with_callback(SHADE_TIMER, LIGHT_SHADE_CYCLE, (timer_func_t)light_control_shade_param);

    memset(&shade_rgbcw_data, 0 ,sizeof(light_data_t));
    app_print("\r\n shade_param = %x ", light_ctrl_handle_data.shade_param);
    if(light_ctrl_handle_data.first_change){
        light_ctrl_handle_data.first_change = FALSE;
        switch (light_ctrl_handle_data.shade_param){
            case SHADE_PARAM_S:{
                current_shade_value = light_ctrl_handle_data.current_hsv.Saturation;
                target_shade_value = light_ctrl_handle_data.target_hsv.Saturation;             
                current_shade_value_float = current_shade_value;
                app_print("Saturation shade: %d >> %d", current_shade_value, target_shade_value);
                break;
            }
            case SHADE_PARAM_V:{
                current_shade_value = light_ctrl_handle_data.current_hsv.Value;
                target_shade_value = light_ctrl_handle_data.target_hsv.Value;
                current_shade_value_float = current_shade_value;
                app_print("Value shade: %d >> %d", current_shade_value, target_shade_value);
                break;
            }
            default:{
                shade_cnt = 0;
                step_cnt = 0;
                return;
                }
        }
        shade_cnt = 0;
        step_cnt = 0;
        app_print("level_error: %d", (current_shade_value - target_shade_value));
    }

    level_error = current_shade_value - target_shade_value;

    if(level_error != 0){
        remain_time_100ms = light_tool_get_abs_value(level_error) * (1.0) / light_ctrl_handle_data.slope / 100; 

        shade_cnt++;
        target_gain = light_ctrl_handle_data.slope * (1.0) * (shade_cnt * LIGHT_SHADE_CYCLE);
        actual_gain = step_cnt * ((int16_t)step_gain_float);
                                            
        if((target_gain - actual_gain) >= step_gain_float){                        
            step_cnt++;
            if(light_tool_get_abs_value(level_error) <= step_gain_float) {
                current_shade_value = target_shade_value;
                step_cnt = 0;
                shade_cnt = 0;
            } else {
                if(level_error < 0) {
                    current_shade_value_float += step_gain_float;
                    current_shade_value = (uint32_t) current_shade_value_float;
                } else {
                    current_shade_value_float -= step_gain_float;
                    current_shade_value = (uint32_t) current_shade_value_float;
                }
            }

            switch (light_ctrl_handle_data.shade_param)
            {
                case SHADE_PARAM_S:{
                    light_ctrl_handle_data.current_hsv.Saturation = current_shade_value;  
                    light_tool_HSV_2_RGB(light_ctrl_handle_data.current_hsv.Hue,\
                                        light_ctrl_handle_data.current_hsv.Saturation,\
                                        light_ctrl_handle_data.current_hsv.Value,\
                                        &shade_rgbcw_data.Red,\
                                        &shade_rgbcw_data.Green,\
                                        &shade_rgbcw_data.Blue);          
                    rgb.Red = shade_rgbcw_data.Red;
                    rgb.Green = shade_rgbcw_data.Green;
                    rgb.Blue = shade_rgbcw_data.Blue;
                    // light_ctrl_data_calc_rgb(&rgb, &shade_rgbcw_data);
                    break;
                }
                case SHADE_PARAM_V:{
                    USHORT_T value_Limit;
                    light_ctrl_handle_data.current_hsv.Value = current_shade_value;
                    value_Limit = light_ctrl_data_hsv_limit(current_shade_value);
                    light_tool_HSV_2_RGB(light_ctrl_handle_data.current_hsv.Hue,\
                                        light_ctrl_handle_data.current_hsv.Saturation,\
                                        value_Limit,\
                                        &shade_rgbcw_data.Red,\
                                        &shade_rgbcw_data.Green,\
                                        &shade_rgbcw_data.Blue);  
                    rgb.Red = shade_rgbcw_data.Red;
                    rgb.Green = shade_rgbcw_data.Green;
                    rgb.Blue = shade_rgbcw_data.Blue;
                    app_print("\r\n ----  shande value = %d ---- ", value_Limit);
                    break;
                }        
                default:
                    break;
            }


            memcpy(&light_ctrl_handle_data.current_val, &shade_rgbcw_data, sizeof(light_data_t));
            opRet = op_light_set_rgb(rgb.Red, rgb.Green, rgb.Blue);
            // app_print("\r\n ---------------- it's in light_control_shade_param R:%d, G:%d, B:%d  -----\r\n", rgb.Red, rgb.Green, rgb.Blue);
        
            shade_limit_cnt++;
            if(shade_limit_cnt > 600){
                app_print("WATCHDOG_RELOAD");
                watchdog_reload(); 
                shade_limit_cnt = 0;
            }
            if(opRet != OPRT_OK){
                PR_ERR("Set RGBCW Error"); 
            }

        }
    }
    else{  //no need to proc
        app_print("NO LEVEL ERROR");
        shade_limit_cnt = 0;
        memcpy(&light_ctrl_handle_data.current_hsv, &light_ctrl_handle_data.target_hsv, sizeof(COLOR_HSV_T));
        light_control_stop();

        light_ctrl_handle_data.slope = SHADE_SLOPE_DEFAULT;
        g_step_gain = SHADE_STEP_GAIN_DEFAULT;
        if(FALSE == light_ctrl_data.onoff_status  && identify_processing_flag != TRUE){
            opRet = op_light_set_rgb(0,0,0);
            if(opRet != OPRT_OK) {
                  PR_ERR("SET RGBCW error %d!",opRet); 
                }
        } 
        else{
            if(level_down_to_off == TRUE){
                level_down_to_off = FALSE;
                opRet = op_light_set_rgb(0,0,0);
                if(opRet != OPRT_OK) {
                    PR_ERR("SET RGBCW error %d!",opRet); 
                }
                
                op_light_ctrl_data_count_down_set(0);

                light_ctrl_data.onoff_status = FALSE;   
                light_ctrl_handle_data.current_hsv.Value = 0;
                memset(&light_ctrl_handle_data.current_val,0 ,sizeof(light_data_t));
                //vLightDataSwitchChangedCallback(FALSE);   ///< on off done todo
            } else {
                #if 0
                if(FALSE == identify_processing_flag){
                    app_light_set_rgbcw(light_ctrl_handle_data.target_val.red, \
                                    light_ctrl_handle_data.target_val.green,\
                                    light_ctrl_handle_data.target_val.blue,\
                                    light_ctrl_handle_data.target_val.white,\
                                    light_ctrl_handle_data.target_val.warm);

                }
                #endif
            }
        }
        app_print("\r\n  shade Light_ctrl_data.color_hsv.Value = %d\r\n", light_ctrl_data.color_hsv.Value);
        op_light_ctrl_data_auto_save();
    }
}


/**
 * @description: loop hue move mode
 * @param {type} none
 * @return: none
 */
static void light_ctrl_shade_hue(void)
{
    light_data_t shade_rgb;
    static UINT_T shade_limit = 0;
    static UINT_T shade_cnt = 0;
    static UINT_T step_cnt = 0;
    static FLOAT_T f_gain_target = 0;
    SHORT_T f_gain_actual = 0;
    SHORT_T level_error = 0;
    COLOR_RGB_T rgb;

    memset(&shade_rgb, 0, sizeof(light_data_t));

    dev_timer_start_with_callback(SHADE_TIMER, LIGHT_SHADE_CYCLE, (timer_func_t)light_ctrl_shade_hue);

    if(hue_move_mode == MOVE_MODE_UP){
        if(light_ctrl_handle_data.first_change){
            light_ctrl_handle_data.first_change = FALSE;
            shade_cnt = 0;
            step_cnt = 0;
            if(light_ctrl_handle_data.current_hsv.Hue == 360){
                light_ctrl_handle_data.current_hsv.Hue = 0;
            }
        }
    } else {
        if(light_ctrl_handle_data.first_change){
            light_ctrl_handle_data.first_change = FALSE;
            shade_cnt = 0;
            step_cnt = 0;
            if(light_ctrl_handle_data.current_hsv.Hue == 0){
                light_ctrl_handle_data.current_hsv.Hue = 360;
            }
        }
    }

    if(Hue_loop == TRUE) {
        shade_cnt++;
        f_gain_target = light_ctrl_handle_data.slope * (1.0) * (shade_cnt * LIGHT_SHADE_CYCLE);
        f_gain_actual = step_cnt * g_step_gain;
        if(f_gain_target - f_gain_actual >= g_step_gain) {
            step_cnt++;
            if(hue_move_mode == MOVE_MODE_UP) {
                light_ctrl_handle_data.current_hsv.Hue += g_step_gain;
                if(light_ctrl_handle_data.current_hsv.Hue >= 360) {
                    light_ctrl_handle_data.current_hsv.Hue = 0;
                    shade_cnt = 0;
                    step_cnt = 0;
                }
            } else {
                if(light_ctrl_handle_data.current_hsv.Hue >= g_step_gain) {
                    light_ctrl_handle_data.current_hsv.Hue -= g_step_gain;
                } else {
                    light_ctrl_handle_data.current_hsv.Hue = 360;
                    shade_cnt = 0;
                    step_cnt = 0;
                }
            }
        }

        light_tool_HSV_2_RGB(light_ctrl_handle_data.current_hsv.Hue,\
                            light_ctrl_data.color_hsv.Saturation,\
                            light_ctrl_data.color_hsv.Value,\
                            &shade_rgb.Red,\
                            &shade_rgb.Green,\
                            &shade_rgb.Blue);
        memcpy(&light_ctrl_handle_data.current_val, &shade_rgb, sizeof(light_data_t));
        rgb.Red = shade_rgb.Red;
        rgb.Green = shade_rgb.Green;
        rgb.Blue = shade_rgb.Blue;
        op_light_set_rgb(rgb.Red, rgb.Green, rgb.Blue);
        app_print("\r\n -- hue loop R = %d, G = %d, B = %d", shade_rgb.Red, shade_rgb.Green, shade_rgb.Blue);
    } else {
        level_error = light_ctrl_handle_data.current_hsv.Hue - light_ctrl_handle_data.target_hsv.Hue;
        if(level_error != 0) {
            remain_time_100ms = light_tool_get_abs_value(level_error) * (1.0) / light_ctrl_handle_data.slope /100;

            shade_cnt++;
            f_gain_target = light_ctrl_handle_data.slope * (1.0) * (shade_cnt * LIGHT_SHADE_CYCLE);
            f_gain_actual = step_cnt * g_step_gain;

            if(f_gain_target - f_gain_actual >= g_step_gain) {
                step_cnt++;
                if(hue_move_mode == MOVE_MODE_UP) {
                    USHORT_T hue_temp = light_ctrl_handle_data.current_hsv.Hue + g_step_gain;
                    if(hue_temp > 360) {
                        hue_temp -= 360; 
                    }
                    level_error = hue_temp - light_ctrl_handle_data.target_hsv.Hue;
                    if(light_tool_get_abs_value(level_error) == 360 || light_tool_get_abs_value(level_error) <= g_step_gain) {
                        light_ctrl_handle_data.current_hsv.Hue = light_ctrl_handle_data.target_hsv.Hue;
                        step_cnt = 0;
                        shade_cnt = 0;
                    } else {
                        light_ctrl_handle_data.current_hsv.Hue = hue_temp;
                    }
                } else {
                    SHORT_T hue_temp = light_ctrl_handle_data.current_hsv.Hue - g_step_gain;
                    if(hue_temp < 0) {
                        hue_temp = 360 - (g_step_gain - light_ctrl_handle_data.current_hsv.Hue);
                    }
                    level_error = hue_temp - light_ctrl_handle_data.target_hsv.Hue;

                    if(light_tool_get_abs_value(level_error) == 360 || light_tool_get_abs_value(level_error) <= g_step_gain) {
                        light_ctrl_handle_data.current_hsv.Hue = light_ctrl_handle_data.target_hsv.Hue;
                        step_cnt = 0;
                        shade_cnt = 0;
                    } else {
                        light_ctrl_handle_data.current_hsv.Hue = hue_temp;
                    }
                }
            }
            light_tool_HSV_2_RGB(light_ctrl_handle_data.current_hsv.Hue,\
                            light_ctrl_data.color_hsv.Saturation,\
                            light_ctrl_data.color_hsv.Value,\
                            &shade_rgb.Red,\
                            &shade_rgb.Green,\
                            &shade_rgb.Blue);
            memcpy(&light_ctrl_handle_data.current_val, &shade_rgb, sizeof(light_data_t));
            op_light_set_rgb(shade_rgb.Red, shade_rgb.Green, shade_rgb.Blue);
            shade_limit++;
            if(shade_limit > 400) {  //2S = 400 * LIGHT_SHADE_CYCLE
                watchdog_reload();
            }
        } else {
            shade_limit = 0;
            light_control_stop();
            memcpy(&light_ctrl_handle_data.current_hsv, &light_ctrl_handle_data.target_hsv,sizeof(COLOR_HSV_T));

            op_light_ctrl_data_auto_save();

        }
    }
}


/**
 * @description: RGBCW with transtime
 * @param {type} none
 * @return: none
 */
static void light_ctrl_shade_RGBCW(void)
{
    STATIC UINT_T shade_limit = 0;    
    STATIC UINT_T shade_cnt = 0;         

    STATIC UINT_T shade_cnt_R = 0;        
    STATIC UINT_T shade_cnt_G = 0;      
    STATIC UINT_T shade_cnt_B = 0;       

    INT_T diff_Red = 0;
    INT_T diff_Green = 0;
    INT_T diff_Blue = 0;

    USHORT_T diff_max = 0;

    static float fscale_Red = 0;
    static float fscale_Green = 0;
    static float fscale_Blue = 0;

    UINT_T tar_step_R = 0;
    UINT_T tar_step_G = 0;
    UINT_T tar_step_B = 0;

    UINT_T act_step_R = 0;
    UINT_T act_step_G = 0;
    UINT_T act_step_B = 0;

    dev_timer_start_with_callback(SHADE_TIMER, LIGHT_SHADE_CYCLE, (timer_func_t)light_ctrl_shade_RGBCW);

    if (memcmp(&light_ctrl_handle_data.target_val, &light_ctrl_handle_data.current_val, SIZEOF(light_data_t)) != 0) {
        diff_Red = light_ctrl_handle_data.current_val.Red - light_ctrl_handle_data.target_val.Red;
        diff_Green = light_ctrl_handle_data.current_val.Green - light_ctrl_handle_data.target_val.Green;
        diff_Blue = light_ctrl_handle_data.current_val.Blue - light_ctrl_handle_data.target_val.Blue;

        diff_max = (USHORT_T)light_tool_get_max_value(\
                    light_tool_get_abs_value(light_ctrl_handle_data.target_val.Red - light_ctrl_handle_data.current_val.Red),\
                    light_tool_get_abs_value(light_ctrl_handle_data.target_val.Red - light_ctrl_handle_data.current_val.Red),\
                    light_tool_get_abs_value(light_ctrl_handle_data.target_val.Red - light_ctrl_handle_data.current_val.Red));
        remain_time_100ms = light_tool_get_abs_value(diff_max) * (1.0) /light_ctrl_handle_data.slope /100;
        shade_cnt++;

        if(TRUE == light_ctrl_handle_data.first_change) {
            light_ctrl_handle_data.first_change = FALSE;
            app_print("\r\n  current RGB :%d %d %d ", light_ctrl_handle_data.current_val.Red, light_ctrl_handle_data.current_val.Green, light_ctrl_handle_data.current_val.Blue);
            shade_gain_and_slope_set(light_tool_get_abs_value(diff_max), trans_time_100ms * 100);
            trans_time_100ms = 0;

            fscale_Red = light_tool_get_abs_value(diff_Red) /1.0 / diff_max * light_ctrl_handle_data.slope;
            fscale_Green = light_tool_get_abs_value(diff_Green) /1.0 / diff_max * light_ctrl_handle_data.slope;
            fscale_Blue = light_tool_get_abs_value(diff_Blue) /1.0 / diff_max * light_ctrl_handle_data.slope;
        }
        tar_step_R = fscale_Red * (1.0) * (shade_cnt * LIGHT_SHADE_CYCLE);
        tar_step_G = fscale_Green * (1.0) * (shade_cnt * LIGHT_SHADE_CYCLE);
        tar_step_B = fscale_Blue * (1.0) * (shade_cnt * LIGHT_SHADE_CYCLE);

        act_step_R = shade_cnt_R * g_step_gain;
        act_step_G = shade_cnt_G * g_step_gain;
        act_step_B = shade_cnt_B * g_step_gain;

        if(diff_Red != 0 ){
            if(tar_step_R - act_step_R >= g_step_gain){
                shade_cnt_R++;
                if(light_tool_get_abs_value(diff_Red) <= g_step_gain){
                    light_ctrl_handle_data.current_val.Red = light_ctrl_handle_data.target_val.Red;
                    shade_cnt_R = 0;
                }else{
                    if(diff_Red < 0){
                        light_ctrl_handle_data.current_val.Red += g_step_gain;
                    }else{
                        light_ctrl_handle_data.current_val.Red -= g_step_gain;
                    }
                }
            }
        }

        if(diff_Green != 0 ){
            if(tar_step_G - act_step_G >= g_step_gain){
                shade_cnt_G++;
                if(light_tool_get_abs_value(diff_Green) <= g_step_gain){
                    light_ctrl_handle_data.current_val.Green = light_ctrl_handle_data.target_val.Green;
                    shade_cnt_G = 0;
                }else{
                    if(diff_Green < 0){
                        light_ctrl_handle_data.current_val.Green += g_step_gain;
                    }else{
                        light_ctrl_handle_data.current_val.Green -= g_step_gain;
                    }
                }
            }
        }

        if(diff_Blue != 0 ){
            if(tar_step_B - act_step_B >= g_step_gain){
                shade_cnt_B++;
                if(light_tool_get_abs_value(diff_Blue) <= g_step_gain){
                    light_ctrl_handle_data.current_val.Blue = light_ctrl_handle_data.target_val.Blue;
                    shade_cnt_B = 0;
                }else{
                    if(diff_Blue < 0){
                        light_ctrl_handle_data.current_val.Blue += g_step_gain;
                    }else{
                        light_ctrl_handle_data.current_val.Blue -= g_step_gain;
                    }
                }
            }
        }

        op_light_set_rgb(light_ctrl_handle_data.current_val.Red, light_ctrl_handle_data.current_val.Green, light_ctrl_handle_data.current_val.Blue);

        shade_limit++;
        if(shade_limit > 400){
            watchdog_reload();
            shade_limit = 0;
        }
    } else {
        shade_limit = 0;
        shade_cnt = 0;
        mode_change = FALSE;
        light_control_stop();
        memcpy(&light_ctrl_handle_data.current_hsv, &light_ctrl_handle_data.target_hsv, sizeof(COLOR_HSV_T));
        op_light_set_rgb(light_ctrl_handle_data.current_val.Red, light_ctrl_handle_data.current_val.Green, light_ctrl_handle_data.current_val.Blue);
        PR_DEBUG("\r\n light_ctrl_shade_RGBCW RGBCW : %d, %d, %d",light_ctrl_handle_data.current_val.Red,\
                                              light_ctrl_handle_data.current_val.Green,\
                                              light_ctrl_handle_data.current_val.Blue);
        op_light_ctrl_data_auto_save();
    }   
}


/**
 * @note: shade function
 * @param  none 
 * @retval: none
 */
static void light_control_start(void)
{
    // app_print("\r\n ----- updata light ----- \r\n");
    light_ctrl_handle_data.first_change = TRUE;
    // app_print("shade_param = %d", light_ctrl_handle_data.shade_param);
    
    switch(light_ctrl_handle_data.shade_param){
        case SHADE_PARAM_RGBCW:
        {
            dev_timer_start_with_callback(SHADE_TIMER, 0, (timer_func_t)light_ctrl_shade_gradually);
            break;
        }
        case SHADE_PARAM_S:
        case SHADE_PARAM_V:
        {
            dev_timer_start_with_callback(SHADE_TIMER, 0, (timer_func_t)light_control_shade_param);          
            break;
        }
        case SHADE_PARAM_H_LOOP: 
        {
            dev_timer_start_with_callback(SHADE_TIMER, 0, (timer_func_t)light_ctrl_shade_hue);      
            break;
        }
        case SHADE_PARAM_RGBCW2:
        {
            dev_timer_start_with_callback(SHADE_TIMER, 0, (timer_func_t)light_ctrl_shade_RGBCW);      
            break;
        }

        default:
            break;
    }

}

/**
 * @berief: Light control proc
 * @param {none}
 * @retval: OPERATE_RET
 */
OPERATE_RET op_light_ctrl_proc(void)
{   
    STATIC BOOL_T last_switch_status = FALSE;
    if (light_ctrl_data.onoff_status == FALSE) { 
        light_control_stop();
        memset(&light_ctrl_handle_data.target_val, 0, sizeof(light_data_t));
        light_ctrl_handle_data.shade_param = SHADE_PARAM_V;
        light_ctrl_handle_data.target_hsv.Value = 0;
        light_ctrl_shade_step_gain_set(SHADE_STEP_GAIN_DEFAULT);
        light_ctrl_handle_data.slope = SHADE_SLOPE_DEFAULT;
        shade_gain_and_slope_set(CTRL_CW_BRIGHT_VALUE_MAX, g_shade_off_time);
        light_control_start();
    } else {
        if (last_switch_status != FALSE) {
            memset(&light_ctrl_handle_data.target_val, 0, sizeof(light_data_t));
            USHORT_T limit_value = 0;
            switch(light_ctrl_data.color_mode) {
                case COLOR_MODE_RGB:{
                    light_ctrl_handle_data.target_val.Red = light_ctrl_data.Color.Red;
                    light_ctrl_handle_data.target_val.Green = light_ctrl_data.Color.Green;
                    light_ctrl_handle_data.target_val.Blue = light_ctrl_data.Color.Blue;
                    break;
                }
                case COLOR_MODE_HSV: {
                    limit_value = light_ctrl_data_hsv_limit(light_ctrl_data.color_hsv.Value);
                    light_tool_HSV_2_RGB(light_ctrl_data.color_hsv.Hue, light_ctrl_data.color_hsv.Saturation, limit_value,\
                                    &(light_ctrl_handle_data.target_val.Red),\
                                    &(light_ctrl_handle_data.target_val.Green),\
                                    &(light_ctrl_handle_data.target_val.Blue));
                    break;
                }
            }
            light_control_start();
        } else {
            USHORT_T limit_value = 0;
            light_ctrl_data.color_mode = COLOR_MODE_HSV;
            memset(&light_ctrl_handle_data.target_val, 0, sizeof(light_data_t));
            light_ctrl_handle_data.shade_param = SHADE_PARAM_V;
            switch(light_ctrl_data.color_mode) {
                case COLOR_MODE_RGB:{
                    light_ctrl_handle_data.target_val.Red = light_ctrl_data.Color.Red;
                    light_ctrl_handle_data.target_val.Green = light_ctrl_data.Color.Green;
                    light_ctrl_handle_data.target_val.Blue = light_ctrl_data.Color.Blue;
                    break;
                }
                case COLOR_MODE_HSV: {
                    limit_value = light_ctrl_data_hsv_limit(light_ctrl_data.color_hsv.Value);
                    light_tool_HSV_2_RGB(light_ctrl_data.color_hsv.Hue, light_ctrl_data.color_hsv.Saturation, limit_value,\
                                    &(light_ctrl_handle_data.target_val.Red),\
                                    &(light_ctrl_handle_data.target_val.Green),\
                                    &(light_ctrl_handle_data.target_val.Blue));
                    break;
                }
            }
            light_ctrl_handle_data.current_hsv.Hue = light_ctrl_handle_data.target_hsv.Hue;
            light_ctrl_handle_data.current_hsv.Saturation = light_ctrl_handle_data.target_hsv.Saturation;
            light_ctrl_handle_data.target_hsv.Value = light_ctrl_data.color_hsv.Value;
            light_ctrl_shade_step_gain_set(SHADE_STEP_GAIN_DEFAULT);
            light_ctrl_handle_data.slope = SHADE_SLOPE_DEFAULT;
            shade_gain_and_slope_set(CTRL_CW_BRIGHT_VALUE_MAX, g_shade_off_time);
            light_control_start();
        }
    }
    last_switch_status = light_ctrl_data.onoff_status;
    return OPRT_OK;
}

/**
 * @description: Move to off when shade stop
 * @param {type} bMoveToOff
 * @return: OPRT_OK
 */
void light_ctrl_data_move_to_off_set(void)
{
    level_down_to_off = TRUE;
}


/**
 * @berief: light ctrl breath proc
 * @param {none}
 * @return: OPERATE_RET
 * @retval: none
 */
static void light_ctrl_count_down_timer_callback(void)
{
    OPERATE_RET opRet = -1;
    bool_t onoff = FALSE;
    uint32_t cnt = op_light_count_down_cnt_get();

    if (cnt > 1) {     /* to avoid count_down = 0 %60 also equal to 0 */
        cnt--;
        op_light_count_down_cnt_set(cnt);
        if ((cnt % 60) == 0) {     /* upload countdown value per min */
            light_data_count_down_response(cnt);
        }
    } else {
        op_light_count_down_cnt_set(0);
        op_light_ctrl_data_switch_get(&onoff);

        onoff = (onoff != FALSE) ? FALSE : TRUE;
        op_light_ctrl_data_switch_set(onoff);
        opRet = op_light_ctrl_proc();
        if (opRet != OPRT_OK) {
            PR_ERR("CountDown process error!");
        }
        light_data_count_down_response(0); 
        return; 
    }
    dev_timer_start_with_callback(COUNTDOWN_SW_TIMER, 1000, (timer_func_t)light_ctrl_count_down_timer_callback);
    
}

/**
 * @berief: set light countdown value
 * @param {IN INT_T CountDownSec -> unit:second}
 * @attention: countdown lave time will return with
 *              calling light_data_count_down_response function every minutes.
 *              switch status will return with calling
 *              light_ctrl_data_switch_response function when countdown active.
 * @retval: OPERATE_RET -> OPRT_OK set countdown OK.
 */
OPERATE_RET op_light_ctrl_data_count_down_set(INT_T CountDownSec)
{

    if ((CountDownSec < 0)|| (CountDownSec > 86400)) {
        PR_ERR("Set countdwon value error!");
        return OPRT_INVALID_PARM;
    }
    op_light_count_down_cnt_set(CountDownSec);
    if (CountDownSec <= 0) {
        dev_timer_stop(COUNTDOWN_SW_TIMER);      /* cancel coutdown proc */
        light_data_count_down_response(0);
        return OPRT_OK;

    } else {
        dev_timer_start_with_callback(COUNTDOWN_SW_TIMER, 1000, (timer_func_t)light_ctrl_count_down_timer_callback);
    }
    light_data_count_down_response(CountDownSec);       /* upload countdown value */

    return OPRT_OK;
}

/**
 * @berief: Light reset cnt clear timercallback
 * @param {none}
 * @return: none
 * @retval: none
 */
void light_ctrl_reset_cnt_clear_timer_callback(void)
{
    OPERATE_RET opRet = -1;
    UCHAR_T cnt = 0; 
    app_print("reset_cnt_clear cnt set 0");
    opRet = op_user_flash_write_reset_cnt(&cnt);     /* Reset cnt ++ &save to flash */
    if (opRet != OPRT_OK) {
        PR_ERR("Reset cnt clear error!");
        return ;
    }
    app_print("cnt clear now!");
    light_sys_reset_cnt_clear_callback();
}

/**
 * @berief: get connect mode cfg
 * @param {none}
 * @retval: UCHAR_T
 */
UCHAR_T uc_light_ctrl_get_connect_mode(void)
{
    return (light_cfg_data.connect_mode);
}
/**
 * @berief: light ctrl normal status(constantly bright) display proc
 * @param {none}
 * @retval: none
 */
static void light_ctrl_normal_display(void)
{
    light_data_t ctrl_data;

    memset(&ctrl_data, 0, SIZEOF(light_data_t));

    op_light_set_rgb(500, 500, 500);
}

/**
 * @description: Move Hue with rate handler,Hue move will not stop until stop move reaceived.
 * @param {move_mode} move up / move down
 * @param {Rate} move uint per second,if rate = 0,this api has no effect
 * @return: OPERATE_RET: OPRT_OK / OPRT_INVALID_PARM
 */
OPERATE_RET op_light_ctrl_data_hue_move(MOVE_MODE_T move_mode, USHORT_T Rate)
{
    if (Rate == 0) {
        PR_ERR("Hue move rate can't be 0 ");
        return OPRT_INVALID_PARM;
    }

    g_step_gain = 1;

    Hue_loop = TRUE;
    hue_move_mode = move_mode;
    light_ctrl_data.Mode = COLOR_MODE;
    light_ctrl_data.color_mode = COLOR_MODE_HSV;
    light_ctrl_handle_data.shade_param = SHADE_PARAM_H_LOOP;

    return OPRT_OK;
}

/**
 * @berief: Light ctrl data save
 * @param {none}
 * @attention: this function directly save ctrl data.
 * @retval: none
 */
OPERATE_RET op_light_ctrl_data_auto_save(void)
{
    OPERATE_RET opRet = -1;
    light_app_data_flash_t *save_data;

    save_data = (light_app_data_flash_t *)malloc(SIZEOF(light_app_data_flash_t));
    if (NULL == save_data) {
        PR_ERR("malloc failed.");
        return OPRT_COM_ERROR;
    }
    save_data->bPower = light_ctrl_data.onoff_status;
    save_data->Mode = light_ctrl_data.Mode;
    save_data->Color.Red = light_ctrl_data.Color.Red;
    save_data->Color.Green = light_ctrl_data.Color.Green;
    save_data->Color.Blue = light_ctrl_data.Color.Blue;
    save_data->color_hsv.Hue = light_ctrl_data.color_hsv.Hue;
    save_data->color_hsv.Saturation = light_ctrl_data.color_hsv.Saturation;
    save_data->color_hsv.Value  = light_ctrl_data.color_hsv.Value;

    opRet = op_user_flash_write_app_data(save_data);
    if (opRet != OPRT_OK) {
        PR_ERR("Write app data ERROR!");
        opRet = OPRT_COM_ERROR;
    }

    free(save_data);
    return opRet;
}

/**
 * @description: network joined notify stop ,turn on light
 * @param {type} none
 * @return: none
 */
void op_light_ctrl_nwk_joined_no_notify_stop(void)
{
    light_tool_HSV_2_RGB(light_ctrl_data.color_hsv.Hue, light_ctrl_data.color_hsv.Saturation, light_ctrl_data.color_hsv.Value,\
                        &(light_ctrl_handle_data.target_val.Red),\
                        &(light_ctrl_handle_data.target_val.Green),\
                        &(light_ctrl_handle_data.target_val.Blue));

    memcpy(&light_ctrl_handle_data.current_hsv, &light_ctrl_handle_data.target_hsv,sizeof(light_ctrl_handle_data.target_hsv));

    op_light_set_rgb(light_ctrl_handle_data.target_val.Red,\
                    light_ctrl_handle_data.target_val.Green,\
                    light_ctrl_handle_data.target_val.Blue);
}


/*
* @berief: network joined,light turn off 200ms,then turn on
* @param {none}
* @attention:
* @retval: none
*/
OPERATE_RET op_light_ctrl_nwk_join_ok_notify(void)
{
    OPERATE_RET opRet = -1;

    opRet = op_light_set_rgb(0,0,0);
    if (opRet != OPRT_OK) {
        PR_ERR("breath set RGBCW error!");
    }
    dev_timer_start_with_callback(BLINK_SW_TIMER, NETWORK_JOIN_IDENTIFY_MS, (timer_func_t)op_light_ctrl_nwk_joined_no_notify_stop);
    return OPRT_OK;
}

/**
 * @description: identify accomplish
 * @param {type} none
 * @return: none
 */
static void light_ctrl_identify_process(void)
{
    static UCHAR_T identify_phase = 0; 
    static USHORT_T level_high = 0;    
    STATIC USHORT_T level_low = 0;     
    static BOOL_T shade_started = FALSE;
    UCHAR_T go_next_flag = 0;
    identify_processing_flag = TRUE;

    dev_timer_start_with_callback(BLINK_SW_TIMER, 20, (timer_func_t)light_ctrl_identify_process);

    if (identify_first_time_flag == TRUE) {
        
        level_high = 80 * 10;
        level_low = 80;

        shade_started = FALSE;
        identify_first_time_flag = FALSE;
        light_ctrl_shade_step_gain_set(10);
        light_ctrl_handle_data.slope = 2;
        
        light_ctrl_handle_data.shade_param = SHADE_PARAM_V;
    }
    app_print("identify_phase = %d ",identify_phase);
    switch (identify_phase)
    {
        case 0:{ 
            app_print("identify phase 1");
            if (light_ctrl_data.Mode == COLOR_MODE) {
                light_ctrl_handle_data.target_hsv.Value = level_high;
                if (light_ctrl_handle_data.target_hsv.Value == light_ctrl_handle_data.current_hsv.Value) {
                    go_next_flag = 1;
                }
            }

            if (go_next_flag == 1) {
                identify_phase = 1;
                shade_started = FALSE;
            } else {
                if (shade_started == FALSE) {
                    shade_started = TRUE;
                    light_control_start();
                }
            }

            break;
        }

        case 1:{
            app_print("identify phase 2");
            if (light_ctrl_data.Mode == COLOR_MODE) {
                if (shade_started == FALSE) {
                    light_ctrl_handle_data.current_hsv.Value = level_high;
                }

                light_ctrl_handle_data.target_hsv.Value = level_low;
                if (light_ctrl_handle_data.target_hsv.Value == light_ctrl_handle_data.current_hsv.Value) {
                    go_next_flag = 1;
                }
            } 
            if (go_next_flag == 1) {
                identify_phase = 2;
                shade_started = FALSE;
            } else {
                if (shade_started == FALSE) {
                    shade_started = TRUE;
                    light_control_start();
                }
            }
            break;
        }

        case 2:{
            app_print("identify phase 3");

            if (light_ctrl_data.Mode == COLOR_MODE) {

                if (shade_started == FALSE) {
                    light_ctrl_handle_data.current_hsv.Value = level_low;
                }

                if (light_ctrl_data.onoff_status == 0) {
                    light_ctrl_handle_data.target_hsv.Value = 0;

                } else {
                    light_ctrl_handle_data.target_hsv.Value = light_ctrl_data.color_hsv.Value;
                }

                if (light_ctrl_handle_data.target_hsv.Value == light_ctrl_handle_data.current_hsv.Value) {
                    go_next_flag = 1;
                }
            }

            if (go_next_flag == 1) {
                identify_phase = 0;
                shade_started = FALSE;
                identify_times --;
                if (identify_times == 0) { 
                    identify_processing_flag = FALSE;
                    dev_timer_stop(BLINK_SW_TIMER);
                    light_ctrl_shade_step_gain_set(SHADE_STEP_GAIN_DEFAULT);
                    light_ctrl_handle_data.slope = 1;
                }
            } else {
                if (shade_started == FALSE) {
                    shade_started = TRUE;
                    light_control_start();
                }
            }
            break;
        }
        default:
            break;
    }
}

/**
 * @description: identify cmd handler
 * @param {identifyTimes} identify times
 * @return: none
 */
void light_ctrl_identify_handler(UCHAR_T identifyTimes)
{
    if (identify_processing_flag == TRUE) {
        app_print("identify_processing, return");
        return;
    } else {
        identify_times = identifyTimes;
        identify_first_time_flag = TRUE;
        app_print("vLightCtrlIdentifyProcess");
        dev_timer_start_with_callback(BLINK_SW_TIMER, 1000, (timer_func_t)light_ctrl_identify_process);
    }
}

/**
 * @berief: light ctrl blink proc
 * @param {none}
 * @return: UCHAR_T
 * @retval: UCHAR_T
 */
static void light_ctrl_blink_display(void)
{
    static int Cnt = 0;       /* first blink off */
    light_data_t blink_data;

    if( Cnt % 3 != 0) {
        blink_data.Red = 500;
        blink_data.Green = 500;
        blink_data.Blue = 500;
        op_light_set_rgb(blink_data.Red, blink_data.Green, blink_data.Blue);
    } else {
        blink_data.Red = 0;
        blink_data.Green = 0;
        blink_data.Blue = 0;
        op_light_set_rgb(blink_data.Red, blink_data.Green, blink_data.Blue);
    }
    op_light_ctrl_proc();
    app_print("\r\n blink...R->%d, G->%d, B->%d \r\n", blink_data.Red, blink_data.Green, blink_data.Blue);

    Cnt++;
}

/**
 * @berief: light ctrl blink timer callback
 * @param {none}
 * @return: none
 * @retval: none
 */
static void light_ctrl_blink_timer_callback(void)
{
    light_ctrl_blink_display();
    dev_timer_start_with_callback(BLINK_SW_TIMER, blink_time_ms, (timer_func_t)light_ctrl_blink_timer_callback);
}

/**
 * @description: start blink
 * @param {IN UINT_T BlinkTimeMs -> blink phase}
 * @attention: blink display will as the parm
 *             -- net_color, net_bright in configuration.
 * @retval: none
 */
OPERATE_RET op_light_ctrl_blink_start(UINT_T blink_time)
{
    blink_time_ms = blink_time;
    dev_timer_start_with_callback(BLINK_SW_TIMER, blink_time, (timer_func_t)light_ctrl_blink_timer_callback);

    return OPRT_OK;
}

/**
 * @description: stop blink
 * @param {type} none
 * @attention: blink stop will directly go to normal status display
 *              normal status will bright as default bright parm
 *              -- def_bright,def_temper,def_color in configuration.
 * @retval: none
 */
OPERATE_RET op_light_ctrl_blink_stop(void)
{
    dev_timer_stop(BLINK_SW_TIMER);
    
    light_ctrl_normal_display();
    return OPRT_OK;
}


  
/**
 * @description: stop Hue,Saturation, XY moving
 * @param {type} none
 * @return: none
 */
OPERATE_RET op_light_ctrl_data_move_stop(void)
{
    dev_timer_stop(SHADE_TIMER);
    
    level_down_to_off = FALSE;

    light_ctrl_shade_gain_set(SHADE_STEP_GAIN_DEFAULT);

    memcpy(&light_ctrl_handle_data.target_hsv, &light_ctrl_handle_data.current_hsv,sizeof(COLOR_HSV_T));

    app_print(" \r\n in op_light_ctrl_data_move_stop ");
    
    light_ctrl_data.Color.Red = light_ctrl_handle_data.current_val.Red;
    light_ctrl_data.Color.Green = light_ctrl_handle_data.current_val.Green;
    light_ctrl_data.Color.Blue = light_ctrl_handle_data.current_val.Blue;

    memcpy(&light_ctrl_data.color_hsv, &light_ctrl_handle_data.current_hsv, sizeof(COLOR_HSV_T));

    light_data_move_complete_cb();

    return OPRT_OK;
}

/**
* @berief: set light realtime control data
* @param {IN BOOL_T bMusicFlag -> music control data}
* @param {IN UCHAR_T *RealTimeData}
* @attention: data format: please reference to DP protocol
* @retval: OPERATE_RET -> OPRT_OK need to call opLightRealTimeCtrlProc function.
*/
OPERATE_RET op_light_ctrl_data_realtime_adjust_set(BOOL_T bMusicFlag, UCHAR_T *RealTimeData)
{

   if (strcmp((const char *)RealTimeData, (const char *)light_ctrl_data.real_time_data) == 0) {
       app_print("the same realtime adjust data");
       return OPRT_INVALID_PARM;
   }

   if (strlen((const char *)RealTimeData) != 21) {
       PR_ERR("Real time adjust data is error! please chek!");
       return OPRT_INVALID_PARM;
   }

   strcpy((CHAR_T*)&light_ctrl_data.real_time_data, (const char *)RealTimeData);
   app_print("light real time adjust ctrl data buf %s",light_ctrl_data.real_time_data);

   return OPRT_OK;
}

/**
* @berief: Light realtime ctrl process
* @param {none}
* @retval: OPERATE_RET
*/
OPERATE_RET op_light_realtime_ctrl_proc(void)
{   
    UCHAR_T change_mode = 0;
    USHORT_T usHub, usSat, usVal;
    LIGHT_CTRL_DATA_T ctrl_data_temp;
    COLOR_HSV_T color_hsv;

    usHub = light_tool_STR_2_USHORT( light_tool_ASC_2_HEX(light_ctrl_data.real_time_data[1]), \
                                            light_tool_ASC_2_HEX(light_ctrl_data.real_time_data[2]),\
                                            light_tool_ASC_2_HEX(light_ctrl_data.real_time_data[3]), \
                                            light_tool_ASC_2_HEX(light_ctrl_data.real_time_data[4]) );
    app_print("the usHue :%d\r\n",usHub);

    usSat = light_tool_STR_2_USHORT( light_tool_ASC_2_HEX(light_ctrl_data.real_time_data[5]), \
                                        light_tool_ASC_2_HEX(light_ctrl_data.real_time_data[6]),\
                                        light_tool_ASC_2_HEX(light_ctrl_data.real_time_data[7]), \
                                        light_tool_ASC_2_HEX(light_ctrl_data.real_time_data[8]) );
    app_print("the usSat :%d\r\n",usSat);

    usVal = light_tool_STR_2_USHORT( light_tool_ASC_2_HEX(light_ctrl_data.real_time_data[9]), \
                                        light_tool_ASC_2_HEX(light_ctrl_data.real_time_data[10]),\
                                        light_tool_ASC_2_HEX(light_ctrl_data.real_time_data[11]), \
                                        light_tool_ASC_2_HEX(light_ctrl_data.real_time_data[12]) );
    app_print("the usVal :%d\r\n",usVal);

    if(usVal < CTRL_RGB_BRIGHT_VALUE_MIN) {
        usVal = 0;
    } else if(usVal >= CTRL_RGB_BRIGHT_VALUE_MAX){
        usVal = CTRL_RGB_BRIGHT_VALUE_MAX;
    }
    light_ctrl_data.ucRealTimeFlag = TRUE;
    change_mode = light_ctrl_data.real_time_data[0] - '0';
    app_print("Real time data H S V M: %d %d %d ,%d", usHub, usSat, usVal, change_mode);

    light_control_stop();

    memset(&ctrl_data_temp, 0, SIZEOF(LIGHT_CTRL_DATA_T));

    ctrl_data_temp.Color.Red = light_ctrl_data.Color.Red;
    ctrl_data_temp.Color.Green = light_ctrl_data.Color.Green;
    ctrl_data_temp.Color.Blue = light_ctrl_data.Color.Blue;

    if(usVal > 0) {
        COLOR_RGB_T color_temp;
        light_tool_HSV_2_RGB(  usHub,\
                            usSat,\
                            usVal,\
                            &color_temp.Red,\
                            &color_temp.Green,\
                            &color_temp.Blue);
        if(color_temp.Red == 0 && color_temp.Green == 0 && color_temp.Blue == 0){
            app_print("Color mode set RGB = 0");
        }

        memset(&light_ctrl_handle_data.target_val, 0, sizeof(light_ctrl_handle_data.target_val));
        light_ctrl_handle_data.target_val.Red = color_temp.Red;
        light_ctrl_handle_data.target_val.Green = color_temp.Green;
        light_ctrl_handle_data.target_val.Blue = color_temp.Blue;

        color_hsv.Hue = usHub;
        color_hsv.Saturation = usSat;
        color_hsv.Value = usVal;

        memcpy(&light_ctrl_handle_data.target_hsv, &color_hsv, sizeof(COLOR_HSV_T));
        memcpy(&light_ctrl_handle_data.current_hsv, &color_hsv, sizeof(COLOR_HSV_T));
    }

    light_ctrl_handle_data.shade_param = SHADE_PARAM_RGBCW;
    light_ctrl_handle_data.slope = SHADE_SLOPE_DEFAULT;
    USHORT_T diff_max;

    diff_max = (USHORT_T)light_tool_get_max_value(\
                    light_tool_get_abs_value(light_ctrl_handle_data.target_val.Red - light_ctrl_handle_data.current_val.Red),\
                    light_tool_get_abs_value(light_ctrl_handle_data.target_val.Red - light_ctrl_handle_data.current_val.Red),\
                    light_tool_get_abs_value(light_ctrl_handle_data.target_val.Red - light_ctrl_handle_data.current_val.Red));

    g_step_gain = diff_max * LIGHT_SHADE_CYCLE / 300;
    if(g_step_gain < 1) {
        g_step_gain = 1;
    }

    app_print("current Real time R G B:%d %d %d",light_ctrl_handle_data.current_val.Red,\
                                        light_ctrl_handle_data.current_val.Green,\
                                        light_ctrl_handle_data.current_val.Blue);

    app_print("target Real time R G B:%d %d %d",light_ctrl_handle_data.target_val.Red,\
                                        light_ctrl_handle_data.target_val.Green,\
                                        light_ctrl_handle_data.target_val.Blue);
    light_control_start();
    
     return OPRT_OK;
}

