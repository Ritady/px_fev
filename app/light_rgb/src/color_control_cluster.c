/*
 * @Author: xiaojia
 * @email: limu.xiao@tuya.com
 * @LastEditors:
 * @file name: color_control_cluster.c
 * @Description: color_control_cluster attribute report and gateway cmd handler source file
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2021.8.30 16:46:11
 * @LastEditTime: 2021.9.1 20:46:51
 */

#include "color_control_cluster.h"
#include "dev_register.h"
#include "light_control.h"
#include "light_tools.h"
#include "app_common.h"
#include "onoff_cluster.h"


/***********************************************************************************************************
 * 
 *                       Basic function
 * 
 **********************************************************************************************************/
/**
 * @description: calculate the length of Hue need to move 
 * @param {move_mode} forward/reverse rotation
 * @param {curr_hue} Current Hue value
 * @param {tar_hue} target Hue value
 * @return: distance The length that needs to be moved
 */
static USHORT_T calc_hue_move_distance(MOVE_MODE_T move_mode, UINT_T curr_hue, UINT_T tar_hue)
{
    USHORT_T distance;
    if (curr_hue == tar_hue) {
        return 0;
    }

    if (move_mode == MOVE_MODE_UP) {
        if (tar_hue > curr_hue) {
            distance = tar_hue - curr_hue;
        } else {
            distance = 360 - (curr_hue - tar_hue);
        }
    } else {
        if (curr_hue > tar_hue) {
            distance = curr_hue - tar_hue;
        } else {
            distance = 360 - (tar_hue - curr_hue);
        }
    }
    return distance;
}



/***********************************************************************************************************
 * 
 *          device repoet their attribute to the gateway
 * 
 **********************************************************************************************************/

/**
 * @description: report mode attribute value
 * @param {IN SEND_QOS_T Qos} QOS value
 * @param {IN UINT_T delay_ms} delay report
 * @param {IN UCHAR_T mode} mode value
 * @return: none
 */
void report_mode_value(SEND_QOS_T Qos, UINT_T delay_ms, UCHAR_T mode)
{
    dev_send_data_t send_data;
    memset(&send_data, 0, sizeof(dev_send_data_t));
    
    send_data.qos = Qos;
    send_data.delay_time = delay_ms;
    send_data.zcl_id = ZCL_ID_MODE;
    send_data.direction = ZCL_DATA_DIRECTION_SERVER_TO_CLIENT;
    send_data.command_id = CMD_REPORT_ATTRIBUTES_COMMAND_ID;
    send_data.addr.mode = SEND_MODE_GW;
    send_data.addr.type.gw.cluster_id = CLUSTER_COLOR_CONTROL_CLUSTER_ID; 
    send_data.addr.type.gw.src_ep = 1;
    send_data.data.zg.attr_sum = 1;
    send_data.data.zg.attr[0].attr_id = TY_ATTR_LIGHT_MODE_ATTRIBUTE_ID;
    send_data.data.zg.attr[0].type = ATTR_INT8U_ATTRIBUTE_TYPE;
    send_data.data.zg.attr[0].value_size = 1;
    send_data.data.zg.attr[0].value[0] = mode;

    dev_zigbee_clear_send_data(ZG_CLEAR_ALL_ZCL_ID, &send_data.zcl_id);
    dev_zigbee_send_data(&send_data,NULL,1000);
    PR_ERR("Report mode QOS:%d VALUE %d",Qos,mode);
}

/**
 * @description: report  hsv  attribute value
 * @param {IN SEND_QOS_T Qos} QOS value
 * @param {IN UINT_T delay_ms} delay report
 * @param {IN zig_hsv} HSV  value
 * @return: none
 */
void report_hsv_value(SEND_QOS_T Qos, UINT_T delay_ms, app_hsv_t* zig_hsv)
{
    dev_send_data_t send_data;
    memset(&send_data, 0, sizeof(dev_send_data_t));

    send_data.qos = Qos;
    send_data.delay_time = delay_ms;
    send_data.zcl_id = ZCL_ID_HSV;
    send_data.direction = ZCL_DATA_DIRECTION_SERVER_TO_CLIENT;
    send_data.command_id = CMD_REPORT_ATTRIBUTES_COMMAND_ID;
    send_data.addr.mode = SEND_MODE_GW;
    send_data.addr.type.gw.cluster_id = CLUSTER_COLOR_CONTROL_CLUSTER_ID;
    send_data.addr.type.gw.src_ep = 1;
    send_data.data.zg.attr_sum = 1;
    send_data.data.zg.attr[0].attr_id = TY_ATTR_COLOR_HSV_ATTRIBUTE_ID;
    send_data.data.zg.attr[0].type = ATTR_INT48U_ATTRIBUTE_TYPE;
    send_data.data.zg.attr[0].value_size = 6;
    send_data.data.zg.attr[0].value[0] = (UCHAR_T)(zig_hsv->H);
    send_data.data.zg.attr[0].value[1] = (UCHAR_T)(zig_hsv->H >> 8);
    send_data.data.zg.attr[0].value[2] = (UCHAR_T)(zig_hsv->S);
    send_data.data.zg.attr[0].value[3] = (UCHAR_T)(zig_hsv->S >> 8);
    send_data.data.zg.attr[0].value[4] = (UCHAR_T)(zig_hsv->V);
    send_data.data.zg.attr[0].value[5] = (UCHAR_T)(zig_hsv->V >> 8);

    dev_zigbee_clear_send_data(ZG_CLEAR_ALL_ZCL_ID, &send_data.zcl_id);
    dev_zigbee_send_data(&send_data,NULL,1000);
    app_print("Report HSV QOS:%d  VALUE: %d %d %d\r\n", Qos,zig_hsv->H, zig_hsv->S, zig_hsv->V);
}


/***********************************************************************************************************
 * 
 *          gateway send cmd to device, device handler the cmd functions
 * 
 **********************************************************************************************************/

/**
 * @description: app set mode command handler
 * @param {IN UCHAR_T mode} mode : whitelight, colorlight, scenemode, musicmode
 * @return: none
 */
void app_cmd_set_mode_handler(UCHAR_T mode)
{
    SEND_QOS_T QOS = QOS_VIP_0;
    UINT_T delay_ms = 0;
    UCHAR_T onoff = 1;

    PR_ERR("Receive  mode : %d", mode);

    report_mode_value(QOS, delay_ms, mode);

    OPERATE_RET ret1 = op_set_onoff_with_countdown_cmd(TRUE, QOS, delay_ms);//If the switch changes, report the switch and countdown
    OPERATE_RET ret2 = op_light_ctrl_data_mode_set((LIGHT_MODE_E)mode);
    if (ret1 != OPRT_OK && ret2 != OPRT_OK) {
        return;
    }
    //updata onoff attribute
    dev_zigbee_write_attribute(1,\
                            CLUSTER_ON_OFF_CLUSTER_ID,\
                            ATTR_ON_OFF_ATTRIBUTE_ID,\
                            &onoff,\
                            ATTR_INT8U_ATTRIBUTE_TYPE);

    //updata mode attribute
    dev_zigbee_write_attribute(1,\
                               CLUSTER_COLOR_CONTROL_CLUSTER_ID,\
                               TY_ATTR_LIGHT_MODE_ATTRIBUTE_ID,\
                               &mode,\
                               ATTR_INT8U_ATTRIBUTE_TYPE);
    //updata app data attribute
    report_mode_value(QOS, delay_ms, mode);

    dev_timer_start_with_callback(APP_CMD_HANDLE_DELAY_TIMER_ID, APP_CMD_HANDLE_DELAY_TIME, (timer_func_t)app_cmd_handle_delay_timer_cb);
}

/**
 * @description: app move to level (set hsv)command handler
 * @param {IN app_hsv_t* ty_hsv} hsv value
 * @return: none
 */
void app_cmd_set_hsv_handler(CMD_TYPE_T cmd_type, app_hsv_t* ty_hsv)
{
    SEND_QOS_T QOS = QOS_VIP_0;
    uint32_t delay_ms = 0;
    UCHAR_T onoff = 1;
    LIGHT_MODE_E mode = COLOR_MODE;
    if(cmd_type == ZIGBEE_CMD_GROUP){
        delay_ms = 10000 + random_ms(10000);
        QOS = QOS_1;
    }

    report_hsv_value(QOS, delay_ms, ty_hsv);

    op_set_onoff_with_countdown_cmd(TRUE, QOS, delay_ms);
    op_light_ctrl_data_hsv_set((COLOR_HSV_T*)ty_hsv);
    OPERATE_RET ret = op_light_ctrl_data_mode_set(mode);    
    if(ret == OPRT_OK){
        report_mode_value(QOS, delay_ms, mode);
    }

    //updata onoff attribute
    dev_zigbee_write_attribute(1,\
                            CLUSTER_ON_OFF_CLUSTER_ID,\
                            ATTR_ON_OFF_ATTRIBUTE_ID,\
                            &onoff,\
                            ATTR_INT8U_ATTRIBUTE_TYPE);

    report_onoff_value(QOS, delay_ms, mode);

    //updata mode attribute
    dev_zigbee_write_attribute(1,\
                            CLUSTER_COLOR_CONTROL_CLUSTER_ID,\
                            TY_ATTR_LIGHT_MODE_ATTRIBUTE_ID,\
                            &mode,\
                            ATTR_INT8U_ATTRIBUTE_TYPE);

    report_mode_value(QOS, delay_ms, onoff);


    //updata hsv attribute
    dev_zigbee_write_attribute(1,\
                            CLUSTER_COLOR_CONTROL_CLUSTER_ID,\
                            TY_ATTR_COLOR_HSV_ATTRIBUTE_ID,\
                            ty_hsv,\
                            ATTR_INT48U_ATTRIBUTE_TYPE);
                            
    report_hsv_value(QOS, delay_ms, ty_hsv);

    dev_timer_start_with_callback(APP_CMD_HANDLE_DELAY_TIMER_ID, APP_CMD_HANDLE_DELAY_TIME, (timer_func_t)app_cmd_handle_delay_timer_cb);
}

/**
 * @description: Tuya real-time data command
 * @param {IN UCHAR_T data} Real-time data HSVBT
 * @param {IN UCHAR_T len} Data length
 * @return: none
 */
void app_cmd_set_real_time_ctrl_handler(UCHAR_T *data, UCHAR_T len)
{
    SEND_QOS_T send_qos = QOS_1;
    uint32_t delay_ms = 0;
    UCHAR_T string[22] = {0};
    LIGHT_MODE_E curr_mode;
    UCHAR_T onoff;

    if (len != 21) {
        PR_ERR("real_time_data_len_error: %d ", len);
        return ;
    }

    memcpy(string, data, 21);
    string[21] = '\0';
    app_print("Receive real time data: %s", string);

    op_light_ctrl_data_mode_get(&curr_mode);
    op_light_ctrl_data_switch_get(&onoff);

    OPERATE_RET ret1 = op_set_onoff_with_countdown_cmd(TRUE, send_qos, delay_ms);
    OPERATE_RET ret2 = op_light_ctrl_data_realtime_adjust_set(0, string);
    op_light_realtime_ctrl_proc();
}

/**
 * @description: Stepless adjustment Hue, this command mean that when the Hue adjusted at a special value should be stoped
 * @param {IN MOVE_MODE_T move_mode} up / down
 * @param {IN UCHAR_T hue} Target Hue
 * @param {IN USHORT_T trans_time} Gradual time, unit 100ms
 * @return: none
 */
void app_cmd_move_to_or_step_hue_handler(MOVE_MODE_T move_mode, UCHAR_T hue, UINT_T trans_time)
{
    SEND_QOS_T QOS = QOS_VIP_0;
    uint32_t delay_ms = 0;
    USHORT_T ty_hue;
    app_hsv_t report_value;
    COLOR_HSV_T ty_hsv;
    UCHAR_T color_mode = 0x00;
  
    ty_hue = zig_h_2_app_H(hue);
    app_print("\r\n set hue = %d\r\n", hue);
    op_light_ctrl_data_hsv_get(&ty_hsv);
    OPERATE_RET ret1 = op_light_ctrl_data_hue_set(ty_hue, move_mode,trans_time);
    OPERATE_RET ret2 = op_light_ctrl_data_mode_set(COLOR_MODE);     //Switch to color_mode

    if (ret1 != OPRT_OK && ret2 != OPRT_OK) {
        PR_ERR("Zigbee Hue set no effect");
        return;
    }


    //update normal hue attribute
    dev_zigbee_write_attribute(1,\
                                CLUSTER_COLOR_CONTROL_CLUSTER_ID,\
                                ATTR_COLOR_CONTROL_CURRENT_HUE_ATTRIBUTE_ID,\
                                &hue,\
                                ATTR_INT8U_ATTRIBUTE_TYPE);
    //update normal mode attribute
    dev_zigbee_write_attribute(1,\
                                CLUSTER_COLOR_CONTROL_CLUSTER_ID,\
                                ATTR_COLOR_CONTROL_COLOR_MODE_ATTRIBUTE_ID,\
                                &color_mode,\
                                ATTR_ENUM8_ATTRIBUTE_TYPE);

    dev_zigbee_write_attribute(1,\
                                CLUSTER_COLOR_CONTROL_CLUSTER_ID,\
                                ATTR_COLOR_CONTROL_ENHANCED_COLOR_MODE_ATTRIBUTE_ID,\
                                &color_mode,\
                                ATTR_ENUM8_ATTRIBUTE_TYPE);
    //update tuya HSV attribuite
    app_print("\r\n  hsv h:%d, s:%d, v:%d ", ty_hsv.Hue, ty_hsv.Saturation, ty_hsv.Value);
    ty_hsv.Hue = ty_hue;
    dev_zigbee_write_attribute(1,\
                            CLUSTER_COLOR_CONTROL_CLUSTER_ID,\
                            TY_ATTR_COLOR_HSV_ATTRIBUTE_ID,\
                            &ty_hsv,\
                            ATTR_INT48U_ATTRIBUTE_TYPE);

    //updata tuya mode attribuite
    UCHAR_T ty_mode = COLOR_MODE;
    dev_zigbee_write_attribute(1,\
                        CLUSTER_COLOR_CONTROL_CLUSTER_ID,\
                        TY_ATTR_LIGHT_MODE_ATTRIBUTE_ID,\
                        &ty_mode,\
                        ATTR_INT8U_ATTRIBUTE_TYPE);

    report_mode_value(QOS, delay_ms, ty_mode);

    report_value.H = ty_hue;
    report_value.S = ty_hsv.Saturation;
    report_value.V = ty_hsv.Value;
    report_hsv_value(QOS, 100, &report_value);


    dev_timer_start_with_callback(APP_CMD_HANDLE_DELAY_TIMER_ID, APP_CMD_HANDLE_DELAY_TIME, (timer_func_t)app_cmd_handle_delay_timer_cb);
}

/**
 * @description: Hue stepless adjustment processing, used to process Hue move instructions
 * @param {IN MOVE_MODE_T move_mode} up / down
 * @param {IN UCHAR_T move_rate} Adjustment rate: move_rate/s
 * @return: none
 */
void app_cmd_move_hue_handler(MOVE_MODE_T move_mode, UCHAR_T move_rate)
{
    UCHAR_T color_mode = 0x00;

    OPERATE_RET ret1 = op_light_ctrl_data_hue_move(move_mode, move_rate);
    OPERATE_RET ret2 = op_light_ctrl_data_mode_set(COLOR_MODE);
    if (ret1 != OPRT_OK && ret2 != OPRT_OK) {
        PR_ERR("Zigbee Hue set no effect");
        return;
    }
    //pdate normal mode attribute
    dev_zigbee_write_attribute(1,\
                                CLUSTER_COLOR_CONTROL_CLUSTER_ID,\
                                ATTR_COLOR_CONTROL_COLOR_MODE_ATTRIBUTE_ID,\
                                &color_mode,\
                                ATTR_ENUM8_ATTRIBUTE_TYPE);

    dev_zigbee_write_attribute(1,\
                                CLUSTER_COLOR_CONTROL_CLUSTER_ID,\
                                ATTR_COLOR_CONTROL_ENHANCED_COLOR_MODE_ATTRIBUTE_ID,\
                                &color_mode,\
                                ATTR_ENUM8_ATTRIBUTE_TYPE);
    


    dev_timer_start_with_callback(APP_CMD_HANDLE_DELAY_TIMER_ID, APP_CMD_HANDLE_DELAY_TIME, (timer_func_t)app_cmd_handle_delay_timer_cb);
}

/**
 * @description: Color saturation adjustment processing,use to resolve saturation move to, move, step command
 * @param {IN UCHAR_T saturation} target saturation
 * @param {IN UINT_T trans_time} delay time
 * @return: none
 */
void app_cmd_move_or_step_saturation_handler(UCHAR_T saturation, UINT_T trans_time)
{
    SEND_QOS_T QOS = QOS_VIP_0;
    uint32_t delay_ms = 0;
    app_hsv_t report_value;
    UINT_T ty_saturation;
    UCHAR_T color_mode = 0x00;

    ty_saturation = zig_s_2_app_S(saturation);

    app_print(" \r\n the saturation is %d ", saturation);

    OPERATE_RET ret1 = op_light_ctrl_data_saturation_set(ty_saturation, trans_time);
    OPERATE_RET ret2 = op_light_ctrl_data_mode_set(COLOR_MODE);     

    if (ret1 != OPRT_OK && ret2 != OPRT_OK) {
        PR_ERR("Zigbee saturation set no effect");
        return;
    }

    //updata normal saturation attribute
    dev_zigbee_write_attribute(1,\
                                CLUSTER_COLOR_CONTROL_CLUSTER_ID,\
                                ATTR_COLOR_CONTROL_CURRENT_SATURATION_ATTRIBUTE_ID,\
                                &saturation,\
                                ATTR_INT8U_ATTRIBUTE_TYPE);

    //updata normal mode attribute
    dev_zigbee_write_attribute(1,\
                                CLUSTER_COLOR_CONTROL_CLUSTER_ID,\
                                ATTR_COLOR_CONTROL_COLOR_MODE_ATTRIBUTE_ID,\
                                &color_mode,\
                                ATTR_ENUM8_ATTRIBUTE_TYPE);

    dev_zigbee_write_attribute(1,\
                                CLUSTER_COLOR_CONTROL_CLUSTER_ID,\
                                ATTR_COLOR_CONTROL_ENHANCED_COLOR_MODE_ATTRIBUTE_ID,\
                                &color_mode,\
                                ATTR_ENUM8_ATTRIBUTE_TYPE);
    //updata tuya HSV attribute
    COLOR_HSV_T ty_hsv;
    op_light_ctrl_data_hsv_get(&ty_hsv);
    ty_hsv.Saturation = ty_saturation;

    dev_zigbee_write_attribute(1,\
                            CLUSTER_COLOR_CONTROL_CLUSTER_ID,\
                            TY_ATTR_COLOR_HSV_ATTRIBUTE_ID,\
                            &ty_hsv,\
                            ATTR_INT48U_ATTRIBUTE_TYPE);

    //updata tuya mode attribute
    UCHAR_T ty_mode = COLOR_MODE;
    dev_zigbee_write_attribute(1,\
                        CLUSTER_COLOR_CONTROL_CLUSTER_ID,\
                        TY_ATTR_LIGHT_MODE_ATTRIBUTE_ID,\
                        &ty_mode,\
                        ATTR_INT8U_ATTRIBUTE_TYPE);

    report_value.H = ty_hsv.Hue;
    report_value.S = ty_saturation;
    report_value.V = ty_hsv.Value;
    app_print(" \r\n move or step saturation H:%d, S:%d, V:%d ", report_value.H, report_value.S, report_value.V);
    report_hsv_value(QOS, delay_ms, &report_value);
    report_mode_value(QOS, 100, ty_mode);

    dev_timer_start_with_callback(APP_CMD_HANDLE_DELAY_TIMER_ID, APP_CMD_HANDLE_DELAY_TIME, (timer_func_t)app_cmd_handle_delay_timer_cb);
}

/**
 * @description: Set hue and saturation at the same time
 * @param {IN UCHAR_T hue} target Hue
 * @param {IN UCHAR_T saturation} target saturation 
 * @param {IN UINT_T trans_time} Gradient time, unit 100ms.
 * @return: none
 */
void app_cmd_move_to_hue_and_saturation_handler(UCHAR_T hue, UCHAR_T saturation, UINT_T trans_time)
{
    SEND_QOS_T QOS = QOS_VIP_0;
    uint32_t delay_ms = 0;
    UINT_T ty_hue;
    UINT_T ty_saturation;
    app_hsv_t report_value;

    UCHAR_T color_mode = 0x00;

    ty_hue = zig_h_2_app_H(hue);
    ty_saturation = zig_s_2_app_S(saturation);

    OPERATE_RET ret1 = op_light_ctrl_data_hs_set(ty_hue, ty_saturation, trans_time);
    OPERATE_RET ret2 = op_light_ctrl_data_mode_set(COLOR_MODE);     //force switch to color mode

    if (ret1 != OPRT_OK && ret2 != OPRT_OK) {
        PR_ERR("Zigbee saturation set no effect");
        return;
    }

    //updata normal Saturation attribute
    dev_zigbee_write_attribute(1,\
                                CLUSTER_COLOR_CONTROL_CLUSTER_ID,\
                                ATTR_COLOR_CONTROL_CURRENT_SATURATION_ATTRIBUTE_ID,\
                                &saturation,\
                                ATTR_INT8U_ATTRIBUTE_TYPE);

    //updata normal Hue attribute
    dev_zigbee_write_attribute(1,\
                                CLUSTER_COLOR_CONTROL_CLUSTER_ID,\
                                ATTR_COLOR_CONTROL_CURRENT_HUE_ATTRIBUTE_ID ,\
                                &hue,\
                                ATTR_INT8U_ATTRIBUTE_TYPE);

    //updata normal mode attribute
    dev_zigbee_write_attribute(1,\
                                CLUSTER_COLOR_CONTROL_CLUSTER_ID,\
                                ATTR_COLOR_CONTROL_COLOR_MODE_ATTRIBUTE_ID,\
                                &color_mode,\
                                ATTR_ENUM8_ATTRIBUTE_TYPE);

    dev_zigbee_write_attribute(1,\
                                CLUSTER_COLOR_CONTROL_CLUSTER_ID,\
                                ATTR_COLOR_CONTROL_ENHANCED_COLOR_MODE_ATTRIBUTE_ID,\
                                &color_mode,\
                                ATTR_ENUM8_ATTRIBUTE_TYPE);
    //updata tuya HSV attribute
    COLOR_HSV_T ty_hsv;
    op_light_ctrl_data_hsv_get(&ty_hsv);
    ty_hsv.Saturation = ty_saturation;
    ty_hsv.Hue = ty_hue;
    dev_zigbee_write_attribute(1,\
                            CLUSTER_COLOR_CONTROL_CLUSTER_ID,\
                            TY_ATTR_COLOR_HSV_ATTRIBUTE_ID,\
                            &ty_hsv,\
                            ATTR_INT48U_ATTRIBUTE_TYPE);

    //updata tuya mode attribute
    UCHAR_T ty_mode = COLOR_MODE;
    dev_zigbee_write_attribute(1,\
                        CLUSTER_COLOR_CONTROL_CLUSTER_ID,\
                        TY_ATTR_LIGHT_MODE_ATTRIBUTE_ID,\
                        &ty_mode,\
                        ATTR_INT8U_ATTRIBUTE_TYPE);

    report_value.H = ty_hue;
    report_value.S = ty_saturation;
    report_value.V = ty_hsv.Value;
    report_hsv_value(QOS, delay_ms, &report_value);
    report_mode_value(QOS, 100, ty_mode);

    dev_timer_start_with_callback(APP_CMD_HANDLE_DELAY_TIMER_ID, APP_CMD_HANDLE_DELAY_TIME, (timer_func_t)app_cmd_handle_delay_timer_cb);
}


/**
 * @description: the gateway onoff cmd handler 
 * @param {UCHAR_T cmd} cmd
 * @param {attr_value_t payload} attribute value
 * @param {ZIGBEE_CMD_T cmd_type} type of cmd
 * @return: none
 */
OPERATE_RET color_control_cluster_handler(UCHAR_T cmd, attr_value_t *payload, ZIGBEE_CMD_T cmd_type)
{
    UCHAR_T onoff;
    op_light_ctrl_data_switch_get(&onoff);
    attr_value_t attr_data_list;
    UCHAR_T option_mask = 0, option_override = 0, option;
    memcpy(&attr_data_list, payload, sizeof(attr_value_t));
    app_print("\r\n ---   %x   --- \r\n", cmd);
    switch (cmd) {
        case CMD_MOVE_TO_HUE_COMMAND_ID:{
            if ( attr_data_list.value_len == 6) {
                option_mask = attr_data_list.value[attr_data_list.value_len-2];
                option_override = attr_data_list.value[attr_data_list.value_len-1];
            }
            option = get_option_value(CLUSTER_COLOR_CONTROL_CLUSTER_ID, option_mask, option_override);
            if (onoff == 1 || option == 0x01) {
                UCHAR_T curr_hue;
                UCHAR_T tar_hue = attr_data_list.value[0];
                UCHAR_T move_move = attr_data_list.value[1];
                USHORT_T trans_time = attr_data_list.value[2] + (attr_data_list.value[3]<<8);

                dev_zigbee_read_attribute(1, \
                                            CLUSTER_COLOR_CONTROL_CLUSTER_ID, \
                                            ATTR_COLOR_CONTROL_CURRENT_HUE_ATTRIBUTE_ID, \
                                            &curr_hue, \
                                            sizeof(curr_hue));

                switch(move_move) {
                    case 0:{ //Rotate at the shortest path
                        USHORT_T up_distance = calc_hue_move_distance(MOVE_MODE_UP, curr_hue, tar_hue);
                        USHORT_T down_distance = calc_hue_move_distance(MOVE_MODE_DOWN, curr_hue, tar_hue);
                        if (up_distance > down_distance) {
                            move_move = MOVE_MODE_DOWN;
                        } else {
                            move_move = MOVE_MODE_UP;
                        }
                        break;
                    }
                    case 1:{ //Rotate at the longest path
                        USHORT_T up_distance = calc_hue_move_distance(MOVE_MODE_UP, curr_hue, tar_hue);
                        USHORT_T down_distance = calc_hue_move_distance(MOVE_MODE_DOWN, curr_hue, tar_hue);
                        if (up_distance >= down_distance) {
                            move_move = MOVE_MODE_UP;
                        } else {
                            move_move = MOVE_MODE_DOWN;
                        }
                        break;
                    }
                    case 2:{ //Rotate in increasing direction, counterclockwise
                        move_move = MOVE_MODE_UP;
                    }
                        break;
                    case 3:{ //Rotate in the decreasing direction,Clockwise
                        move_move = MOVE_MODE_DOWN;
                    }
                        break;
                    default:
                        move_move = MOVE_MODE_UP;
                    break;
                }
                app_cmd_move_to_or_step_hue_handler((MOVE_MODE_T)move_move, tar_hue, trans_time);
            }
            break;
        }
        case CMD_MOVE_HUE_COMMAND_ID:{
            if ( attr_data_list.value_len == 4) {
                option_mask = attr_data_list.value[attr_data_list.value_len-2];
                option_override = attr_data_list.value[attr_data_list.value_len-1];
            }
            option = get_option_value(CLUSTER_COLOR_CONTROL_CLUSTER_ID, option_mask, option_override);
            if (onoff == 1 || option == 0x01) {
                UCHAR_T move_mode = attr_data_list.value[0];
                UCHAR_T move_rate = attr_data_list.value[1];
                if (move_rate == 0) {
                    return ZCL_CMD_RET_FAILED;
                }
                if (move_mode == 0) {
                    op_light_ctrl_data_move_stop();
                }else if (move_mode == 1) {
                    app_cmd_move_hue_handler(MOVE_MODE_UP, move_rate);
                }else if (move_mode == 3) {
                    app_cmd_move_hue_handler(MOVE_MODE_DOWN, move_rate);
                } else {
                    return ZCL_CMD_RET_FAILED;
                }
            }
            break;
        }
        case CMD_STEP_HUE_COMMAND_ID:{
            if ( attr_data_list.value_len == 5) {
                option_mask = attr_data_list.value[attr_data_list.value_len-2];
                option_override = attr_data_list.value[attr_data_list.value_len-1];
            }
            option = get_option_value(CLUSTER_COLOR_CONTROL_CLUSTER_ID, option_mask, option_override);
            if (onoff == 1 || option == 0x01) {
                UCHAR_T curr_hue, tar_hue;
                UCHAR_T step_mode = attr_data_list.value[0];
                UCHAR_T step_size = attr_data_list.value[1];
                UCHAR_T trans_time = attr_data_list.value[2];
                dev_zigbee_read_attribute(1, \
                                            CLUSTER_COLOR_CONTROL_CLUSTER_ID, \
                                            ATTR_COLOR_CONTROL_CURRENT_HUE_ATTRIBUTE_ID, \
                                            &curr_hue, \
                                            sizeof(curr_hue));
                if (step_mode == 0x01) { // step up
                    if (curr_hue + step_size > 254) {
                        tar_hue = curr_hue + step_size - 254;
                    } else {
                        tar_hue = curr_hue + step_size;
                    }
                    app_cmd_move_to_or_step_hue_handler(MOVE_MODE_UP, tar_hue, trans_time);
                }else if (step_mode == 0x03) { // step dowm
                    if (curr_hue < step_size) {
                        tar_hue = 254 - (step_size - curr_hue);
                    } else { 
                        tar_hue = curr_hue - step_size;
                    }
                    app_cmd_move_to_or_step_hue_handler(MOVE_MODE_DOWN, tar_hue, trans_time);
                } else {
                    return ZCL_CMD_RET_FAILED;
                }
            }
            break;
        }
        case CMD_MOVE_TO_SATURATION_COMMAND_ID:{
            if ( attr_data_list.value_len == 5) {
                option_mask = attr_data_list.value[attr_data_list.value_len-2];
                option_override = attr_data_list.value[attr_data_list.value_len-1];
            }
            option = get_option_value(CLUSTER_COLOR_CONTROL_CLUSTER_ID, option_mask, option_override);
            if (onoff == 1 || option == 0x01) {
                UCHAR_T saturation = attr_data_list.value[0];
                USHORT_T train_time = attr_data_list.value[1] + (attr_data_list.value[2]<<8);

                app_cmd_move_or_step_saturation_handler(saturation, train_time);
            }
            break;
        }
        case CMD_MOVE_SATURATION_COMMAND_ID:{
            if ( attr_data_list.value_len == 4) {
                option_mask = attr_data_list.value[attr_data_list.value_len-2];
                option_override = attr_data_list.value[attr_data_list.value_len-1];
            }
            option = get_option_value(CLUSTER_COLOR_CONTROL_CLUSTER_ID, option_mask, option_override);
            if (onoff == 1 || option == 0x01) {
                UCHAR_T move_mode = attr_data_list.value[0];
                USHORT_T move_rate = attr_data_list.value[1];
                UCHAR_T cur_saturation;
                UCHAR_T tar_saturation;

                dev_zigbee_read_attribute(1, \
                                            CLUSTER_COLOR_CONTROL_CLUSTER_ID, \
                                            ATTR_COLOR_CONTROL_CURRENT_SATURATION_ATTRIBUTE_ID, \
                                            &cur_saturation, \
                                            sizeof(cur_saturation));
                if (move_mode == 0x00) {

                    op_light_ctrl_data_move_stop();
                    return ZCL_CMD_RET_SUCCESS;

                }else if (move_mode == 0x01) {//move up

                    tar_saturation = 0xFE;

                }else if (move_mode == 0x03) {//move down

                    tar_saturation = 0;

                }
                if (move_rate == 0) {
                  move_rate = 1000;
                }
                USHORT_T trans_time = light_tool_get_abs_value(cur_saturation - tar_saturation) * 10 / move_rate;
                app_cmd_move_or_step_saturation_handler(tar_saturation, trans_time);
            }

            break;
        }
        case CMD_STEP_SATURATION_COMMAND_ID:{
            if ( attr_data_list.value_len == 5) {
                option_mask = attr_data_list.value[attr_data_list.value_len-2];
                option_override = attr_data_list.value[attr_data_list.value_len-1];
            }
            option = get_option_value(CLUSTER_COLOR_CONTROL_CLUSTER_ID, option_mask, option_override);
            if (onoff == 1 || option == 0x01) {
                UCHAR_T step_mode = attr_data_list.value[0];
                UCHAR_T step_size = attr_data_list.value[1];
                UCHAR_T trans_time = attr_data_list.value[2];

                UCHAR_T cur_saturation;
                UCHAR_T tar_saturation;

                dev_zigbee_read_attribute(1, \
                                            CLUSTER_COLOR_CONTROL_CLUSTER_ID, \
                                            ATTR_COLOR_CONTROL_CURRENT_SATURATION_ATTRIBUTE_ID, \
                                            &cur_saturation, \
                                            sizeof(cur_saturation));
                if (step_mode == 0x01) {  //step up

                    if (cur_saturation + step_size >= 0xFE) {
                        tar_saturation = 0xFE;
                    } else {
                        tar_saturation = cur_saturation + step_size;
                    }
                }else if (step_mode == 0x03) {//step down
                    if (cur_saturation < step_size) {
                        tar_saturation = 0;
                    } else {
                        tar_saturation = cur_saturation - step_size;
                    }
                } else {
                    return ZCL_CMD_RET_FAILED;
                }

                app_cmd_move_or_step_saturation_handler(tar_saturation, trans_time);
            }
            break;
        }
        case CMD_MOVE_TO_HUE_AND_SATURATION_COMMAND_ID:{
            if ( attr_data_list.value_len == 6) {
                option_mask = attr_data_list.value[attr_data_list.value_len-2];
                option_override = attr_data_list.value[attr_data_list.value_len-1];
            }
            option = get_option_value(CLUSTER_COLOR_CONTROL_CLUSTER_ID, option_mask, option_override);
            if (onoff == 1 || option == 0x01) {
                UCHAR_T tar_hue = attr_data_list.value[0];
                UCHAR_T tar_saturation = attr_data_list.value[1];
                USHORT_T trans_time = attr_data_list.value[2] + (attr_data_list.value[3]<<8);

                app_cmd_move_to_hue_and_saturation_handler(tar_hue, tar_saturation, trans_time);
            }
            break;
        }
        case CMD_STOP_MOVE_STEP_COMMAND_ID:{

            op_light_ctrl_data_move_stop();

            break;
        }
        case TY_CMD_SET_COLOR_MODE_CMD_ID: {
            if(onoff != 1) {
                return ZCL_CMD_RET_FAILED;
            }
            uint8_t mode = attr_data_list.value[0];
            app_cmd_set_mode_handler(mode);
            break;
        }
        case  TY_CMD_SET_COLOR_HSV_CMD_ID :{
            app_hsv_t hsv;
            hsv.H = attr_data_list.value[0] + (attr_data_list.value[1]<<8);
            hsv.S = attr_data_list.value[2] + (attr_data_list.value[3]<<8);
            hsv.V = attr_data_list.value[4] + (attr_data_list.value[5]<<8);
            app_cmd_set_hsv_handler((CMD_TYPE_T)cmd_type, &hsv);
            break;
        }
        case TY_CMD_SET_COLOR_REALTIME_DATA_CMD_ID:{
            app_print(" \r\n  --- in tuya cmd set color realtime data --- ");
            app_cmd_set_real_time_ctrl_handler(attr_data_list.value, attr_data_list.value_len);

            cmd_control_shade_flag = 0;//No need to write flash after real-time data command
            break;
        }
        default:
            return ZCL_CMD_RET_FAILED;
        break;
    }
    return OPRT_OK;
}