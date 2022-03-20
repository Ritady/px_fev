/**
 * @Author: qinlang
 * @email: qinlang.chen@tuya.com
 * @LastEditors: qinlang
 * @file name: device_register.c
 * @Description: this file is a template of the sdk callbacks function,
 * user need to achieve the function yourself
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2021-08-31 17:29:33
 * @LastEditTime: 2021-08-31 15:51:35
 */
#include "device_register.h"

#define USER_ON_OFF_LIGHT_ATTR_LIST \
    { 0x0000,                          ATTR_BOOLEAN_ATTRIBUTE_TYPE, 1, (ATTR_MASK_TOKEN_FAST),                     (uint8_t*)0x00  }, /* 20 / On/off / on/off*/\
    { ATTR_ON_TIME_ATTRIBUTE_ID,       ATTR_INT16U_ATTRIBUTE_TYPE,  2, (ATTR_MASK_TOKEN_FAST|ATTR_MASK_WRITABLE),  (uint8_t*)0x00  },\
    { ATTR_OFF_WAIT_TIME_ATTRIBUTE_ID, ATTR_INT16U_ATTRIBUTE_TYPE,  2, (ATTR_MASK_TOKEN_FAST|ATTR_MASK_WRITABLE),  (uint8_t*)0x00  },\
    { LED_MODE_ATTRIBUTE_ID,           ATTR_ENUM8_ATTRIBUTE_TYPE,   1, (ATTR_MASK_TOKEN_FAST|ATTR_MASK_WRITABLE),  (uint8_t*)0x00  }, /*led mode*/ \
    { POWERON_STATUS_ATTRIBUTE_ID,     ATTR_ENUM8_ATTRIBUTE_TYPE,   1, (ATTR_MASK_TOKEN_FAST|ATTR_MASK_WRITABLE),  (uint8_t*)0x00  },/*poweron status*/ \
    { 0xFFFD,                          ATTR_INT16U_ATTRIBUTE_TYPE,  2, (0x00),                                     (uint8_t*)0x0002}, /* 24 / On/off / cluster revision*/ \

const attr_t g_identify_attr_list[] =
{
    IDENTIFY_ATTR_LIST
};      
      
const attr_t g_group_attr_list[] =
{
    GROUP_ATTR_LIST
};

const attr_t g_scene_attr_list[] =
{
    SCENE_ATTR_LIST
};

const attr_t g_light_attr_list[] =
{
    USER_ON_OFF_LIGHT_ATTR_LIST
};

const cluster_t g_server_cluster_id[] =
{
    DEF_CLUSTER_IDENTIFY_CLUSTER_ID(g_identify_attr_list)
    DEF_CLUSTER_GROUPS_CLUSTER_ID(g_group_attr_list)
    DEF_CLUSTER_SCENES_CLUSTER_ID(g_scene_attr_list)
    DEF_CLUSTER_ON_OFF_CLUSTER_ID(g_light_attr_list)
};

#define SERVER_CLUSTER_LEN  get_array_len(g_server_cluster_id)

const attr_t g_green_power_attr_list[] =
{
    GREEN_POWER_ATTR_LIST
};
const cluster_t g_gp_cluster_id[] =
{
    DEF_CLUSTER_GREE_POWER_ID(g_green_power_attr_list)
};
#define GP_CLUSTER_LEN      get_array_len(g_gp_cluster_id)

const dev_description_t g_dev_des[] =
{
    { EP_1,  ZHA_PROFILE_ID, ZG_DEVICE_ID_ON_OFF_SWITCH, SERVER_CLUSTER_LEN, (cluster_t *)&g_server_cluster_id[0], 0, NULL },
    { EP_GP, ZGP_PROFILE_ID, (DEV_ID_T)0x0061, 0, NULL, GP_CLUSTER_LEN, (cluster_t *)&g_gp_cluster_id[0]}
};

const zg_dev_config_t g_zg_dev_config =
{
    .dev_type = ZG_ROUTER,
    .config.router_cfg.reserved = 0,
};

const join_config_t g_join_config =
{
    .auto_join_power_on_flag = TRUE,
    .auto_join_remote_leave_flag = TRUE,
    .join_timeout = ZIGBEE_JOIN_MAX_TIMEOUT,
};

#define ZG_REPORTING_CONFIG \
    { ZCL_REPORTING_DIRECTION_REPORTED, 0x01, CLUSTER_ON_OFF_CLUSTER_ID, 0x0000, ZCL_CLUSTER_MASK_SERVER, 0x0000, 1, 65534, 0 }, \
      
const zg_report_table_t g_report_config[] = 
{
    ZG_REPORTING_CONFIG
};
#define REPORT_TABLE_LEN      get_array_len(g_report_config)

ext_plugin_cmd_callback_struct_t SDK_CALLBACK_CMD_LIST[] = {
    {CLUSTER_IDENTIFY_CLUSTER_ID, identify_handler}
};

/**
 * @description: zigbee device init
 * @param {type} none
 * @return: none
 */
void dev_zigbee_init(void)
{
    dev_register_zg_ep_infor((dev_description_t *)g_dev_des, CH_SUMS + 1); // green power EP + 1 

    sdk_config_multi_key_join(TRUE);
    
    ext_plugin_identify_server_enable();    // identify enable
    // ext_plugin_identify_client_enable();
    ext_plugin_register_cmd_handle(SDK_CALLBACK_CMD_LIST, get_array_len(SDK_CALLBACK_CMD_LIST));
    
    ext_plugin_green_power_client_enable(); // grenn power enable
    
    ext_plugin_reporting_enable();          // report enable
    zg_report_table_init((zg_report_table_t *)g_report_config, REPORT_TABLE_LEN);
    
    dev_register_zg_dev_config((zg_dev_config_t *)&g_zg_dev_config);

    dev_zg_join_config((join_config_t *)&g_join_config);

    dev_zigbee_enable_zigbee30_test_mode(TRUE);

    dev_zigbee_recovery_network_set(TRUE);  // nwk recover open
}
