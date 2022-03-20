/*
 * @Author: xiaojia
 * @email: limu.xiao@tuya.com
 * @LastEditors:
 * @file name: onoff_cluster.h
 * @Description: onoff cluster attribute report and gateway cmd handler include file
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2021.8.30 16:49:30
 * @LastEditTime: 2021.8.31 10:59:01
 */

#ifndef  __ONOFF_CLUSTER_H__
#define  __ONOFF_CLUSTER_H__

#include  "tuya_zigbee_stack.h"
#include  "tuya_zigbee_sdk.h"
#include  "light_control.h"
#include  "light_types.h"


#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */


#define  ZCL_ID_ONOFF             100  //ZCL ID
#define  ZCL_ID_COUNT_DOWN        104


/***********************************************************************************************************
 * 
 *          device repoet their attribute to the gateway
 * 
 **********************************************************************************************************/

/**
 * @description: report onoff attribute value
 * @param {IN SEND_QOS_T Qos} QOS value
 * @param {IN USHORT_T delay_ms} delay report
 * @param {IN bool_t onoff} on / off
 * @return: none
 */
void report_onoff_value(SEND_QOS_T Qos, UINT_T delay_ms, bool_t onoff);

/**
 * @description: report count down attribute value
 * @param {IN SEND_QOS_T Qos} QOS value
 * @param {IN USHORT_T delay_ms} delay report
 * @param {IN UINT_T count_down} count down value
 * @return: none
 */
void report_count_down_data(SEND_QOS_T Qos, UINT_T delay_ms, UINT_T count_down);



/***********************************************************************************************************
 * 
 *          gateway send cmd to device, device handler the cmd functions
 * 
 **********************************************************************************************************/

/**
 * @note app on/off command handler
 * @param [in]{UINT_T} onoff
 * @param [in]{ZIGBEE_CMD_T} single or group transmit cmd
 * @return: none
 */
void app_cmd_set_onoff_handler(ZIGBEE_CMD_T cmd_type,  UINT_T onoff);


/**
 * @description: app command set count down handler
 * @param {IN UINT_T count_down} seconds
 * @return: none
 */
void app_cmd_set_count_down_handler(UINT_T count_down);

/**
 * @description: the gateway onoff cmd handler 
 * @param {UCHAR_T cmd} cmd
 * @param {attr_value_t payload} attribute value
 * @param {ZIGBEE_CMD_T cmd_type} type of cmd
 * @return: none
 */
OPERATE_RET onoff_cluster_handler(UCHAR_T cmd, attr_value_t *payload, ZIGBEE_CMD_T cmd_type);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif