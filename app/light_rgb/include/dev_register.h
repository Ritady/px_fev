/*
 * @Author: xiaojia
 * @email: limu.xiao@tuya.com
 * @LastEditors:
 * @file name: dev_register.h
 * @Description: dev_register the device include file
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2021.08.30 14:35:20
 * @LastEditTime: 2021.09.01 20:06:32
 */

#ifndef __REGISTER_H__
#define __REGISTER_H__

#include "tuya_zigbee_sdk.h"
#include "zigbee_dev_template.h"
#include "light_types.h"
#include "hal_uart.h"
#include "app_common.h"
#include "tuya_zigbee_modules.h"


#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */


#define MODULE_TABLE_LEN   get_array_len(module_tabel)

#define EP_SUMS sizeof(g_dev_des)/sizeof(g_dev_des[0])
#define BEACON_SEND_INTERVAL_200MS   200  //200ms 

/*********************************************************************************************************************************
*                                       私有指令
*********************************************************************************************************************************/
//on-off
 #define TY_CMD_SET_COUNT_DOWN_CMD_ID           0xF0 //count down---on/off cluster

//level
#define TY_CMD_SET_COLOR_MODE_CMD_ID            0xF0    //change mode cmd: white/color/scene/music

//COLOR CONTROL
#define TY_CMD_SET_COLOR_HSV_CMD_ID             0xE1    //Set HSV, H:360, S:1000, V:1000,if current mode is not color mode should switch to color mode
#define TY_CMD_SET_COLOR_MODE_CMD_ID            0xF0    //change mode cmd: white/color/scene/music
#define TY_CMD_SET_COLOR_REALTIME_DATA_CMD_ID   0xF2    //real time ctrl cmd: mode(1) + H(2) + S(2) + V(2) + B(2) + T(2) = 11 bytes


/*********************************************************************************************************************************
*                                              私有属性
*********************************************************************************************************************************/
/*Onoff Cluster*/
#define TY_ATTR_COUNT_DOWN_ATTR_ID                  0xF000  //report current count data,uint：1s

/*Level Cluster*/
#define TY_ATTR_LIGHT_V_VALUE_ATTRIBUTE_ID          0xF001  //color V attribute (1 byte)

//COLOR CONTROL
// #define TY_ATTR_COLOR_TEMPERATURE_ATTR_ID           0xE000  //light mode ：1 byte

#define TY_ATTR_COLOR_HSV_ATTRIBUTE_ID              0xE100  //light color hsv：6 bytes

#define TY_ATTR_LIGHT_MODE_ATTRIBUTE_ID             0xF000  //light mode attribute (1 byte)
#define TY_ATTR_LIGHT_APP_DATA_ATTRIBUTE_ID         0xF102  //all app data attribute: onoff(1) + mode(1) + H(2) + S(2) + V(2) + B(2) + T(2)= 12 Bytes
#define TY_ATTR_LIGHT_SCENE_DATA_ATTRIBUTE_ID       0xF003  //scene data attribute (2+8*n, n<=8) Bytes
#define TY_ATTR_LIGHT_SCENE_GROUP_ID_ATTRIBUTE_ID   0xF004  //scene group id

#define OPTION_ATTRIBUTE_ID                         0x000F




void dev_zigbee_init(void);

OPERATE_RET dev_gpio_module_init(PCHAR_T name, UCHAR_T len);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif