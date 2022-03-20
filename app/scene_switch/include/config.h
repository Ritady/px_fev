/*************************************************************************************/
/* Automatically-generated file. Do not edit! */
/*************************************************************************************/
#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "tuya_zigbee_sdk.h"
/* firmware information! */
#define FIRMWARE_INFO                   "scene_switch_1.0.0"
#define FIRMWARE_NAME                   "scene_switch"
#define FIRMWARE_VER                    0x40
#define PRODUCTOR_ID_PFEFIX             "_TZ3002_"
#define IC                              "EFR32MG21A020F768"
#define OTA_IMAGE_TYPE                  0x1602
#define MODULE_NAME                     ZS3L
#define MODEL_ID                        "TS0026"
#define PRODUCTOR_ID                    "zicffvsr"


/* uart config! */
#define UART_ENABLE                     true
#define UART_NUM                        0x2

#define UART0_RX_PORT                   PORT_A
#define UART0_RX_PIN                    PIN_1
#define UART0_RX_LOC                    LOC_0
#define UART0_TX_PORT                   PORT_A
#define UART0_TX_PIN                    PIN_0
#define UART0_TX_LOC                    LOC_0
#define UART0_BANDRATE                  115200

#define UART1_RX_PORT                   PORT_A
#define UART1_RX_PIN                    PIN_2
#define UART1_RX_LOC                    LOC_0
#define UART1_TX_PORT                   PORT_A
#define UART1_TX_PIN                    PIN_3
#define UART1_TX_LOC                    LOC_0
#define UART1_BANDRATE                  115200



/* io config! */
/* led config! */
#define LED0_PORT                       PORT_B
#define LED0_PIN                        PIN_1
#define LED0_MODE                       GPIO_MODE_OUTPUT_PP
#define LED0_OUT                        GPIO_DOUT_HIGH
#define LED0_DRIVER                     GPIO_LEVEL_LOW



/* io config! */
/* key config! */
#define KEY0_PORT                       PORT_C
#define KEY0_PIN                        PIN_1
#define KEY0_MODE                       GPIO_MODE_INPUT_PULL
#define KEY0_OUT                        GPIO_DOUT_HIGH
#define KEY0_DRIVER                     GPIO_LEVEL_LOW

#define KEY1_PORT                       PORT_C
#define KEY1_PIN                        PIN_0
#define KEY1_MODE                       GPIO_MODE_INPUT_PULL
#define KEY1_OUT                        GPIO_DOUT_HIGH
#define KEY1_DRIVER                     GPIO_LEVEL_LOW

#define KEY2_PORT                       PORT_A
#define KEY2_PIN                        PIN_0
#define KEY2_MODE                       GPIO_MODE_INPUT_PULL
#define KEY2_OUT                        GPIO_DOUT_HIGH
#define KEY2_DRIVER                     GPIO_LEVEL_LOW

#define KEY3_PORT                       PORT_A
#define KEY3_PIN                        PIN_3
#define KEY3_MODE                       GPIO_MODE_INPUT_PULL
#define KEY3_OUT                        GPIO_DOUT_HIGH
#define KEY3_DRIVER                     GPIO_LEVEL_LOW

#define KEY4_PORT                       PORT_A
#define KEY4_PIN                        PIN_4
#define KEY4_MODE                       GPIO_MODE_INPUT_PULL
#define KEY4_OUT                        GPIO_DOUT_HIGH
#define KEY4_DRIVER                     GPIO_LEVEL_LOW

#define KEY5_PORT                       PORT_A
#define KEY5_PIN                        PIN_6
#define KEY5_MODE                       GPIO_MODE_INPUT_PULL
#define KEY5_OUT                        GPIO_DOUT_HIGH
#define KEY5_DRIVER                     GPIO_LEVEL_LOW


#endif
