/*************************************************************************************/
/* Automatically-generated file. Do not edit! */
/*************************************************************************************/
#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "tuya_zigbee_sdk.h"
/* firmware information! */
#define FIRMWARE_NAME                   "tmp_hum_lcd_sensor"
#define FIRMWARE_VER                    0x40
#define PRODUCTOR_ID_PFEFIX             "_TZ3000_"
#define IC                              "EFR32MG21A020F768"
#define MANUFACTURER_CODE               0x1002
#define OTA_IMAGE_TYPE                  0x1602
#define MODEL_ID                        "TS0201"
#define PRODUCTOR_ID                    "gjll29eb"
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
#define LED_NUM                         1
#define LED0_PORT                       PORT_C
#define LED0_PIN                        PIN_0
#define LED0_MODE                       GPIO_MODE_OUTPUT_PP
#define LED0_OUT                        GPIO_DOUT_HIGH
#define LED0_DRIVER                     GPIO_LEVEL_LOW



/* io config! */
/* key config! */
#define KEY_NUM                         1
#define KEY0_PORT                       PORT_A
#define KEY0_PIN                        PIN_0
#define KEY0_MODE                       GPIO_MODE_INPUT_PULL
#define KEY0_OUT                        GPIO_DOUT_HIGH
#define KEY0_DRIVER                     GPIO_LEVEL_LOW


/* io list config! */
#define LEDS_IO_LIST \
    {LED0_PORT, LED0_PIN, LED0_MODE, LED0_OUT, LED0_DRIVER, }, \

/* io list config! */
#define KEYS_IO_LIST \
    {KEY0_PORT, KEY0_PIN, KEY0_MODE, KEY0_OUT, KEY0_DRIVER, }, \

#endif
