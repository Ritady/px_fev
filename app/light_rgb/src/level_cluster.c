/*
 * @Author: xiaojia
 * @email: limu.xiao@tuya.com
 * @LastEditors:
 * @file name: level_cluster.c
 * @Description: level cluster attribute report and gateway cmd handler source file
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2021.8.30 16:50:30
 * @LastEditTime: 2021.8.30 20:46:51
 */

#include "stdio.h"
#include "stdlib.h"
#include "light_control.h"
#include "level_cluster.h"
#include "dev_register.h"
#include "app_common.h"
#include "light_tools.h"
#include "color_control_cluster.h"
#include "onoff_cluster.h"


/***********************************************************************************************************
 * 
 *          gateway send cmd to device, device handler the cmd functions
 * 
 **********************************************************************************************************/

/**
 * @note report level  attribute value
 * @param [in]{SEND_QOS_T} QOS value
 * @param [in]{USHORT_T} delay report
 * @param [in]{UCHAR_T} level
 * @return: none
 */
void report_level_value(SEND_QOS_T Qos, USHORT_T delay_ms, UINT_T level)
{
    dev_send_data_t send_data;
    memset(&send_data, 0, sizeof(dev_send_data_t));

    send_data.qos = Qos;
    send_data.delay_time = delay_ms;
    send_data.zcl_id = ZCL_ID_BRIGHT;
    send_data.direction = ZCL_DATA_DIRECTION_SERVER_TO_CLIENT;
    send_data.command_id = CMD_REPORT_ATTRIBUTES_COMMAND_ID;
    send_data.addr.mode = SEND_MODE_GW;
    send_data.addr.type.gw.cluster_id = CLUSTER_LEVEL_CONTROL_CLUSTER_ID;
    send_data.addr.type.gw.src_ep = 1;
    send_data.data.zg.attr_sum = 1;
    send_data.data.zg.attr[0].attr_id = ATTR_CURRENT_LEVEL_ATTRIBUTE_ID;
    send_data.data.zg.attr[0].type = ATTR_INT8U_ATTRIBUTE_TYPE;
    send_data.data.zg.attr[0].value_size = 1;
    send_data.data.zg.attr[0].value[0] = level;

    dev_zigbee_clear_send_data(ZG_CLEAR_ALL_ZCL_ID, &send_data.zcl_id);
    dev_zigbee_send_data(&send_data, NULL, 1000);
    app_print("\r\n  Report level QOS:%d VALUE %d", Qos, level);
}

void report_ty_v_value(SEND_QOS_T Qos, USHORT_T delay_ms, UINT_T v_bright)
{
    dev_send_data_t send_data;
    memset(&send_data, 0, sizeof(dev_send_data_t));
    send_data.qos = Qos;
    send_data.delay_time = delay_ms;
    send_data.zcl_id = ZCL_ID_BRIGHT;
    send_data.direction = ZCL_DATA_DIRECTION_SERVER_TO_CLIENT;
    send_data.command_id = CMD_REPORT_ATTRIBUTES_COMMAND_ID;
    send_data.addr.mode = SEND_MODE_GW;
    send_data.addr.type.gw.cluster_id = CLUSTER_LEVEL_CONTROL_CLUSTER_ID;
    send_data.addr.type.gw.src_ep = 1;
    send_data.data.zg.attr_sum = 1;
    send_data.data.zg.attr[0].attr_id = TY_ATTR_LIGHT_V_VALUE_ATTRIBUTE_ID;
    send_data.data.zg.attr[0].type = ATTR_INT16U_ATTRIBUTE_TYPE;
    send_data.data.zg.attr[0].value_size = 2;
    send_data.data.zg.attr[0].value[0] = (UCHAR_T)(v_bright);
    send_data.data.zg.attr[0].value[1] = (UCHAR_T)(v_bright >> 8);

    dev_zigbee_clear_send_data(ZG_CLEAR_ALL_ZCL_ID, &send_data.zcl_id);
    dev_zigbee_send_data(&send_data, NULL, 1000);
    PR_DEBUG("\r\n  Report level QOS:%d VALUE %d", Qos, v_bright);
}


/**
 * @description: Set a level value
 * @param {IN UINT_T level} level 0 ~ 254
 * @param {IN USHORT_T trans_time} delay_time value 
 * @param {IN bool_t with_onoff} affect the switch or not
 * @return: none
 */
void app_cmd_move_to_level_handler(UINT_T level, USHORT_T trans_time, bool_t with_onoff)
{
    SEND_QOS_T send_qos = QOS_VIP_0;
    uint32_t delay_ms = 0;
    UCHAR_T level_temp = level;
    USHORT_T ty_level = 0;
    UCHAR_T onoff;
    LIGHT_MODE_E mode;
    LIGHT_MODE_E pre_mode;

    op_light_ctrl_data_mode_get(&pre_mode);

    if(level_temp == 0) {
        level_temp = 1;         
        if(with_onoff == TRUE){
            level_down_to_off = TRUE; // if level = 0,with_onoff = 1, shade param and turn off the light
        }
    } else {
        if(with_onoff == TRUE){           //if level != 0, with_onoff = 1, need to turn on the light right now
            bool_t pre_onoff = TRUE;
            bool_t judge_onoff;
            op_light_ctrl_data_switch_get(&judge_onoff);
            op_light_ctrl_data_switch_set(pre_onoff);
            if(pre_onoff != judge_onoff) {
                report_onoff_value(QOS_VIP_0, 0, 0);
            }
        }
    }

    ty_level= zig_b_2_app_B(level_temp);
    if (ty_level < CTRL_CW_BRIGHT_VALUE_MIN) {
        ty_level = CTRL_CW_BRIGHT_VALUE_MIN; //The minimum white light brightness value is 10, otherwise the report will be converted to 0%
    }

    OPERATE_RET op_ret = op_light_ctrl_data_level_set(ty_level, trans_time);
    if (op_ret != OPRT_OK) {
        app_print("Zigbee Level set no effect");
        return;
    }

    op_light_ctrl_data_switch_get(&onoff);

    //updata onoff attribute
    dev_zigbee_write_attribute(1,\
                                CLUSTER_ON_OFF_CLUSTER_ID,\
                                ATTR_ON_OFF_ATTRIBUTE_ID,\
                                &onoff,\
                                ATTR_BOOLEAN_ATTRIBUTE_TYPE);

    report_onoff_value(send_qos, delay_ms, onoff);

    //updata mode attribute
    op_light_ctrl_data_mode_get(&mode);
    if (mode != pre_mode) {
      //updata mode attribute
    dev_zigbee_write_attribute(1,\
                              CLUSTER_COLOR_CONTROL_CLUSTER_ID,\
                              TY_ATTR_LIGHT_MODE_ATTRIBUTE_ID,\
                              &mode,\
                              ATTR_INT8U_ATTRIBUTE_TYPE);
      PR_ERR("set bright change to WHITE MODE!");

    }
    report_mode_value(send_qos, delay_ms, mode);

      //updata Zigbee 3.0 level attribute
    dev_zigbee_write_attribute(1,\
                                CLUSTER_LEVEL_CONTROL_CLUSTER_ID,\
                                ATTR_CURRENT_LEVEL_ATTRIBUTE_ID,\
                                &level_temp,\
                                ATTR_INT8U_ATTRIBUTE_TYPE);

    //updata white light level attribute
    dev_zigbee_write_attribute(1,\
                                CLUSTER_LEVEL_CONTROL_CLUSTER_ID,\
                                TY_ATTR_LIGHT_V_VALUE_ATTRIBUTE_ID,\
                                &ty_level,\
                                ATTR_INT16U_ATTRIBUTE_TYPE);

    report_ty_v_value(send_qos, delay_ms, ty_level);

    //updata HSV bright attribute
    COLOR_HSV_T ty_hsv;
    op_light_ctrl_data_hsv_get(&ty_hsv);
    // ty_hsv.Value = ty_level;
    dev_zigbee_write_attribute(1,\
                                CLUSTER_COLOR_CONTROL_CLUSTER_ID,\
                                TY_ATTR_COLOR_HSV_ATTRIBUTE_ID,\
                                &ty_hsv,\
                                ATTR_INT48U_ATTRIBUTE_TYPE);
    report_hsv_value(send_qos, delay_ms, (app_hsv_t*)(&ty_hsv));

    dev_timer_start_with_callback(APP_CMD_HANDLE_DELAY_TIMER_ID, 0, (timer_func_t)app_cmd_handle_delay_timer_cb);
}

/**
 * @description: Stepless adjustment level value, if the onoff status changed, need to clear count down time and report attribute
 * @param {IN MOVE_MODE_T move_mode} Brighten/Darken
 * @param {IN USHORT_T Rate} Stepless adjustment speed, the unit is Rate/1000ms
 * @param {IN BOOL_T with_onoff} affect the switch or not
 * @return: none
 */
void app_cmd_move_level_handler(MOVE_MODE_T move_mode, USHORT_T Rate, BOOL_T with_onoff)
{
    UCHAR_T onoff;
    UCHAR_T level;

    op_light_ctrl_data_switch_get(&onoff);

    if (move_mode == MOVE_MODE_UP) {
        level = 254;
        if (with_onoff == TRUE) { //moveup + withoff update the onoff attribute immediately
            op_light_ctrl_data_switch_get(&onoff);
            if (onoff != TRUE) {
                onoff = TRUE;
                report_onoff_value(QOS_1, 0, onoff);
                op_light_ctrl_data_count_down_set(0);       //Stop countdown
                report_count_down_data(QOS_1, 0, 0);

                dev_zigbee_write_attribute(1,\
                                            CLUSTER_ON_OFF_CLUSTER_ID,\
                                            ATTR_ON_OFF_ATTRIBUTE_ID,\
                                            &onoff,\
                                            ATTR_BOOLEAN_ATTRIBUTE_TYPE);
            }
        }
    } else {
        level = 1; //onoff the property is updated when the move is completed or stop
    }

    dev_zigbee_write_attribute(1,\
                                CLUSTER_LEVEL_CONTROL_CLUSTER_ID,\
                                ATTR_CURRENT_LEVEL_ATTRIBUTE_ID,\
                                &level,\
                                ATTR_INT8U_ATTRIBUTE_TYPE);

    USHORT_T ty_rate = zig_b_2_app_B(Rate);
    OPERATE_RET ret = op_light_ctrl_data_level_move(move_mode, ty_rate, with_onoff);
    if (ret != OPRT_OK) {
        PR_ERR("Zigbee level move no effect");
        return;
    }
    dev_timer_start_with_callback(APP_CMD_HANDLE_DELAY_TIMER_ID, APP_CMD_HANDLE_DELAY_TIME, (timer_func_t)app_cmd_handle_delay_timer_cb);
}


/**
 * @description: The dimming end callback, usually because the dimming process receives a STOP command, or the dimming process ends
 * @param {type} none
 * @return: none
 */
void light_data_move_complete_cb(void)
{
    SEND_QOS_T QOS = QOS_VIP_0;
    uint32_t delay_ms = 0;
    app_hsv_t report_value;
    UCHAR_T level;
    COLOR_HSV_T hsv_value;

    app_print("Move Complete Callback...");

    LIGHT_CTRL_DATA_T* ctrl_data = op_light_ctrl_data_get();

//onoff , Tuya onoff don't effect the switch status
    dev_zigbee_write_attribute(1,\
                                CLUSTER_ON_OFF_CLUSTER_ID,\
                                ATTR_ON_OFF_ATTRIBUTE_ID,\
                                &(ctrl_data->onoff_status),\
                                ATTR_INT8U_ATTRIBUTE_TYPE);
    report_onoff_value(QOS, delay_ms, ctrl_data->onoff_status);
//Tuya mode
    dev_zigbee_write_attribute(1,
                                CLUSTER_COLOR_CONTROL_CLUSTER_ID,
                                TY_ATTR_LIGHT_MODE_ATTRIBUTE_ID,
                                &(ctrl_data->Mode),
                                ATTR_INT8U_ATTRIBUTE_TYPE);
    report_mode_value(QOS, delay_ms, ctrl_data->Mode);
//Tuya color light HSV
    memcpy(&hsv_value, &ctrl_data->color_hsv, sizeof(COLOR_HSV_T));
    dev_zigbee_write_attribute(1,\
                                CLUSTER_COLOR_CONTROL_CLUSTER_ID,\
                                TY_ATTR_COLOR_HSV_ATTRIBUTE_ID,\
                                &hsv_value,\
                                ATTR_INT48U_ATTRIBUTE_TYPE);
    report_value.H = ctrl_data->color_hsv.Hue;
    report_value.S = ctrl_data->color_hsv.Saturation;
    report_value.V = ctrl_data->color_hsv.Value;
    report_hsv_value(QOS, delay_ms, &report_value);

//standard level
    level = app_B_2_zig_b(ctrl_data->color_hsv.Value);
    if (level < 4) {
        level = 1;
    }

    dev_zigbee_write_attribute(1,\
                                CLUSTER_LEVEL_CONTROL_CLUSTER_ID,\
                                ATTR_CURRENT_LEVEL_ATTRIBUTE_ID,\
                                &level,\
                                ATTR_INT8U_ATTRIBUTE_TYPE);

//standard Hue
    level = app_H_2_zig_h(ctrl_data->color_hsv.Hue);
    dev_zigbee_write_attribute(1,\
                            CLUSTER_COLOR_CONTROL_CLUSTER_ID,\
                            ATTR_COLOR_CONTROL_CURRENT_HUE_ATTRIBUTE_ID,\
                            &level,\
                            ATTR_INT8U_ATTRIBUTE_TYPE);


//standard Saturation
    level = app_S_2_zig_s(ctrl_data->color_hsv.Saturation);
    dev_zigbee_write_attribute(1,\
                            CLUSTER_COLOR_CONTROL_CLUSTER_ID,\
                            ATTR_COLOR_CONTROL_CURRENT_SATURATION_ATTRIBUTE_ID,\
                            &level,\
                            ATTR_INT8U_ATTRIBUTE_TYPE);

}

/**
 * @description: Step adjustment brightness command processing
 * @param {IN UCHAR_T step_mode} 0：Step increase; 1：Step reduction
 * @param {IN UCHAR_T step_size} step value
 * @param {IN UINT_T trans_time} delay_time value
 * @param {IN bool_t with_onoff} affect the switch or not
 * @return: none
 */
void app_cmd_step_level_handler(UCHAR_T step_mode, UCHAR_T step_size, UINT_T trans_time, bool_t with_onoff)
{
    UCHAR_T curr_zig_bright = 0;
    UCHAR_T new_zig_bright = 0;

    dev_zigbee_read_attribute(1, \
                        CLUSTER_LEVEL_CONTROL_CLUSTER_ID, \
                        ATTR_CURRENT_LEVEL_ATTRIBUTE_ID, \
                        &curr_zig_bright, \
                        sizeof(curr_zig_bright));


    if (step_mode == 0) {  //up
        if (curr_zig_bright + step_size > ZIGBEE_WHITE_B_MAX) {
            new_zig_bright = ZIGBEE_WHITE_B_MAX;
        } else {
            new_zig_bright = curr_zig_bright + step_size;
        }
    } else {  //down
        if ((curr_zig_bright - step_size) <0) {
            new_zig_bright = 0;  
        } else {
            new_zig_bright = curr_zig_bright - step_size;
        }
    }

    app_cmd_move_to_level_handler(new_zig_bright, trans_time, with_onoff);
}


/**
 * @description: the gateway level cmd handler 
 * @param {IN CLUSTER_ID_T cluster_id} the cluster which care about level 
 *  * @param {IN attr_value_t *payload} attribute value
 * @return: none
 */
OPERATE_RET level_cluster_handler(UCHAR_T cmd, attr_value_t *payload, ZIGBEE_CMD_T cmd_type)
{
    UCHAR_T onoff;

    attr_value_t attr_data_list;
    memcpy(&attr_data_list, payload, sizeof(attr_value_t));
    app_print("\r\n ---   %x   --- \r\n", cmd);
    op_light_ctrl_data_switch_get(&onoff);
    switch (cmd) {
        case CMD_MOVE_TO_LEVEL_COMMAND_ID:
        case CMD_MOVE_TO_LEVEL_WITH_ON_OFF_COMMAND_ID:{ 
        UCHAR_T level, option_mask = 0, option_override = 0, option;
        USHORT_T trans_time;
        level = attr_data_list.value[0];
        trans_time = attr_data_list.value[1] + (attr_data_list.value[2]<<8);

        if(attr_data_list.value_len == 5){
            option_mask = attr_data_list.value[attr_data_list.value_len-2];
            option_override = attr_data_list.value[attr_data_list.value_len-1];
        }
        option = get_option_value(CLUSTER_LEVEL_CONTROL_CLUSTER_ID, option_mask, option_override);

        if(cmd == CMD_MOVE_TO_LEVEL_WITH_ON_OFF_COMMAND_ID){
            app_cmd_move_to_level_handler(level, trans_time, TRUE);
        }
        if(1 == onoff || option == 0x01){ 
            app_cmd_move_to_level_handler(level, trans_time, FALSE);
        }
        break;
        }
        case CMD_MOVE_COMMAND_ID:
        case CMD_MOVE_WITH_ON_OFF_COMMAND_ID:{  //move mode + rate
            UCHAR_T move_mode, option_mask = 0, option_override = 0, option;
            UCHAR_T move_rate;

            move_mode = attr_data_list.value[0];
            move_rate = attr_data_list.value[1];
            
            if(attr_data_list.value_len == 4){
                option_mask = attr_data_list.value[attr_data_list.value_len-2];
                option_override = attr_data_list.value[attr_data_list.value_len-1];
            }
            option = get_option_value(CLUSTER_LEVEL_CONTROL_CLUSTER_ID, option_mask, option_override);
            if (cmd == CMD_MOVE_WITH_ON_OFF_COMMAND_ID) {
                app_cmd_move_level_handler((MOVE_MODE_T)move_mode, move_rate, TRUE);
            } else {
                if (1 == onoff || option == 0x01) { // light on, need to handler
                    app_cmd_move_level_handler((MOVE_MODE_T)move_mode, move_rate, FALSE);
                }
            }
            break;
        }
        case CMD_STEP_COMMAND_ID:             //   don't effect switch
        case CMD_STEP_WITH_ON_OFF_COMMAND_ID:{ //  effect switch
            UCHAR_T option_mask = 0, option_override = 0, option;
            UCHAR_T step_mode, step_size;
            USHORT_T trans_time;

            step_mode = attr_data_list.value[0];
            step_size = attr_data_list.value[1];
            trans_time = attr_data_list.value[2] + (attr_data_list.value[3]<<8);

            if( attr_data_list.value_len == 6){
                option_mask = attr_data_list.value[attr_data_list.value_len-2];
                option_override = attr_data_list.value[attr_data_list.value_len-1];
            }
            option = get_option_value(CLUSTER_LEVEL_CONTROL_CLUSTER_ID, option_mask, option_override);
            if (cmd == CMD_STEP_WITH_ON_OFF_COMMAND_ID) {
                app_cmd_step_level_handler(step_mode, step_size, trans_time, TRUE);

            } else {
                if (onoff == 1 || option == 0x01) {
                    app_cmd_step_level_handler(step_mode, step_size, trans_time, FALSE);
                }
            }
            break;
        }
        case CMD_STOP_COMMAND_ID:
        case CMD_STOP_WITH_ON_OFF_COMMAND_ID:{

            op_light_ctrl_data_move_stop();

            break;
        }
        default:
            return ZCL_CMD_RET_FAILED;
        break;
    }

    return OPRT_OK;
}