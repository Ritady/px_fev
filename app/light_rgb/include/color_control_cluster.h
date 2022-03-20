/*
 * @Author: xiaojia
 * @email: limu.xiao@tuya.com
 * @LastEditors:
 * @file name: color_control_cluster.h
 * @Description: color_control_cluster attribute report and gateway cmd handler include file
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2021.8.30 16:46:11
 * @LastEditTime: 2021.8.30 20:46:51
 */

#ifndef __COLOR_CONTROL_CLUSTER_H__
#define __COLOR_CONTROL_CLUSTER_H__

#include "tuya_zigbee_sdk.h"
#include "tuya_zigbee_stack.h"
#include "tuya_mcu_os.h"
#include "light_types.h"
#include "dev_register.h"
#include "light_control.h"
#include "app_common.h"
#include "light_tools.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */


#define  ZCL_ID_MODE              101
#define  ZCL_ID_HSV               102
#define  ZCL_ID_SCENE             103
#define  ZCL_ID_APP_DATA          105
#define  ZCL_ID_SCENE_SYNC        106


/***********************************************************************************************************
 * 
 *                       Basic function
 * 
 **********************************************************************************************************/

/**
 * @description: decompress scene data to string format
 * @param {input_buf} compressed scene data
 * @param {input_buf_len} compressed scene data length
 * @param {output_str} decompressed scene data,string format
 * @return: OPERATE_RET
 */
OPERATE_RET op_scene_data_decompress(UCHAR_T* input_buf, UINT_T input_buf_len, CHAR_T* output_str);

/**
 * @description: calculate the length of Hue need to move 
 * @param {move_mode} forward/reverse rotation
 * @param {curr_hue} Current Hue value
 * @param {tar_hue} target Hue value
 * @return: distance The length that needs to be moved
 */
USHORT_T calc_hue_move_distance(MOVE_MODE_T move_mode, UINT_T curr_hue, UINT_T tar_hue);


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
void report_mode_value(SEND_QOS_T Qos, UINT_T delay_ms, UCHAR_T mode);

/**
 * @description: report  hsv  attribute value
 * @param {IN SEND_QOS_T Qos} QOS value
 * @param {IN USHORT_T delay_ms} delay report
 * @param {IN zig_hsv} HSV  value
 * @return: none
 */
void report_hsv_value(SEND_QOS_T Qos, UINT_T delay_ms, app_hsv_t* zig_hsv);

/**
 * @description: report scene data attribute value
 * @param {IN SEND_QOS_T Qos} QOS value
 * @param {IN USHORT_T delay_ms} delay report
 * @param {IN UCHAR_T scene_data} scene_data value
 * @param {IN UCHAR_T length} profile length
 * @return: none
 */
void report_color_scene(SEND_QOS_T Qos, UINT_T delay_ms, UCHAR_T *scene_data, UCHAR_T length);



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
void app_cmd_set_mode_handler(UCHAR_T mode);

/**
 * @description: app move to level (set hsv)command handler
 * @param {IN app_hsv_t* ty_hsv} hsv value
 * @return: none
 */
void app_cmd_set_hsv_handler(CMD_TYPE_T cmd_type, app_hsv_t* ty_hsv);

/**
 * @description: Tuya real-time data command
 * @param {IN UCHAR_T data} Real-time data HSVBT
 * @param {IN UCHAR_T len} Data length
 * @return: none
 */
void app_cmd_set_real_time_ctrl_handler(UCHAR_T *data, UCHAR_T len);

/**
 * @description: Stepless adjustment Hue, this command mean that when the Hue adjusted at a special value should be stoped
 * @param {IN MOVE_MODE_T move_mode} up / down
 * @param {IN UCHAR_T hue} Target Hue
 * @param {IN UINT_T trans_time} Gradual time, unit 100ms
 * @return: none
 */
void app_cmd_move_to_or_step_hue_handler(MOVE_MODE_T move_mode, UCHAR_T hue, UINT_T trans_time);

/**
 * @description: Hue stepless adjustment processing, used to process Hue move instructions
 * @param {IN MOVE_MODE_T move_mode} up / down
 * @param {IN UCHAR_T move_rate} Adjustment rate: moveRate/s
 * @return: none
 */
void app_cmd_move_hue_handler(MOVE_MODE_T move_mode, UCHAR_T move_rate);

/**
 * @description: Color saturation adjustment processing,use to resolve saturation move to, move, step command
  * @param {IN UCHAR_T saturation} target saturation
 * @param {IN UINT_T trans_time} delay time
 * @return: none
 */
void app_cmd_move_or_step_saturation_handler(UCHAR_T saturation, UINT_T trans_time);

/**
 * @description: Set hue and saturation at the same time
 * @param {IN UCHAR_T hue} target Hue
 * @param {IN UCHAR_T saturation} target saturation 
 * @param {IN UINT_T trans_time} Gradient time, unit 100ms.
 * @return: none
 */
void app_cmd_move_to_hue_and_saturation_handler(UCHAR_T hue, UCHAR_T saturation, UINT_T trans_time);


/**
 * @description: the gateway onoff cmd handler 
 * @param {UCHAR_T cmd} cmd
 * @param {attr_value_t payload} attribute value
 * @param {ZIGBEE_CMD_T cmd_type} type of cmd
 * @return: none
 */
OPERATE_RET color_control_cluster_handler(UCHAR_T cmd, attr_value_t *payload, ZIGBEE_CMD_T cmd_type);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif