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
    { 0x0000,     ATTR_BOOLEAN_ATTRIBUTE_TYPE,  1, (ATTR_MASK_TOKEN_FAST|ATTR_MASK_WRITABLE),  (uint8_t*)0x00  },   /* on/off */\
    { 0xFFFD,     ATTR_INT16U_ATTRIBUTE_TYPE,   2, (0x00),                                     (uint8_t*)0x0002},   /* cluster revision */ \

#define TUYA_COMMON_ATTR_LIST \
    { 0x0000,     ATTR_BOOLEAN_ATTRIBUTE_TYPE,  1, (ATTR_MASK_TOKEN_FAST|ATTR_MASK_WRITABLE),  (uint8_t*)0x00  },   /* on/off */\
    { 0x0001,     ATTR_BOOLEAN_ATTRIBUTE_TYPE,  1, (ATTR_MASK_TOKEN_FAST|ATTR_MASK_WRITABLE),  (uint8_t*)0x00  },   /* on/off */\
    { 0xD004,     ATTR_INT16U_ATTRIBUTE_TYPE,   2, (ATTR_MASK_TOKEN_FAST|ATTR_MASK_WRITABLE),  (uint8_t*)0x0000  }, /* group id */ \
    { 0xD005,     ATTR_INT8U_ATTRIBUTE_TYPE,    1, (ATTR_MASK_TOKEN_FAST|ATTR_MASK_WRITABLE),  (uint8_t*)0x00  },   /* scene id */ \
    { 0xFFFD,     ATTR_INT16U_ATTRIBUTE_TYPE,   2, (0x00),                                     (uint8_t*)0x0002  }, /* cluster revision */ \

				
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


const attr_t g_tuya_common_attr_list[] =
{
    TUYA_COMMON_ATTR_LIST
};

#define DEF_CLUSTER_COMMON_PRIVATE_CLUSTER_ID(a) { CLUSTER_TUYA_COMMON_CLUSTER_ID, (attr_t *)&((a)[0]), get_array_len((a)) },

const cluster_t g_server_cluster_id[] =
{
    DEF_CLUSTER_IDENTIFY_CLUSTER_ID(g_identify_attr_list)
    DEF_CLUSTER_GROUPS_CLUSTER_ID(g_group_attr_list)
    DEF_CLUSTER_SCENES_CLUSTER_ID(g_scene_attr_list)
    DEF_CLUSTER_ON_OFF_CLUSTER_ID(g_light_attr_list)
    DEF_CLUSTER_COMMON_PRIVATE_CLUSTER_ID(g_tuya_common_attr_list)
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
    { EP_1,  ZHA_PROFILE_ID, ZG_DEVICE_ID_SCENE_SELECTOR, SERVER_CLUSTER_LEN, (cluster_t *)&g_server_cluster_id[0], 0, NULL },
    { EP_2,  ZHA_PROFILE_ID, ZG_DEVICE_ID_SCENE_SELECTOR, SERVER_CLUSTER_LEN, (cluster_t *)&g_server_cluster_id[0], 0, NULL },
    { EP_3,  ZHA_PROFILE_ID, ZG_DEVICE_ID_SCENE_SELECTOR, SERVER_CLUSTER_LEN, (cluster_t *)&g_server_cluster_id[0], 0, NULL },
    { EP_4,  ZHA_PROFILE_ID, ZG_DEVICE_ID_SCENE_SELECTOR, SERVER_CLUSTER_LEN, (cluster_t *)&g_server_cluster_id[0], 0, NULL },
    { EP_5,  ZHA_PROFILE_ID, ZG_DEVICE_ID_SCENE_SELECTOR, SERVER_CLUSTER_LEN, (cluster_t *)&g_server_cluster_id[0], 0, NULL },
    { EP_6,  ZHA_PROFILE_ID, ZG_DEVICE_ID_SCENE_SELECTOR, SERVER_CLUSTER_LEN, (cluster_t *)&g_server_cluster_id[0], 0, NULL },
    { EP_GP, ZGP_PROFILE_ID, (DEV_ID_T)0x0061, 0, NULL, GP_CLUSTER_LEN, (cluster_t *)&g_gp_cluster_id[0]},
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

/**
 * @description: zigbee device init
 * @param {type} none
 * @return: none
 */
void dev_zigbee_init(void)
{
    dev_register_zg_ep_infor((dev_description_t *)g_dev_des, get_array_len(g_dev_des)); // green power EP + 1 

    sdk_config_multi_key_join(TRUE);
    
    ext_plugin_green_power_client_enable(); // grenn power enable
    
    dev_register_zg_dev_config((zg_dev_config_t *)&g_zg_dev_config);

    dev_zg_join_config((join_config_t *)&g_join_config);

    dev_zigbee_enable_zigbee30_test_mode(FALSE);

    dev_zigbee_recovery_network_set(TRUE);  // nwk recover open
}
