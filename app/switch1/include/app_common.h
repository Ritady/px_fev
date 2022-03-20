/**
* @file app_common.h
* @brief some peripheral
* @version 0.1
* @date 2021-09-03
*
* @copyright Copyright (c) tuya.inc 2019
* 
*/
#ifndef __APP_COMMON_H__
#define __APP_COMMON_H__

#include "zigbee_dev_template.h"
#include "config.h"
#include "device_register.h"
#include "onoff_cluster.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  keys, indicators and relays configuration information
 */
#define         KEY_1_PORT                      KEY0_PORT
#define         KEY_1_PIN                       KEY0_PIN
#define         KEY_1_MODE                      KEY0_MODE          
#define         KEY_1_OUT                       KEY0_OUT            
#define         KEY_1_DRIVER                    KEY0_DRIVER         

#define         LED_1_PORT                      LED0_PORT    
#define         LED_1_PIN                       LED0_PIN
#define         LED_1_MODE                      LED0_MODE
#define         LED_1_DOUT                      LED0_OUT
#define         LED_1_DRIVE                     LED0_DRIVER

#define         NET_LED_2_PORT                  LED1_PORT
#define         NET_LED_2_PIN                   LED1_PIN
#define         NET_LED_2_MODE                  LED1_MODE
#define         NET_LED_2_DOUT                  LED1_OUT
#define         NET_LED_2_DRIVE                 LED1_DRIVER

#define         RELAY_1_PORT                    LED2_PORT
#define         RELAY_1_PIN                     LED2_PIN
#define         RELAY_1_MODE                    LED2_MODE
#define         RELAY_1_DOUT                    LED2_OUT
#define         RELAY_1_DRIVE                   LED2_DRIVER

/**
 * the index of keys, indicators and relays in the input or output table
 */
#define 		KEY_1_IO_INDEX              	0
#define 		LED_1_IO_INDEX              	0
#define 		NET_LED_1_IO_INDEX          	1
#define 		RELAY_1_IO_INDEX            	2
       
/**
 * sequence of events
 */
#define 		EVT_JOIN_NETWORK   				DEV_EVT_1 
#define 		EVT_LED_BLINK      				DEV_EVT_2 
#define 		LOCAL_ONOFF_EVT    				DEV_EVT_3 
#define 		JOIN_OK_DELAY_EVT  				DEV_EVT_4
#define 		IDENTIFY_EVT       				DEV_EVT_5

/**
 * print enable
 */
#define PRINT_ENABLE       1 
#if (PRINT_ENABLE)
#define tuya_print    app_print
#else
#define tuya_print   
#endif

/**
 * enumerate zcl_id
 */
typedef enum{
    REPORT_ZCL_ID_ONOFF = 100,
    REPORT_ZCL_ID_POWERON_STATUS,
    REPORT_ZCL_ID_LED_MODE,
    REPORT_ZCL_ID_ONOFF_TIME
}ZCL_ID_T;


/**
 * enumerate zigbee device work mode
 */
typedef enum{
    DEV_WORK_ST_NORMAL = 0,
    DEV_WORK_ST_TEST
} DEV_WORK_ST_T;

extern const gpio_config_t gpio_input_config[];

extern const gpio_config_t gpio_ouput_config[];

/**
 * @description: gpio init
 * @param: none
 * @return: none
 */
void gpio_init(void);

/**
 * @description: operate led and relay io
 * @param {endpoint} the endpoint of the write attribute
 * @param {st} on or off
 * @return: none
 */
void dev_switch_op(uint8_t ep, DEV_IO_ST_T st);

extern void dev_key_handle(uint32_t key_id, key_st_t key_st, uint32_t push_time);

extern void identify_handler(ext_plugin_args_t *args);

#ifdef __cplusplus
}
#endif

#endif
