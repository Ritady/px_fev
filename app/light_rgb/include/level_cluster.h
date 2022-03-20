/*
 * @Author: xiaojia
 * @email: limu.xiao@tuya.com
 * @LastEditors:
 * @file name: level_cluster.h
 * @Description: level cluster attribute report and gateway cmd handler include file
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2021.8.30 16:50:12
 * @LastEditTime: 2021.8.31 11:00:48
 */

#ifndef __LEVEL_CLUSTER_H__
#define __LEVEL_CLUSTER_H__

#include "tuya_zigbee_stack.h"
#include "tuya_zigbee_sdk.h"
#include "light_control.h"


#define ZCL_ID_BRIGHT  107

/**
 * **********************************************************************************************************
 * 
 *          gateway send cmd to device, device handler the cmd functions
 * 
 ***********************************************************************************************************
 */


/**
 * @note report level  attribute value
 * @param [in]{SEND_QOS_T} QOS value
 * @param [in]{USHORT_T} delay report
 * @param [in]{UINT_T} level
 * @return: none
 */
void report_level_value(SEND_QOS_T Qos, USHORT_T delay_ms, UINT_T level);

void report_ty_v_value(SEND_QOS_T Qos, USHORT_T delay_ms, UINT_T v_bright);

/**
 * @description: Set a level value
 * @param {IN UINT_T level} level 0 ~ 254
 * @param {IN USHORT_T trans_time} delay_time value 
 * @param {IN bool_t with_onoff} affect the switch or not
 * @return: none
 */
void app_cmd_move_to_level_handler(UINT_T level, USHORT_T trans_time, bool_t with_onoff);

/**
 * @description: Stepless adjustment level value, if the onoff status changed, need to clear count down time and report attribute
 * @param {IN MOVE_MODE_T move_mode} Brighten/Darken
 * @param {IN USHORT_T Rate} Stepless adjustment speed, the unit is Rate/1000ms
 * @param {IN BOOL_T with_onoff} affect the switch or not
 * @return: none
 */
void app_cmd_move_level_handler(MOVE_MODE_T move_mode, USHORT_T Rate, BOOL_T with_onoff);


/**
 * @description: The dimming end callback, usually because the dimming process receives a STOP command, or the dimming process ends
 * @param {type} none
 * @return: none
 */
void light_data_move_complete_cb(void);

/**
 * @description: Step adjustment brightness command processing
 * @param {IN UCHAR_T step_mode} 0：Step increase; 1：Step reduction
 * @param {IN UCHAR_T step_size} step value
 * @param {IN UINT_T trans_time} delay_time value
 * @param {IN bool_t with_onoff} affect the switch or not
 * @return: none
 */
void app_cmd_step_level_handler(UCHAR_T step_mode, UCHAR_T step_size, UINT_T trans_time, bool_t with_onoff);



OPERATE_RET level_cluster_handler(UCHAR_T cmd, attr_value_t *payload, ZIGBEE_CMD_T cmd_type);


#endif