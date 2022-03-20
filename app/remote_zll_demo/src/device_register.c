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
#include "device_register.h"
#include "app_common.h"
#include "tuya_zigbee_stack.h"
#include "mg21_remote_zll_callback.h"


//client cluster
const attr_t g_identify_client_attr_list[] = {
   IDENTIFY_CLIENT_ATTR_LIST
};
const attr_t g_group_client_attr_list[] = {
   GROUP_CLIENT_ATTR_LIST
};
const attr_t g_scene_client_attr_list[] = {
   SCENE_CLIENT_ATTR_LIST
};
const attr_t g_on_off_client_attr_list[] = {
   ON_OFF_CLIENT_ATTR_LIST
};
const attr_t g_level_client_attr_list[] = {
   LEVEL_CLIENT_ATTR_LIST
};
const attr_t g_color_client_attr_list[] = {
   COLOR_CLIENT_ATTR_LIST
};
const attr_t g_zll_client_attr_list[] = {
   ZLL_CLIENT_ATTR_LIST
};

//server cluster
const attr_t g_power_attr_list[] = {
    POWER_ATTR_LIST
};
const attr_t g_identify_server_attr_list[] = {
    IDENTIFY_ATTR_LIST
};

const attr_t g_group_server_attr_list[] = {
    GROUP_ATTR_LIST
};

const attr_t g_zll_server_attr_list[] = {
    ZLL_ATTR_LIST
};

//client cluster list
const cluster_t g_client_cluster_id[] =
{
    DEF_CLUSTER_IDENTIFY_CLUSTER_ID(g_identify_client_attr_list)
        DEF_CLUSTER_GROUPS_CLUSTER_ID(g_group_client_attr_list)
            DEF_CLUSTER_SCENES_CLUSTER_ID(g_scene_client_attr_list)
                DEF_CLUSTER_ON_OFF_CLUSTER_ID(g_on_off_client_attr_list)
                    DEF_CLUSTER_LEVEL_CONTROL_CLUSTER_ID(g_level_client_attr_list)
                        DEF_CLUSTER_ZLL_COMMISSIONING_CLUSTER_ID(g_zll_client_attr_list)
};

//server cluster list
const cluster_t g_server_cluster_id[] = {
    DEF_CLUSTER_POWER_CLUSTER_ID(g_power_attr_list)
        DEF_CLUSTER_IDENTIFY_CLUSTER_ID(g_identify_server_attr_list)
            DEF_CLUSTER_GROUPS_CLUSTER_ID(g_group_server_attr_list)
                DEF_CLUSTER_ZLL_COMMISSIONING_CLUSTER_ID(g_zll_server_attr_list)
};

const dev_description_t g_dev_des[] = {
    ZIG_DEV_DESC_MODEL(g_server_cluster_id, g_client_cluster_id)
};

const zg_report_table_t g_report_config[] = {
    ZG_REPORTING_CONFIG
};

const zg_dev_config_t g_zg_dev_config = {
    .dev_type =                                                     ZG_SLEEPY_END_DEVICE,
    .config.sleep_dev_cfg.poll_conifg.poll_failed_times =           POLL_FAILE_MAX,
    .config.sleep_dev_cfg.poll_conifg.poll_interval =               POLL_INTERVAL_MS,
    .config.sleep_dev_cfg.poll_conifg.wait_app_ack_time =           WAIT_APP_ACK_MS,
    .config.sleep_dev_cfg.poll_conifg.poll_forever_flag =           FALSE,

    .config.sleep_dev_cfg.rejoin_config.next_rejoin_time =          NEXT_REJOIN_TIME,
    .config.sleep_dev_cfg.rejoin_config.wake_up_time_after_join =   JOIN_WAKUP_POLL_TIME_MS,
    .config.sleep_dev_cfg.rejoin_config.wake_up_time_after_rejoin = REJOIN_WAKUP_POLL_TIME_MS,
    .config.sleep_dev_cfg.rejoin_config.rejoin_try_times =          BEACON_TIME,
    .config.sleep_dev_cfg.rejoin_config.power_on_auto_rejoin_flag = TRUE,
    .config.sleep_dev_cfg.rejoin_config.auto_rejoin_send_data =     TRUE,

    .beacon_send_interval_for_join =                                BEACON_SEND_FOR_JOIN_MS,
    .beacon_send_interval_for_rejoin =                              BEACON_SEND_FOR_REJOIN_MS,
    .zb_scan_duration =                                             ZB_SCAN_DURATION_3
};

const join_config_t join_config = {
    .auto_join_power_on_flag =          FALSE,
    .auto_join_remote_leave_flag =      FALSE,
    .join_timeout =                     ZIGBEE_JOIN_MAX_TIMEOUT
};

/**
 * @description: zigbee device init
 * @param {type} none
 * @return: none
 */
void dev_init(void)
{
    //disable beacon mf test
    mf_test_disable_beacon_test();

    // zigbee endpoint register
    dev_register_zg_ep_infor((dev_description_t *)g_dev_des, EP_SUMS);

    //enable identify server & client
    ext_plugin_identify_server_enable();

    // device information register
    dev_register_zg_dev_config((zg_dev_config_t *)&g_zg_dev_config);
    
    //join network information register
    dev_zg_join_config((join_config_t *)&join_config);
    
    // //prohibit self-recovery
    // dev_zigbee_recovery_network_set(FALSE);

    //rejoin nwk policy
    zg_rejoin_scan_policy(ZG_SCAN_POLICY_CURR_CHANNEL_FOREVER);
    
    //Try multiple key into the network to speed up the network access speed(centralized、distributed、install code)
    sdk_config_multi_key_join(TRUE);
}