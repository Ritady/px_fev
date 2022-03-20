/*************************************************************************************/
/* Automatically-generated file. Do not edit! */
/*************************************************************************************/
#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "tuya_zigbee_sdk.h"
/* firmware information! */
#define FIRMWARE_NAME                   "simple_contact_sensor"
#define FIRMWARE_VER                    0x40
#define PRODUCTOR_ID_PFEFIX             "_TZ3000_"
#define IC                              "EFR32MG21A020F768"
#define MANUFACTURER_CODE               0x1002
#define OTA_IMAGE_TYPE                  0x1602
#define MODEL_ID                        "TS0203"
#define PRODUCTOR_ID                    "alo8ncai"
#define MODULE_NAME                     ZS3L


/* uart config! */
#define UART_ENABLE                     true
#define UART_NUM                        0x1

#define UART0_RX_PORT                   PORT_A
#define UART0_RX_PIN                    PIN_6
#define UART0_RX_LOC                    LOC_0
#define UART0_TX_PORT                   PORT_A
#define UART0_TX_PIN                    PIN_5
#define UART0_TX_LOC                    LOC_0
#define UART0_BANDRATE                  115200



/* io config! */
/* led config! */
#define LED_NUM                         3
#define LED0_PORT                       PORT_B
#define LED0_PIN                        PIN_1
#define LED0_MODE                       GPIO_MODE_OUTPUT_PP
#define LED0_OUT                        GPIO_DOUT_HIGH
#define LED0_DRIVER                     GPIO_LEVEL_LOW

#define LED1_PORT                       PORT_C
#define LED1_PIN                        PIN_2
#define LED1_MODE                       GPIO_MODE_OUTPUT_PP
#define LED1_OUT                        GPIO_DOUT_HIGH
#define LED1_DRIVER                     GPIO_LEVEL_LOW

#define LED2_PORT                       PORT_D
#define LED2_PIN                        PIN_0
#define LED2_MODE                       GPIO_MODE_OUTPUT_PP
#define LED2_OUT                        GPIO_DOUT_HIGH
#define LED2_DRIVER                     GPIO_LEVEL_LOW



/* io config! */
/* key config! */
#define KEY_NUM                         1
#define KEY0_PORT                       PORT_B
#define KEY0_PIN                        PIN_0
#define KEY0_MODE                       GPIO_MODE_INPUT_PULL
#define KEY0_OUT                        GPIO_DOUT_HIGH
#define KEY0_DRIVER                     GPIO_LEVEL_LOW



/* io config! */
/* sensor config! */
#define SENSOR_NUM                      1
#define ALARM1_PORT                     PORT_A
#define ALARM1_PIN                      PIN_3
#define ALARM1_MODE                     GPIO_MODE_INPUT_HIGH_IMPEDANCE
#define ALARM1_OUT                      GPIO_DOUT_LOW
#define ALARM1_DRIVER                   GPIO_LEVEL_ALL



/* io config! */
/* tamper config! */
#define TAMPER_NUM                      1
#define TAMPER_PORT                     PORT_A
#define TAMPER_PIN                      PIN_4
#define TAMPER_MODE                     GPIO_MODE_INPUT_HIGH_IMPEDANCE
#define TAMPER_OUT                      GPIO_DOUT_LOW
#define TAMPER_DRIVER                   GPIO_LEVEL_ALL


/* io list config! */
#define LEDS_IO_LIST \
    {LED0_PORT, LED0_PIN, LED0_MODE, LED0_OUT, LED0_DRIVER, }, \
    {LED1_PORT, LED1_PIN, LED1_MODE, LED1_OUT, LED1_DRIVER, }, \
    {LED2_PORT, LED2_PIN, LED2_MODE, LED2_OUT, LED2_DRIVER, }, \

/* io list config! */
#define KEYS_IO_LIST \
    {KEY0_PORT, KEY0_PIN, KEY0_MODE, KEY0_OUT, KEY0_DRIVER, }, \

/* io list config! */
#define SENSORS_IO_LIST \
    {ALARM1_PORT, ALARM1_PIN, ALARM1_MODE, ALARM1_OUT, ALARM1_DRIVER, }, \

/* io list config! */
#define TAMPER_IO_LIST \
    {TAMPER_PORT, TAMPER_PIN, TAMPER_MODE, TAMPER_OUT, TAMPER_DRIVER, }, \

#endif
