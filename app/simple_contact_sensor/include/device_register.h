/**
* @file device_register.h
* @brief device_register module is used to device_register relevant
* @version 1.0
* @date 2021-08-31
*
* @copyright Copyright (c) tuya.inc 2019
* 
*/
#ifndef __DEVICE_REGISTER_H__
#define __DEVICE_REGISTER_H__

#include "config.h"


#ifdef __cplusplus
extern "C" {
#endif

#define IAS_ZONE_ATTR_LIST1 \
    {0x0000, ATTR_ENUM8_ATTRIBUTE_TYPE, 1, (0x00), {(uint8_t*)0x00}}, /* zone state */\
    {0x0001, ATTR_ENUM16_ATTRIBUTE_TYPE, 2, (ATTR_MASK_TOKENIZE), {(uint8_t*)0x0028}}, /* zone type */\
    {0x0002, ATTR_BITMAP16_ATTRIBUTE_TYPE, 2, (ATTR_MASK_TOKENIZE), { (uint8_t*)0x0000}}, /* zone status */\
    {0x0010, ATTR_IEEE_ADDRESS_ATTRIBUTE_TYPE, 8, (ATTR_MASK_TOKENIZE | ATTR_MASK_WRITABLE), {NULL}}, /* IAS CIE address */\
    {0x0011, ATTR_INT8U_ATTRIBUTE_TYPE, 1, (ATTR_MASK_TOKENIZE), {(uint8_t*)0xff}}, /* Zone ID */\
    {0xFFFD, ATTR_INT16U_ATTRIBUTE_TYPE, 2, (0x00), {(uint8_t*)0x0001}}, /* cluster revision */\

#define POWER_ATTR_LIST1 \
    {0x0000, ATTR_INT16U_ATTRIBUTE_TYPE, 2, (ATTR_MASK_SINGLETON), (uint8_t*)0x0000UL}, /* mains voltage */\
    {0x0020, ATTR_INT8U_ATTRIBUTE_TYPE, 1, (ATTR_MASK_SINGLETON), (uint8_t*)0x00UL}, /* battery voltage */\
    {0x0021, ATTR_INT8U_ATTRIBUTE_TYPE, 1, (ATTR_MASK_SINGLETON), (uint8_t*)0x00} , /* battery percentage remaining */\
    {0xFFFD, ATTR_INT16U_ATTRIBUTE_TYPE, 2, (ATTR_MASK_SINGLETON), (uint8_t*)0x0001}, /* cluster revision */\

#define DEF_CLUSTER_POWER_CLUSTER_ID1(a) \
    {CLUSTER_POWER_CONFIG_CLUSTER_ID, (attr_t *)&((a)[0]), get_array_len((a))},

#define DEF_CLUSTER_IAS_ZONE_CLUSTER_ID1(a) \
    {CLUSTER_IAS_ZONE_CLUSTER_ID, (attr_t *)&((a)[0]), get_array_len((a))},
             
#define ZIG_DEV_DESC_MODEL(a) \
    {1, ZHA_PROFILE_ID, ZG_DEVICE_ID_IAS_ZONE, get_array_len((a)), (cluster_t *)&((a)[0]), 0, NULL},

/* ledinfo parameter config! */
#define POWER_UP_LED_ON_TIME            3000
#define NWK_JOIN_LED_BLINK_FREQ         250

/* join parameter config! */
#define RESET_TIME_MS                   3000
#define CHANNEL_SW_PER_MS               300
#define JOINED_CONTINUE_POLL_TIME_MS    25000
#define POWER_ON_JOIN                   0
#define RM_LEAVE_JOIN                   1
#define JOIN_TIME_OUT_MS                30000

/* rejoin parameter config! */
#define POWER_ON_REJOIN                 1
#define AUTO_REJOIN_POLL                1
#define REJOINED_CONTINUE_POLL_TIME_MS  4000
#define BEACON_PER_MS                   1000
#define BEACON_TIME                     4
#define NEXT_REJOIN_PER_HOUR            4

/* poll parameter config! */
#define POLL_FOREVER                    0
#define POLL_INTERVAL_MS                1000
#define WAIT_APP_ACK_MS                 4000
#define POLL_MISS_MAX                   6
    
/* epinfo parameter config! */
#define DEV_EP_SUM                      1

/**
 * @description: zigbee device init
 * @param {type} none
 * @return: none
 */
void zg_dev_init(void);




#ifdef __cplusplus
}
#endif
#endif // !__DEVICE_REGISTER_H__