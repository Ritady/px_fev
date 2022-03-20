/*
 * @Author: xiaojia
 * @email: limu.xiao@tuya.com
 * @LastEditors:
 * @file name: dev_register.c
 * @Description: dev_register the device source file
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2021.8.30 14:46:07
 * @LastEditTime: 2021.9.1 20:05:46
 */

#include "stdio.h"
#include "stdlib.h"
#include "dev_register.h"
#include "app_common.h"
#include "light_tools.h"



#define PRIVATE_V_ATTR \
    { 0x0000, ATTR_INT8U_ATTRIBUTE_TYPE, 1, (ATTR_MASK_WRITABLE | ATTR_MASK_TOKEN_FAST), (uint8_t*)0x10 }, /* 41 / Color Control / current hue*/\
    { 0x0001, ATTR_INT8U_ATTRIBUTE_TYPE, 1, (ATTR_MASK_WRITABLE | ATTR_MASK_TOKEN_FAST), (uint8_t*)0x10 }, /* 42 / Color Control / current saturation*/\
    { TY_ATTR_LIGHT_MODE_ATTRIBUTE_ID, ATTR_INT8U_ATTRIBUTE_TYPE, 1, (0|ATTR_MASK_SINGLETON), { (uint8_t*)0x00 } }, /* current mode*/\
    { TY_ATTR_COLOR_HSV_ATTRIBUTE_ID, ATTR_INT48U_ATTRIBUTE_TYPE, 6, (ATTR_MASK_WRITABLE | ATTR_MASK_TOKEN_FAST), { (uint8_t*)0x00 } }, /* current HSV*/\
    { TY_ATTR_LIGHT_V_VALUE_ATTRIBUTE_ID, ATTR_INT8U_ATTRIBUTE_TYPE, 1, (ATTR_MASK_WRITABLE | ATTR_MASK_TOKEN_FAST), { (uint8_t*)0x00 } }, /* current V*/\
    { TY_ATTR_LIGHT_APP_DATA_ATTRIBUTE_ID, ATTR_ARRAY_ATTRIBUTE_TYPE, 13, ATTR_MASK_WRITABLE, { (uint8_t*)0x00 } }, /* app  data*/\
    { TY_ATTR_LIGHT_SCENE_DATA_ATTRIBUTE_ID, ATTR_ARRAY_ATTRIBUTE_TYPE, 67, (ATTR_MASK_SINGLETON), { (uint8_t*)0x00 } }, /* scene data*/\

#define COLOR_CONTROL_ATTR_LIST_ZG30 \
    { 0x0000, ATTR_INT8U_ATTRIBUTE_TYPE, 1, (ATTR_MASK_WRITABLE | ATTR_MASK_TOKEN_FAST), (uint8_t*)0x10 }, /*  Color Control / current hue*/\
    { 0x0001, ATTR_INT8U_ATTRIBUTE_TYPE, 1, (ATTR_MASK_WRITABLE | ATTR_MASK_TOKEN_FAST), (uint8_t*)0x10 }, /*  Color Control / current saturation*/\
    { 0x0008, ATTR_ENUM8_ATTRIBUTE_TYPE, 1, (0x00), (uint8_t*)0x02 }, /*  Color Control / color mode*/\
    { 0x400A, ATTR_BITMAP16_ATTRIBUTE_TYPE, 2, (0x00), (uint8_t*)0x0019 }, /* 54 / Color Control / color capabilities*/\
    { 0x400D, ATTR_INT16U_ATTRIBUTE_TYPE, 2, (0x00), (uint8_t*)254 }, /* 57 / Color Control / couple color temp to level min-mireds*/\
    { 0x4010, ATTR_INT16U_ATTRIBUTE_TYPE, 2, (ATTR_MASK_WRITABLE|ATTR_MASK_TOKEN_FAST), (uint8_t*)153 }, /* 58 / Color Control / start up color temperature mireds*/\
    { 0xFFFD, ATTR_INT16U_ATTRIBUTE_TYPE, 2, (0x00), (uint8_t*)0x0002 }, /* 26 / IAS Zone / cluster revision*/\


#define SERVER_CLUSTER_LEN  get_array_len(g_server_cluster_id)
#define CLIENT_CLUSTER_LEN  get_array_len(g_client_cluster_id)



const attr_t g_group_attr_list[] = {
    GROUP_ATTR_LIST
};

const attr_t g_scene_attr_list[] = {
    SCENE_ATTR_LIST
};

const attr_t g_onoff_attr_list[] = {
    ON_OFF_LIGHT_ATTR_LIST
    {TY_ATTR_COUNT_DOWN_ATTR_ID, ATTR_INT32U_ATTRIBUTE_TYPE, 4, (ATTR_MASK_WRITABLE),(uint8_t*)0x000000}, /* level Control / options */\
};

const attr_t g_level_attr_list[] = {
    LEVEL_CONTROL_ATTR_LIST
};

const attr_t g_color_attr_list[] = {
    COLOR_CONTROL_ATTR_LIST
    PRIVATE_V_ATTR
    COLOR_CONTROL_ATTR_LIST_ZG30
};

attr_t g_green_power_attr_list[] = {
    GREEN_POWER_ATTR_LIST
};

attr_t g_identify_attr_list[] = {
    IDENTIFY_ATTR_LIST
};

module_table_t module_tabel[] =
{
    {"TYZS1", TYZS1},
    {"TYZS1L", TYZS1L},
    {"TYZS2", TYZS2},
    {"TYZS2R", TYZS2R},
    {"TYZS3", TYZS3},
    {"TYZS4", TYZS4},
    {"TYZS5", TYZS5},
    {"TYZS5L", TYZS5L},
    {"TYZS6", TYZS6},
    {"TYZS7", TYZS7},
    {"TYZS8", TYZS8},
    {"TYZS9V", TYZS9V},
    {"TYZS10", TYZS10},
    {"TYZS11", TYZS11},
    {"TYZS12", TYZS12},
    {"TYZS13", TYZS13},
    {"TYZS15", TYZS15},
    {"ZS3L", ZS3L},
};

#define DEF_CLUSTER_IDENTIFY_CLUSTER_ID(a) \
    { CLUSTER_IDENTIFY_CLUSTER_ID, (attr_t *)&((a)[0]), get_array_len((a)) },

const attr_t g_private_attr_list[] = {
    PRIVATE_ATTR_LIST
};

const cluster_t g_server_cluster_id[] = {
    DEF_CLUSTER_IDENTIFY_CLUSTER_ID(g_identify_attr_list)
    DEF_CLUSTER_GROUPS_CLUSTER_ID(g_group_attr_list)
    DEF_CLUSTER_SCENES_CLUSTER_ID(g_scene_attr_list)
    DEF_CLUSTER_ON_OFF_CLUSTER_ID(g_onoff_attr_list)
    DEF_CLUSTER_LEVEL_CONTROL_CLUSTER_ID(g_level_attr_list)
    DEF_CLUSTER_COLOR_CONTROL_CLUSTER_ID(g_color_attr_list)
    DEF_CLUSTER_PRIVATE_CLUSTER_ID(g_private_attr_list)
};

cluster_t g_client_cluster_id[] = {
    DEF_CLUSTER_GREE_POWER_ID(g_green_power_attr_list)
};


const dev_description_t g_dev_des[] = {
    { 1, ZHA_PROFILE_ID, ZG_DEVICE_ID_DIMMABLE_LIGHT, SERVER_CLUSTER_LEN, (cluster_t *)&g_server_cluster_id[0], 0, NULL },
    { 0xF2, ZGP_PROFILE_ID, (DEV_ID_T)0x0061, 0, NULL,  CLIENT_CLUSTER_LEN, (cluster_t *)&g_client_cluster_id[0]},
};

const zg_dev_config_t zg_dev_config = {
    .dev_type = ZG_ROUTER,
    .beacon_send_interval_for_join = BEACON_SEND_INTERVAL_200MS,
    .zb_scan_duration = ZB_SCAN_DURATION_3,
};

const join_config_t g_join_config = {
    .auto_join_power_on_flag = TRUE,
    .auto_join_remote_leave_flag = TRUE,
    .join_timeout = ZIGBEE_JOIN_MAX_TIMEOUT,
};

void dev_zigbee_init(void)
{
    dev_register_zg_ep_infor((dev_description_t *)g_dev_des, EP_SUMS);

    dev_register_zg_dev_config((zg_dev_config_t *)&zg_dev_config);

    dev_zg_join_config((join_config_t *)&g_join_config);
}


OPERATE_RET dev_gpio_module_init(PCHAR_T name, UCHAR_T len)
{
    UCHAR_T g_module;
    UCHAR_T i;
    for(i = 0; i < MODULE_TABLE_LEN; i++) {
       if (b_string_compare(name, module_tabel[i].string)) {
          g_module =  module_tabel[i].module;
          return OPRT_OK;
       }
    }
    app_print("GPIO module init ERR, and module = %c", g_module);
    return OPRT_INVALID_PARM;
}