/*
 * @file: app_common.h
 * @brief: 
 * @author: Arien
 * @date: 2021-10-13 20:00:00
 * @email: Arien.ye@tuya.com
 * @copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @company: http://www.tuya.com
 */
#ifndef __APP_COMMON_H__
#define __APP_COMMON_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "tuya_zigbee_modules.h"
#include "tuya_zigbee_sdk.h"
#include "cluster_tmp_measure.h"
#include "cluster_hum_measure.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TUYA_DEBUG

#ifdef TUYA_DEBUG
#define tuya_print(...) uart_printf(UART_ID_UART0, __VA_ARGS__)
#else
#define tuya_print(...)
#endif
 

#define POWER_UP_3S_SYN_EVT_ID              DEV_EVT_1
#define LED_ALARM_FLASH_EVT_ID              DEV_EVT_2
#define NWK_UP_EVT_ID                       DEV_EVT_3
#define NWK_START_ASSOCIATE_EVT_ID          DEV_EVT_4
#define SHT40_DALAY_READ_DATA_EVT_ID        DEV_EVT_5
#define THRESHOLD_READ_EVT_ID               DEV_EVT_6
#define CYCLER_READ_EVT_ID                  DEV_EVT_7
#define THRESHOLD_REPORT_EVT_ID             DEV_EVT_8
#define CYCLER_REPORT_EVT_ID                DEV_EVT_9
#define FAST_SAMPLE_EVT_ID                  DEV_EVT_10
#define LCD_REFRESH_EVT_ID                  DEV_EVT_11
#define LCD_NET_BLINK_EVT_ID                DEV_EVT_12

/* battery_param parameter config! */
#define BATTERY_REPORT_MINUTE           240
#define BATTERY_MAX_MILLIV              3000
#define BATTERY_MIN_MILLIV              2400

#define INVALID_TMR_ID                  0
#define LED_INDEX_0                     0
#define NORMAL_TX_POWER                 11
#define MAX_TX_POWER                    11

/* sensor parameter config */
#define TMP_THRESHOLD_VALUE             (0.6*10UL)      //The temperature threshold value is 0.6 C
#define HUM_THRESHOLD_VALUE             (0.06*100UL)    //The humidity threshold value is 6%

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

extern bool_t g_key_lock_flg;

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
 * @description: temperature and humidity sensor init
 * @param {type} none
 * @return: none
 */
void temperature_humidity_sensor_init(void);

/**
 * @description: temperature and humidity sensor report callback
 * @param: {in} ep: ep id
 * @param: {in} data: report data
 * @param: {in} mask: tmp or hum data mask
 * @return: none
 */
bool_t sensor_data_report_callback(uint8_t ep, sht40_data_t* data, uint8_t mask);

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
 * @description: battery sampling event callback
 * @param {in} data_st: args point
 * @return: none
 */
void battery_sampling_evt_callback(cb_args_t *data_st);




#ifdef __cplusplus
}
#endif

#endif