/*************************************************************************************/
/* Automatically-generated file. Do not edit! */
/*************************************************************************************/
#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "tuya_zigbee_sdk.h"
/* firmware information! */
#define FIRMWARE_INFO                   "light_rgb_demo_1.0.0"
#define FIRMWARE_NAME                   "light_rgb_demo"
#define FIRMWARE_VER                    0x40
#define PRODUCTOR_ID_PFEFIX             "_TZ3210_"
#define IC                              "EFR32MG21A020F768"
#define OTA_IMAGE_TYPE                  0x1602
#define MODULE_NAME                     ZS3L
#define MODEL_ID                        "TS0503B"
#define PRODUCTOR_ID                    "0heakj88"


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
#define LED0_OUT                        GPIO_DOUT_LOW
#define LED0_DRIVER                     GPIO_DOUT_HIGH

#define LED1_PORT                       PORT_A
#define LED1_PIN                        PIN_0
#define LED1_MODE                       GPIO_MODE_OUTPUT_PP
#define LED1_OUT                        GPIO_DOUT_HIGH
#define LED1_DRIVER                     GPIO_DOUT_LOW

#define LED2_PORT                       PORT_A
#define LED2_PIN                        PIN_4
#define LED2_MODE                       GPIO_MODE_OUTPUT_PP
#define LED2_OUT                        GPIO_DOUT_HIGH
#define LED2_DRIVER                     GPIO_DOUT_LOW


#endif
