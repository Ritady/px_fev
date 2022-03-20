/*
 * @Author: xiaojia
 * @email: limu.xiao@tuya.com
 * @LastEditors:
 * @file name: dev_register.h
 * @Description: dev_register the device include file
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2021.10.09 14:35:20
 * @LastEditTime: 2021.10.09 20:06:32
 */

#ifndef __DEVICE_REGISTER_H__
#define __DEVICE_REGISTER_H__

#include "tuya_zigbee_sdk.h"
#include "tuya_zigbee_stack.h"
#include "mg21_remote_zll_callback.h"


#ifdef __cplusplus
extern "C" {
#endif

#define ZG_REPORTING_CONFIG \
    { ZCL_REPORTING_DIRECTION_REPORTED, 1, CLUSTER_BASIC_CLUSTER_ID, 0x0001, ZCL_CLUSTER_MASK_SERVER, 0x0000, 1, 65534, 0 }, \
    { ZCL_REPORTING_DIRECTION_REPORTED, 1, CLUSTER_POWER_CONFIG_CLUSTER_ID, 0x0021, ZCL_CLUSTER_MASK_SERVER, 0x0000, 14400, 65534, 10 }, \

//identify cluster
#define DEF_CLUSTER_IDENTIFY_CLUSTER_ID(a) \
    { CLUSTER_IDENTIFY_CLUSTER_ID, (attr_t *)&((a)[0]), get_array_len((a)) },

#define ZIG_DEV_DESC_MODEL(a, b) \
    {1, ZHA_PROFILE_ID, ZG_DEVICE_ID_DIMMER_SWITCH, get_array_len((a)), (cluster_t *)&((a)[0]), get_array_len((b)), (cluster_t *)&((b)[0])},
// #define ZIG_DEV_DESC_MODEL(a) \
//     {1, ZHA_PROFILE_ID, ZG_DEVICE_ID_DIMMER_SWITCH, 0, NULL, get_array_len((a)), (cluster_t *)&((a)[0])},   //only to register for the client side


#define EP_SUMS  1
#define BATTERY_REPORT_PERIOD                (4*3600*1000)  //(4*3600*1000ms  -> 4hours)
#define KEY_PUSH_TIME_FOR_ZIGBEE_JOIN_NEW    5000           //5s
#define ZIGBEE_JOIN_MAX_TIMEOUT              (30*1000)      //30s

#define ZLL_HEART_BEAT                       (4*3600*1000)



/*  OTA device poll config  */
#define POLL_SPEED_UP_MS                     250
#define POLL_NORMAL_SPEED_MS                 500

/* poll parameter config! */
#define POLL_FOREVER                         0
#define POLL_INTERVAL_MS                     500
#define WAIT_APP_ACK_MS                      1000
#define POLL_FAILE_MAX                       3

/* join parameter config! */
#define RESET_TIME_MS                        3000
#define BEACON_SEND_FOR_JOIN_MS              100
#define JOINED_CONTINUE_POLL_TIME_MS         25000
#define POWER_ON_JOIN                        0
#define RM_LEAVE_JOIN                        1
#define JOIN_TIME_OUT_MS                     30000

/* rejoin parameter config! */
#define BEACON_TIME                          3
#define BEACON_SEND_FOR_REJOIN_MS            300
#define JOIN_WAKUP_POLL_TIME_MS              30000
#define REJOIN_WAKUP_POLL_TIME_MS            10000
#define NEXT_REJOIN_TIME                     (4*60*60*1000)   // 4 hours (4*60*60*1000ms)

/**
 * @description: zigbee device init
 * @param {type} none
 * @return: none
 */
void dev_init(void);


#ifdef __cplusplus
}
#endif

#endif 