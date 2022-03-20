/**
* @file cluster_tuya_common.h
* @brief about tuya_common cluster
* @version 0.1
* @date 2021-09-03
*
* @copyright Copyright (c) tuya.inc 2019
* 
*/
#ifndef __CLUSTER_TUYA_COMMON_H__
#define __CLUSTER_TUYA_COMMON_H__

#include "tuya_zigbee_sdk.h"
#include "zigbee_dev_template.h"
#include "device_register.h"
#include "app_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CMD_RECEIVE_GROUP_ID_SCENE_ID_COMMAND_ID  0xE1

#define ATTR_SAVE_GROUP_ID_ATTRIBUTE_ID           0xD004
#define ATTR_SAVE_SCENE_ID_ATTRIBUTE_ID           0xD005

#define CUSTOM_GROUP_SCENE_ATTR_ID                0x0000
#define CUSTOM_GROUP_SCENE_ATTR_ID_MODE           0x0001

/**
 * @description: report onoff attributes 
 * @param {endpoint} the endpoint of the write attribute
 * @param {qs} permit loss packages or not
 * @return: none
 */
void cluster_tuya_common_handler(dev_msg_t *dev_msg);

#ifdef __cplusplus
}
#endif

#endif