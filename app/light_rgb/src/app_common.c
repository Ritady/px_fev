/*
 * @Author: xiaojia
 * @email: limu.xiao@tuya.com
 * @LastEditors:
 * @file name: app_common.h
 * @Description: common function source file
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2021.8.30 17:28:05
 * @LastEditTime: 2021.8.30 17:28:01
 */

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "tuya_tools.h"
#include "dev_register.h"
#include "light_control.h"
#include "app_common.h"
#include "onoff_cluster.h"


#define MG21_PIN_TABLE_LEN   get_array_len(MODULE_MG21)
#define PIN_NUM_ERROR       0xFF

/********************************
    USHORT_T usFreq;            ///< PWM Frequency
    USHORT_T usDuty;            ///< PWM Init duty
    UCHAR_T ucList[5];          ///< GPIO List
    UCHAR_T ucChannel_num;      ///< GPIO List length
    BOOL_T bPolarity;
**********************************/
user_pwm_init_t vLight_pwm_init =
{
   1000,
   500,
   {R_NUM,G_NUM,B_NUM},
   3,
   TRUE
};

num2pin_t MODULE_MG21[]=
{
    {1,PORT_A,PIN_0},
    {2,PORT_A,PIN_1},
    {3,PORT_A,PIN_2},
    {4,PORT_A,PIN_3},
    {5,PORT_A,PIN_4},
    {6,PORT_A,PIN_5},
    {7,PORT_A,PIN_6},
    {8,PORT_B,PIN_0},
    {9,PORT_B,PIN_1},
    {10,PORT_C,PIN_0},
    {11,PORT_C,PIN_1},
    {12,PORT_C,PIN_2},
    {13,PORT_C,PIN_3},
    {14,PORT_C,PIN_4},
    {15,PORT_C,PIN_5},
    {16,PORT_D,PIN_0},
    {17,PORT_D,PIN_1},
    {18,PORT_D,PIN_2},
    {19,PORT_D,PIN_3},
    {20,PORT_D,PIN_4},
};

UCHAR_T scene_sync_ready_flag = 0; //Is it time to send a synchronization command
UCHAR_T is_master_device = FALSE; //it is the master device
USHORT_T g_join_network_time = 600;//10 min
UCHAR_T  g_nwk_type = ZG_JOIN_TYPE_NO_NETWORK;
UCHAR_T mf_test_flag = 0;
NETWORK_STATE_E g_network_state = NETWORK_STATE_OFFLINE;
UCHAR_T  cmd_control_shade_flag = 1;

UCHAR_T g_light_channel = 0;

bool_t user_pwm_init_flag = FALSE;
bool_t g_pwm_bPolarity = FALSE;

UCHAR_T WRITE_DATA[FLASH_DATA_SIZE_MAX] = {0};
UCHAR_T READ_DATA[FLASH_DATA_SIZE_MAX] = {0};

/**
 * @description: write app data attribute value,app data = onoff(1) + mode(1) + H(2) + S(2) + V(2) + B(2) + T(2)= 12 Bytes
 * @param {OUT app_data_t* app_data} data value to be write
 * @return: success:1 / faild:0
 */
bool_t write_app_data_attr(app_data_t *app_data)
{
    UCHAR_T buffer[sizeof(app_data_t) + 1] = {0};
    if (app_data == NULL) {
        PR_ERR("app_data ERROR");
        return FALSE;
    }
    buffer[0] = sizeof(app_data_t);
    memcpy(&buffer[1], app_data, sizeof(app_data_t));
    ATTR_CMD_RET_T ret = dev_zigbee_write_attribute(1,
                                                 CLUSTER_COLOR_CONTROL_CLUSTER_ID, 
                                                 TY_ATTR_LIGHT_APP_DATA_ATTRIBUTE_ID, 
                                                 buffer, 
                                                 ATTR_ARRAY_ATTRIBUTE_TYPE);
    if (ret != ATTR_CMD_RET_SUCCESS) {
        return FALSE;
    }
    return TRUE;
}

/**
 * @description: read app data attribute value,app data = onoff(1) + mode(1) + H(2) + S(2) + V(2) + B(2) + T(2)= 12 Bytes
 * @param {OUT app_data_t* app_data} readout data value
 * @return: success:1 / faild:0
 */
bool_t read_app_data_attr(app_data_t* app_data)
{
    UINT_T buffer[sizeof(app_data_t)+1] = {0};
    memset(buffer, 0, sizeof(buffer));

    ATTR_CMD_RET_T ret = dev_zigbee_read_attribute(1,\
                                                  CLUSTER_COLOR_CONTROL_CLUSTER_ID,\
                                                  TY_ATTR_LIGHT_APP_DATA_ATTRIBUTE_ID,\
                                                  buffer, \
                                                  sizeof(buffer));
    if (ret != ATTR_CMD_RET_SUCCESS) {
        PR_ERR("Read app_data ERROR");
        return FALSE;
    }
    memcpy(app_data, &buffer[1], sizeof(app_data_t));
    return TRUE;
}

/**
 * @description: app command received and process now
 * @param {type} none
 * @return: none
 */
void app_cmd_handle_delay_timer_cb(void)
{
    op_light_ctrl_proc();
}

/**
 * @description: load app data from Flash,and init the attributes
 * @param {type} none
 * @return: none
 */
void dev_load_attr_data(void)
{
    UINT_T level;

    LIGHT_CTRL_DATA_T* app_data = op_light_ctrl_data_get();
//onoff
    dev_zigbee_write_attribute(1,
                                CLUSTER_ON_OFF_CLUSTER_ID,
                                ATTR_ON_OFF_ATTRIBUTE_ID,
                                &app_data->onoff_status,
                                ATTR_BOOLEAN_ATTRIBUTE_TYPE);
//tuya mode
    dev_zigbee_write_attribute(1,
                                CLUSTER_COLOR_CONTROL_CLUSTER_ID,
                                TY_ATTR_LIGHT_MODE_ATTRIBUTE_ID,
                                &app_data->Mode,
                                ATTR_INT8U_ATTRIBUTE_TYPE);
//Tuya HSV
    dev_zigbee_write_attribute(1,
                                CLUSTER_COLOR_CONTROL_CLUSTER_ID,
                                TY_ATTR_COLOR_HSV_ATTRIBUTE_ID,
                                &app_data->color_hsv,
                                ATTR_INT48U_ATTRIBUTE_TYPE);

//standard level                              //when the light mode is color mode, it change HSV -> V's value
    level = app_B_2_zig_b(app_data->color_hsv.Value);

    if (level < 4) {
        level = 1;
    }
    dev_zigbee_write_attribute(1,\
                                CLUSTER_LEVEL_CONTROL_CLUSTER_ID,\
                                ATTR_CURRENT_LEVEL_ATTRIBUTE_ID,\
                                &level,\
                                ATTR_INT8U_ATTRIBUTE_TYPE);
//standard H(Zigbee 3.0)
    level = app_H_2_zig_h(app_data->color_hsv.Hue);
    dev_zigbee_write_attribute(1,\
                                CLUSTER_COLOR_CONTROL_CLUSTER_ID,\
                                ATTR_COLOR_CONTROL_CURRENT_HUE_ATTRIBUTE_ID,\
                                &level,\
                                ATTR_INT8U_ATTRIBUTE_TYPE);

//standard S(Zigbee 3.0)
    level  =  app_S_2_zig_s(app_data->color_hsv.Saturation);
    dev_zigbee_write_attribute(1,\
                                CLUSTER_COLOR_CONTROL_CLUSTER_ID,\
                                ATTR_COLOR_CONTROL_CURRENT_SATURATION_ATTRIBUTE_ID,\
                                &level,\
                                ATTR_INT8U_ATTRIBUTE_TYPE);

}

void dev_read_attr_data(void)
{
    UINT_T ret = 0;
    UINT_T attr_read[67] = {0}; 

    // read onoff
    ret = dev_zigbee_read_attribute(1,
                                    CLUSTER_ON_OFF_CLUSTER_ID,
                                    ATTR_ON_OFF_ATTRIBUTE_ID,
                                    attr_read,
                                    1);
    if(ret == ATTR_CMD_RET_SUCCESS){
        app_print("attr on/off : %d",attr_read[0]);
    }
    else{
        app_print("Read ERROR");
    }

    // read light mode
    ret = dev_zigbee_read_attribute(1,
                                    CLUSTER_COLOR_CONTROL_CLUSTER_ID,
                                    TY_ATTR_LIGHT_MODE_ATTRIBUTE_ID,
                                    attr_read,
                                    1);
    if(ret == ATTR_CMD_RET_SUCCESS){
        app_print("attr mode : %d",attr_read[0]);
    }else{
        app_print("Read ERROR");
    }

    // read light hsv -> v
    ret = dev_zigbee_read_attribute(1,
                                    CLUSTER_COLOR_CONTROL_CLUSTER_ID,
                                    TY_ATTR_LIGHT_V_VALUE_ATTRIBUTE_ID,
                                    attr_read,
                                    1);
    if(ret == ATTR_CMD_RET_SUCCESS){
        app_print("attr V : %d",attr_read[0]);
    }else{
        app_print("Read ERROR");
    }

    // read light hsv
    ret = dev_zigbee_read_attribute(1,
                                    CLUSTER_COLOR_CONTROL_CLUSTER_ID,
                                    TY_ATTR_COLOR_HSV_ATTRIBUTE_ID,
                                    &attr_read,
                                    6);
    if(ret == ATTR_CMD_RET_SUCCESS){
        app_print("attr H: %x , S: %x , V: %x ",(attr_read[0] + attr_read[1] << 8),\
                                            (attr_read[2] + attr_read[3] << 8),\
                                            (attr_read[4] + attr_read[5] << 8));
    }else{
        app_print("Read ERROR");
    }
}

/**
 * @description: get the final option,single control command is effective when the light is off,
 * need to combine the option attribute value and the current command's option_mask+optionOverride.
 * .Currently, only case of optionMask=0000 0001, Update the function when the alliance standard has expanded
 * @param {cluter} cluster id
 * @param {optionMask} Used to indicate which bits of the option attribute need to be rewritten
 * @param {optionOverride} to show the Bit of optionMask,which the rewritten value
 * @return: the final option value
 */
UCHAR_T get_option_value(CLUSTER_ID_T cluter, UCHAR_T option_mask, UCHAR_T option_override)
{
    UCHAR_T option = 0;
    UCHAR_T state = 0;

    state = dev_zigbee_read_attribute(1,cluter, OPTION_ATTRIBUTE_ID,&option, sizeof(option));
    if (state != ATTR_CMD_RET_SUCCESS) {
       option = 0;
    }

    if (option_mask == 0x01) {
        if (option_override == 0x01) {
            option = 0x01;
        } else {
            option = 0x00;
        }
    }
    if (option != 0) {
        option = 1;
    }
    return option;
}

/**
 * @berief: read light light ctrl data
 * @param {OUT UCHAR_T *data -> reset cnt}
 * @return: OPERATE_RET
 * @retval: none
 */
OPERATE_RET op_user_flash_read_light_ctrl_data(light_app_data_flash_t *data)
{
    USHORT_T ret = 0;
    UINT_T addr = LIGHT_APP_DATA_OFFSET;
    USHORT_T len = sizeof(light_app_data_flash_t);

    if ((addr + len ) >= FLASH_DATA_MAX_ADDRESS||addr < FLASH_DATA_START_ADDRESS) {
        app_print("read data size out range !");
        return 1;
    }
    memset(READ_DATA, 0, sizeof(READ_DATA));
    ret = user_flash_data_read(READ_DATA, sizeof(READ_DATA));
    if (!ret)
    {   
        return 0;
    }
    if (READ_DATA[addr] == 0x5A)
    {
        memcpy(data, &READ_DATA[addr + 1], len);
    } else {
        app_print("the Frame header was wrong");
        return 0;
    }
    return OPRT_OK;
}

/**
 * @berief: read light reset cnt
 * @param {OUT UCHAR_T *data -> reset cnt}
 * @return: OPERATE_RET
 * @retval: none
 */
OPERATE_RET op_user_flash_read_reset_cnt(UCHAR_T *data)
{
    USHORT_T ret = 0;
    UINT_T addr = 0;
    USHORT_T len = 1;

    if ((addr + len ) >= FLASH_DATA_MAX_ADDRESS||addr < FLASH_DATA_START_ADDRESS) {
        app_print("read data size out range !");
        return 1;
    }
    memset(READ_DATA, 0, sizeof(READ_DATA));
    ret = user_flash_data_read(READ_DATA, sizeof(READ_DATA));
    if (!ret)
    {   
        return 0;
    }
    if (READ_DATA[addr] == 0x5A)
    {
        memcpy(data, &READ_DATA[addr + 1], len);
    } else {
        app_print("the Frame header was wrong");
        return 0;
    }

    return OPRT_OK;
}

/**
 * @berief: save light reset cnt
 * @param {IN UCHAR_T data -> save reset cnt to flash}
 * @return: OPERATE_RET
 * @retval: none
 */
OPERATE_RET op_user_flash_write_reset_cnt(UCHAR_T *data)
{
    UCHAR_T ret = 0;
    USHORT_T i = 0;
    UINT_T addr = 0;
    USHORT_T len = 1 ;

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
        WRITE_DATA[addr+i+1] = *(data+i);
    }
    ret = user_flash_data_write(WRITE_DATA, sizeof(WRITE_DATA));
    if (ret != sizeof(WRITE_DATA)) {
        app_print("write data error !");
        return OPRT_COM_ERROR;
    }
    return OPRT_OK;
}

/**
 * @description: Turning on lights caused by non-switching commands
 * @param {IN bool_t onoff} onoff value
 * @param {IN SEND_QOS_T Qos} QOS value
 * @param {IN USHORT_T delay_ms} delay report
 * @return: none
 */
OPERATE_RET op_set_onoff_with_countdown_cmd(bool_t onoff, SEND_QOS_T Qos, UINT_T delay_ms)
{
    bool_t pre_onoff;

    op_light_ctrl_data_switch_get(&pre_onoff);
    op_light_ctrl_data_switch_set(onoff);
    if (pre_onoff != onoff) {
        report_onoff_value(Qos, delay_ms, 1);
        report_count_down_data(Qos, delay_ms, 0);
        return OPRT_OK;
    } else {
        return OPRT_INVALID_PARM;
    }
}

/**
 * @description: join start delay timer hander, join now
 * @param {none}
 * @return: none
 */
void network_join_start_delay_timer_callback(void)
{
    dev_zigbee_join_start(ZIGBEE_JOIN_MAX_TIMEOUT); 
}

GPIO_PORT_T gpio_get_port(UCHAR_T port_num)
{       
    UCHAR_T i;
    for(i = 0; i < MG21_PIN_TABLE_LEN; i++) {
        if (port_num == MODULE_MG21[i].num) {
            return MODULE_MG21[i].port;
        }
    }
    if (i == MG21_PIN_TABLE_LEN) {  //can't find port in the table
        return (GPIO_PORT_T)PIN_NUM_ERROR;
    }
    return (GPIO_PORT_T)PIN_NUM_ERROR;
}

GPIO_PIN_T gpio_get_pin(UCHAR_T pin_num)
{
    UCHAR_T i;
    for(i = 0; i < MG21_PIN_TABLE_LEN; i++) {
        if (pin_num == MODULE_MG21[i].num) {
            return MODULE_MG21[i].pin;
        }
    }
    if (i == MG21_PIN_TABLE_LEN) { //can't find port in the table
        return (GPIO_PIN_T)PIN_NUM_ERROR;
    }
    return (GPIO_PIN_T)PIN_NUM_ERROR;
}

/**
 * @berief: pwm init
 * @param {user_pwm_init_t *light_config -> gpio set config}
 * @return: OPERATE_RET
 * @retval: OPERATE_RET
 */
OPERATE_RET light_pwm_init(user_pwm_init_t *light_config)
{       
    OPERATE_RET ret = OPRT_OK;
    pwm_gpio_t pwm_gpio_list[3];
    UCHAR_T i = 0;
    UCHAR_T list[3] = {0};


    if (light_config == NULL) {
        app_print("USER PWM init is invalid!");
        return OPRT_INVALID_PARM;
    }

    if((light_config->usFreq < PWM_MIN_FREQ) || (light_config->usFreq > PWM_MAX_FREQ)) {
        app_print("USER PWM init is invalid!");
        return OPRT_INVALID_PARM;
    }

    if(light_config->usDuty > PWM_MAX_DUTY) {
        app_print("USER PWM init is invalid!");
        return OPRT_INVALID_PARM;
    }

    if((light_config->ucChannel_num <= 0) || (light_config->ucChannel_num > 5)){
        app_print("USER PWM init is invalid!");
        return OPRT_INVALID_PARM;
    }

    memset(pwm_gpio_list, 0, sizeof(pwm_gpio_t) * 3);

    list[0] = light_config->ucList[0];
    list[1] = light_config->ucList[1];
    list[2] = light_config->ucList[2];

    for(i = 0; i < (light_config->ucChannel_num); i++) {
        pwm_gpio_list[i].port = gpio_get_port(list[i]);
        if (pwm_gpio_list[i].port == PIN_NUM_ERROR) {
            return OPRT_INVALID_PARM;
        }
        pwm_gpio_list[i].pin = gpio_get_pin(list[i]);
        if (pwm_gpio_list[i].pin == PIN_NUM_ERROR) {
            return OPRT_INVALID_PARM;
        }
        pwm_gpio_list[i].lv = light_config->usDuty? GPIO_LEVEL_HIGH:GPIO_LEVEL_LOW;
    }
    
    if (!hal_pwm_init(pwm_gpio_list, light_config->ucChannel_num, light_config->usFreq)) {
        app_print("HAL PWM init ERR");
        return OPRT_INVALID_PARM; 
    } else {
        app_print("PWM init ok!");
    }

    user_pwm_init_flag = TRUE;
    g_pwm_bPolarity = light_config->bPolarity;   ///< note all pwm channel use same polarity
    g_light_channel = light_config->ucChannel_num;
    return ret;
}

void power_on_reset_data(void)
{
    ZG_JOIN_TYPE_T nwk_type = zg_get_join_type();
    op_light_ctrl_data_switch_set(TRUE);        //power on turn on the light 
    op_light_ctrl_data_mode_set(COLOR_MODE);  //set the color mode

    if (nwk_type == ZG_JOIN_TYPE_NO_NETWORK) {    //if the device power on and havn't network will set a default color
        op_light_set_rgb(80, 210, 40);
        app_print(" device power on and no network ");
    } else {
        op_light_set_rgb(10, 80, 280);
        app_print(" device power on and online ");
    }
}

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
void dev_hw_reboot_proc(void)
{
    OPERATE_RET opRet = -1;
    UCHAR_T Cnt = 0;

    opRet = op_user_flash_read_reset_cnt(&Cnt);     /* read cnt from flash */
    if (opRet != OPRT_OK) {
        app_print("Read reset cnt error!");
        Cnt = 0;
    }
    PR_ERR("read reset cnt %d",Cnt);

    Cnt++;
    opRet = op_user_flash_write_reset_cnt(&Cnt);     /* Reset cnt ++ &save to flash */
    if (opRet != OPRT_OK) {
        PR_ERR("Reset cnt add write error!");
        return;
    }

    PR_ERR("start reset cnt clear timer!!!!!");

    /* start clear reset cnt timer */
    dev_timer_start_with_callback(CLEAR_RESET_CNT_SW_TIMER, 5000, (timer_func_t)light_ctrl_reset_cnt_clear_timer_callback);
}


/**
 * @berief: Light reset to re-distribute proc
 * @param {none}
 * @attention: this func will call light_sys_reset_cnt_over_callback()
 *              light_sys_reset_cnt_over_callback() need to implement by system
 * @retval: OPERATE_RET
 */
OPERATE_RET dev_reset_cnt_proc(void)
{
    OPERATE_RET opRet = -1;
    UCHAR_T Cnt = 0;

    opRet = op_user_flash_read_reset_cnt(&Cnt);
    if (opRet != OPRT_OK) {
        PR_ERR("Read reset cnt error!");
        return OPRT_COM_ERROR;
    }

    if (Cnt < 3) {   // low down than thire times 
        PR_ERR("Don't reset ctrl data!");
        return OPRT_OK;
    }

   PR_ERR("Reset ctrl data!");
   dev_timer_start_with_callback(NETWORK_JOIN_START_DELAY_TIMER_ID,\
                             NETWORK_JOIN_START_DELAY_TIME,\
                            (timer_func_t)network_join_start_delay_timer_callback);

    return OPRT_OK;
}

void remember_light_proc(void)
{
    light_app_data_flash_t *remember_data;

    remember_data = (light_app_data_flash_t *)malloc(SIZEOF(light_app_data_flash_t));

    op_user_flash_read_light_ctrl_data(remember_data);

    light_ctrl_handle_data.target_val.Red = remember_data->Color.Red;
    light_ctrl_handle_data.target_val.Green = remember_data->Color.Green;
    light_ctrl_handle_data.target_val.Blue = remember_data->Color.Blue;
    light_ctrl_handle_data.target_hsv.Hue = remember_data->color_hsv.Hue;
    light_ctrl_handle_data.target_hsv.Saturation = remember_data->color_hsv.Saturation;
    light_ctrl_handle_data.target_hsv.Value = remember_data->color_hsv.Value;
    app_print(" \r\n the remember_data R:%d, G:%d, B:%d and H:%d, S:%d, V:%d", light_ctrl_handle_data.target_val.Red,\
                                                                            light_ctrl_handle_data.target_val.Green,\
                                                                            light_ctrl_handle_data.target_val.Blue,\
                                                                            light_ctrl_handle_data.target_hsv.Hue,\
                                                                            light_ctrl_handle_data.target_hsv.Saturation,\
                                                                            light_ctrl_handle_data.target_hsv.Value);

    free(remember_data);
}

/**
 * @berief: light init
 * @param {UINT_T HWTimercycle -> hardware time period,unit:us}
 * @return: OPERATE_RET
 * @retval: OPERATE_RET
 */
OPERATE_RET dev_light_init(void)
{
    light_pwm_init(&vLight_pwm_init);// GPIO_PWM drive init
    app_print("light pwn init!\r\n");

    power_on_reset_data();

    dev_hw_reboot_proc();         //cnt the hardware reboot times

    if (op_light_ctrl_proc() != OPRT_OK) {
        PR_ERR("op_light_ctrl_proc error!");
    }

    app_print("light init ok!\r\n");
    return OPRT_OK;
}
