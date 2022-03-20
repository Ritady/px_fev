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
//#include "cluster_onoff.h"

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
		
#define         KEY_2_PORT                      KEY1_PORT
#define         KEY_2_PIN                       KEY1_PIN
#define         KEY_2_MODE                      KEY1_MODE          
#define         KEY_2_OUT                       KEY1_OUT            
#define         KEY_2_DRIVER                    KEY1_DRIVER 
		
#define         KEY_3_PORT                      KEY2_PORT
#define         KEY_3_PIN                       KEY2_PIN
#define         KEY_3_MODE                      KEY2_MODE          
#define         KEY_3_OUT                       KEY2_OUT            
#define         KEY_3_DRIVER                    KEY2_DRIVER 

#define         KEY_4_PORT                      KEY3_PORT
#define         KEY_4_PIN                       KEY3_PIN
#define         KEY_4_MODE                      KEY3_MODE          
#define         KEY_4_OUT                       KEY3_OUT            
#define         KEY_4_DRIVER                    KEY3_DRIVER
		
#define         KEY_5_PORT                      KEY4_PORT
#define         KEY_5_PIN                       KEY4_PIN
#define         KEY_5_MODE                      KEY4_MODE          
#define         KEY_5_OUT                       KEY4_OUT            
#define         KEY_5_DRIVER                    KEY4_DRIVER
		
#define         KEY_6_PORT                      KEY5_PORT
#define         KEY_6_PIN                       KEY5_PIN
#define         KEY_6_MODE                      KEY5_MODE          
#define         KEY_6_OUT                       KEY5_OUT            
#define         KEY_6_DRIVER                    KEY5_DRIVER
		
#define         NET_LED_PORT                  	LED0_PORT
#define         NET_LED_PIN                   	LED0_PIN
#define         NET_LED_MODE                  	LED0_MODE
#define         NET_LED_DOUT                  	LED0_OUT
#define         NET_LED_DRIVE                 	LED0_DRIVER

/**
 * the index of keys, indicators and relays in the input or output table
 */
#define 		KEY_1_IO_INDEX              	0
#define 		KEY_2_IO_INDEX              	1
#define 		KEY_3_IO_INDEX              	2
#define 		KEY_4_IO_INDEX              	3
#define 		KEY_5_IO_INDEX              	4
#define 		KEY_6_IO_INDEX              	5

#define 		NET_LED_IO_INDEX          	    0
    
    
#define ZS3L_USART_CONFIG_USER {\
    UART_ID_UART0,\
    UART_PIN_TYPE_CONFIG,\
    {PORT_D, PIN_0},\
    {PORT_D, PIN_1},\
    LOC_29, \
    LOC_25, \
    115200,\
    USART_PARITY_NONE,\
    USART_STOPBITS_ONE,\
    USART_DATABITS_8BIT,\
    NULL\
}

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
    REPORT_ZCL_ID_RESPONSE_SCENE_ADD = 100,
    REPORT_ZCL_ID_PRIVATE_CMD,
}ZCL_ID_T;

/**
 * enumerate zigbee device work mode
 */
typedef enum{
    DEV_WORK_ST_NORMAL = 0,
    DEV_WORK_ST_TEST
} DEV_WORK_ST_T;

/**
 * @description: gpio init
 * @param: none
 * @return: none
 */
void gpio_init(void);

/**
 * @description: mf test uart register, aotomatic generated, not edit
 * @param {void} none
 * @return: user_uart_config_t mf uart config information
 */
user_uart_config_t* mf_test_uart_config(void);

/**
 * @description: key processing 
 * @param {key_id} the key_id of key
 * @param {key_st} the trigger state of key
 * @param {push_time} press time of key
 * @return: none
 */
void dev_key_handle(uint32_t key_id, key_st_t key_st, uint32_t push_time);

extern bool_t dev_scene_recall_send_command_broadcast(uint8_t endpoint, uint16_t groupId, uint8_t sceneId);

#ifdef __cplusplus
}
#endif

#endif
