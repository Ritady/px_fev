/*
 * @file: device_register.c
 * @brief: 
 * @author: Arien
 * @date: 2021-08-31 18:00:00
 * @email: Arien.ye@tuya.com
 * @copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @company: http://www.tuya.com
 */
#include "device_register.h"

/* zigbee endpoint register */
// application attribute
const attr_t ias_zone_attr_list[] = {
    IAS_ZONE_ATTR_LIST1
};
// power attribute
const attr_t power_attr_list[] = {
    POWER_ATTR_LIST1
};
// application cluster
const cluster_t app_server_cluster_list[] = {
    DEF_CLUSTER_POWER_CLUSTER_ID1(power_attr_list)
    DEF_CLUSTER_IAS_ZONE_CLUSTER_ID1(ias_zone_attr_list)
};
// application device description
const dev_description_t zig_dev_desc_model[] = {
    ZIG_DEV_DESC_MODEL(app_server_cluster_list)
};


/* zigbee sleep device register */
const zg_dev_config_t st_zg_dev_config = {
    .dev_type = ZG_SLEEPY_END_DEVICE,
    .config.sleep_dev_cfg.poll_conifg.poll_interval = POLL_INTERVAL_MS,
    .config.sleep_dev_cfg.poll_conifg.wait_app_ack_time = WAIT_APP_ACK_MS,
    .config.sleep_dev_cfg.poll_conifg.poll_forever_flag = POLL_FOREVER,
    .config.sleep_dev_cfg.poll_conifg.poll_failed_times = POLL_MISS_MAX,
    .config.sleep_dev_cfg.rejoin_config.next_rejoin_time = NEXT_REJOIN_PER_HOUR * 60 * 60 * 1000,
    .config.sleep_dev_cfg.rejoin_config.wake_up_time_after_join = JOINED_CONTINUE_POLL_TIME_MS,
    .config.sleep_dev_cfg.rejoin_config.wake_up_time_after_rejoin = REJOINED_CONTINUE_POLL_TIME_MS,
    .config.sleep_dev_cfg.rejoin_config.rejoin_try_times = BEACON_TIME,
    .config.sleep_dev_cfg.rejoin_config.power_on_auto_rejoin_flag = POWER_ON_REJOIN,
    .config.sleep_dev_cfg.rejoin_config.auto_rejoin_send_data= AUTO_REJOIN_POLL,
    .beacon_send_interval_for_join = CHANNEL_SW_PER_MS,
    .beacon_send_interval_for_rejoin = BEACON_PER_MS,
    .zb_scan_duration = ZB_SCAN_DURATION_3
};

/* zigbee join register */
const join_config_t st_join_config = {
    .auto_join_power_on_flag = POWER_ON_JOIN,
    .auto_join_remote_leave_flag = RM_LEAVE_JOIN,
    .join_timeout= JOIN_TIME_OUT_MS
};


/**
 * @description: zigbee device init
 * @param {type} none
 * @return: none
 */
void zg_dev_init(void)
{
    // zigbee endpoint register
    dev_register_zg_ep_infor((dev_description_t *)zig_dev_desc_model, DEV_EP_SUM);

    // device information register
    dev_register_zg_dev_config((zg_dev_config_t *)&st_zg_dev_config);
   
    // join information register
    dev_zg_join_config((join_config_t *)&st_join_config);
}