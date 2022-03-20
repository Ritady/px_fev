/**
* @file app_common.h
* @brief 
* @version 1.0
* @date 2021-08-31
*
* @copyright Copyright (c) tuya.inc 2019
* 
*/
#ifndef __APP_COMMON_H__
#define __APP_COMMON_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "tuya_zigbee_modules.h"
#include "tuya_zigbee_sdk.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TUYA_DEBUG

#ifdef TUYA_DEBUG
#define tuya_print(...) uart_printf(UART_ID_UART0, __VA_ARGS__)
#else
#define tuya_print(...)
#endif
 
#define LED_POWER_UP_SYN_EVT_ID             DEV_EVT_1
#define LED_ALARM1_FLASH_EVT_ID             DEV_EVT_2
#define NWK_UP_EVT_ID                       DEV_EVT_3
#define NWK_START_ASSOCIATE_EVT_ID          DEV_EVT_4
#define LED_TAMPER_FLASH_EVT_ID             DEV_EVT_5
#define QUICKEN_ARRIVE_LOSE_STATUS_EVT_ID   DEV_EVT_6
#define GPIO_TAMPER_DEBOUNCE_EVT_ID         DEV_EVT_7
#define GPIO_ALARM1_DEBOUNCE_EVT_ID         DEV_EVT_8

#define ALARM1_ATTR_BIT                     0
#define ALARM2_ATTR_BIT                     1
#define TAMPER_ATTR_BIT                     2

/* battery_param parameter config! */
#define BATTERY_REPORT_MINUTE           240
#define BATTERY_MAX_MILLIV              3000
#define BATTERY_MIN_MILLIV              2400
  
#define USER_USART_CONFIG_DEFAULT {\
    UART_ID_UART0,\
    UART_PIN_TYPE_CONFIG,\
    {UART0_TX_PORT, UART0_TX_PIN},\
    {UART0_RX_PORT, UART0_RX_PIN},\
    UART0_TX_LOC, \
    UART0_RX_LOC, \
    115200,\
    USART_PARITY_NONE,\
    USART_STOPBITS_ONE,\
    USART_DATABITS_8BIT,\
    NULL\
}


// key status union
typedef union {
  uint8_t key_bits_flg;
   struct{
    uint8_t key_short_bit   :1;
    uint8_t key_long_bit    :1;
  }key_bits;
}key_st;

// power type enum
typedef enum {
    battery_source = 3,
    DC_source = 4
}power_type_t;

typedef void (*func_type)(GPIO_PORT_T, GPIO_PIN_T);

typedef struct {
    DEV_IO_ST_T init_status;
    DEV_IO_ST_T new_status;
    DEV_IO_ST_T last_status;
    DEV_IO_ST_T gpio_polarity;
    gpio_config_t sensor_io_st;
    func_type sensor_func;
} sensor_t;

typedef enum {
    GPIO_SENSOR_INACTIVE = 0x00,
    GPIO_SENSOR_ACTIVE   = 0x01,
} GPIO_STATE_T;


extern bool_t g_key_lock_flg;
extern sensor_t alarm_info;
extern sensor_t tamper_info;

/**
 * @description: uart init func
 * @param {type} none
 * @return: none
 */
void uart_init(void);

/**
 * @description: mf test uart register, aotomatic generated, not edit
 * @param {void} none
 * @return: user_uart_config_t mf uart config information
 */
user_uart_config_t* mf_test_uart_config(void);

/**
 * @description: device peripherals init
 * @param {type} none
 * @return: none
 */
void per_dev_init(void);

/**
 * @description: ias zone sensor init
 * @param {type} none
 * @return: none
 */
void ias_sensor_init(void);

/**
 * @description: sensor pin sample debounce handler
 * @param {type} none
 * @return: none
 */
void gpio_sensor_debounce_handler(uint8_t evt);

/**
 * @description: Key event handler
 * @param: {in} key_id: key id
 * @param: {in} key_st: key status
 * @param: {in} push_time: key push time
 * @return: none
 */
void keys_evt_handler(uint32_t key_id, key_st_t key_st, uint32_t push_time);

/**
 * @description: report battery voltage
 * @param {in} ep: endpoind
 * @return: none
 */
void zig_report_battery_voltage(uint8_t ep);

/**
 * @description: event handler
 * @param: {in} evt: event id
 * @return: none
 */
void dev_evt_callback(uint8_t evt);

/**
 * @description: power sampler init
 * @param {type} none
 * @return: none
 */
void power_sampler_init(void);

/**
 * @description: write battery type property function
 * @param {in} type: power source type
 * @return: none
 */
void dev_config_power_source_type(power_type_t type);

/**
 * @description: sensor init gpio set
 * @param {type} none
 * @return: none
 */
uint8_t sensor_init_io_status_set(sensor_t *senor_info);

/**
 * @description: battery sampling event callback
 * @param {in} data_st: args point
 * @return: none
 */
void battery_sampling_evt_callback(cb_args_t *data_st);




#ifdef __cplusplus
}
#endif

#endif