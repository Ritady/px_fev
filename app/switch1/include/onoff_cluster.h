/**
* @file onoff_cluster.h
* @brief about onoff cluster
* @version 0.1
* @date 2021-09-03
*
* @copyright Copyright (c) tuya.inc 2019
* 
*/
#ifndef __ONOFF_CLUSTER_H__
#define __ONOFF_CLUSTER_H__

#include "tuya_zigbee_sdk.h"
#include "zigbee_dev_template.h"
#include "device_register.h"
#include "app_common.h"

#ifdef __cplusplus
extern "C" {
#endif
   
/**
 * enumerate led mode
 */
typedef enum{
    LED_MODE_NONE = 0,
    LED_MODE_RELAY,
    LED_MODE_POS
}LED_MODE_T;

/**
 * enumerate poweron status
 */
typedef enum{
    POWERON_STATUS_OFF = 0,
    POWERON_STATUS_ON,
    POWERON_STATUS_MEMORY
}POWERON_STATUS_T;
   
typedef struct
{
    uint8_t channels; 
    uint8_t timer; // timer id
}ONOFF_TIME_T;

typedef struct
{
    uint8_t start_timer_fg;
    uint16_t ontime_t;
    uint16_t offtime_t;
}ONOFF_TIME_CTL_T;


extern ONOFF_TIME_T g_on_off_time;


extern uint8_t g_relay_onoff_status[];

/**
 * @description: on off time report
 * @param {channel} channel
 * @param ontime
 * @param offwaittime
 * @return: none
 */
void user_on_off_time_rpt(uint8_t channel, uint16_t ontime_t, uint16_t offtime_t);

/**
 * @description: on off time stop
 * @param {channel} channel
 * @return: none
 */
void user_on_off_time_stop(uint8_t channel);

/**
 * @description: on off time message receive  
 * @param {channel} channel
 * @return: none
 */
void user_on_off_time_rev_msg(uint8_t channel, uint16_t onTime, uint16_t offWaitTime);

/**
 * @description: report onoff attributes 
 * @param {endpoint} the endpoint of the write attribute
 * @param {qs} permit loss packages or not
 * @return: none
 */
void dev_report_onoff_msg(uint8_t ep, SEND_QOS_T qs);

/**
 * @description: report led mode attributes 
 * @param {endpoint} the endpoint of the write attribute
 * @param {led_mode} led mode
 * @param {qs} permit loss packages or not
 * @return: none
 */
void dev_report_led_mode(uint8_t endpoint, uint8_t led_mode, SEND_QOS_T qs);

/**
 * @description: report attributes 
 * @param {endpoint} the endpoint of the write attribute
 * @param {led_mode} poweron status
 * @param {qs} permit loss packages or not
 * @return: none
 */
void dev_report_poweron_status(uint8_t endpoint, uint8_t poweron_status, SEND_QOS_T qs);

/**
 * @description: Data about ONOFF cluster was received
 * @param dev_msg
 * @return: none
 */
void onoff_cluster_handler(dev_msg_t *dev_msg);

/**
 * @description: Received data about the onoff cluster "write attribute"
 * @param endpoint
 * @param attr id
 * @return: none
 */
void onoff_msg_write_attr_handler(uint8_t endpoint, uint16_t attr_id);

/**
 * @description: load attributes, operation indicators and relays
 * @param {void} 
 * @return: none
 */
void dev_status_load(void);

#ifdef __cplusplus
}
#endif

#endif
