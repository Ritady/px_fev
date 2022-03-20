/**
* @file device_register.h
* @brief device Information Registration 
* @version 0.1
* @date 2021-09-03
*
* @copyright Copyright (c) tuya.inc 2019
* 
*/
#ifndef __DEVICE_REGISTER_H__
#define __DEVICE_REGISTER_H__
   
#include "tuya_zigbee_sdk.h"
#include "zigbee_dev_template.h"
#include "config.h"
#include "app_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * private attribute id
 */
#define LED_MODE_ATTRIBUTE_ID         0x8001

#define POWERON_STATUS_ATTRIBUTE_ID   0x8002

#define ZIGBEE_JOIN_MAX_TIMEOUT       300000   // 300s  

#define EP_1                          0x01
  
#define CH_SUMS                       0x01   
  
#define EP_GP                         0xF2  

extern const zg_dev_config_t g_zg_dev_config;

extern const join_config_t g_join_config;
  
extern const dev_description_t g_dev_des[];

extern void dev_zigbee_recovery_network_set(bool_t enable);

extern void sdk_config_multi_key_join(bool_t );

extern void dev_zigbee_enable_zigbee30_test_mode(bool_t );

/**
 * @description: zigbee device init
 * @param {type} none
 * @return: none
 */
void dev_zigbee_init(void);

#ifdef __cplusplus
}
#endif

#endif

















