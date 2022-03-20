/**
* @file cluster_onoff.h
* @brief about tuya_common cluster
* @version 0.1
* @date 2021-09-03
*
* @copyright Copyright (c) tuya.inc 2019
* 
*/
#ifndef __CLUSTER_ONOFF_H__
#define __CLUSTER_ONOFF_H__

#include "tuya_zigbee_sdk.h"
#include "zigbee_dev_template.h"
#include "device_register.h"
#include "app_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CMD_KEY_INDEX_COMMAND_ID        0xFD

/**
 * @description: report onoff attributes 
 * @param {endpoint} the endpoint of the write attribute
 * @param {qs} permit loss packages or not
 * @return: none
 */
void send_onoff_reversed(uint8_t endpoint, SEND_QOS_T qs);

/**
 * @description: Data about ONOFF cluster was received
 * @param dev_msg
 * @return: none
 */
void cluster_onoff_handler(dev_msg_t *dev_msg);

#ifdef __cplusplus
}
#endif

#endif