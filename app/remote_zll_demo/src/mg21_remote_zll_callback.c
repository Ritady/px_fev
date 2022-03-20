/**
 * @Author: limu
 * @email: limu.xiao@tuya.com
 * @LastEditors: limu
 * @file name: callbacks.c
 * @Description: this file is a template of the sdk callbacks function,
 * user need to achieve the function yourself
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2019-03-12 17:29:33
 * @LastEditTime: 2019-04-17 15:51:35
 */

#include "stdio.h"
#include "stdlib.h"
#include "tuya_mf_test.h"
#include "tuya_zigbee_sdk.h"
#include "zigbee_dev_template.h"
#include "config.h"
#include "mg21_remote_zll_callback.h"
#include "remote_control_command.h"
#include "zigbee_raw_cmd_api.h"
#include "device_register.h"
#include "app_common.h"


uint16_t pre_target_id = 0xffff;
bool_t  target_factory_new = FALSE;
zll_device_info_t zll_target_info;
user_uart_config_t* mf_test_uart_config(void);


/**
 * @description: device first power on, user can register device and init the gpio
 * @param {type} none
 * @return: none
 */
void dev_power_on_init(void)
{
    ext_plugin_cmd_callback_struct_t device_cluster_cmd_callback[] ={
        {
           CLUSTER_GROUPS_CLUSTER_ID,
           dev_group_cmd_callback
        },
        {
           CLUSTER_IDENTIFY_CLUSTER_ID,
           dev_identify_cmd_callback
        },
    };

    //registe group cluster command callback
    ext_plugin_register_cmd_handle(device_cluster_cmd_callback, get_array_len(device_cluster_cmd_callback));

    dev_init();

    //disable change power
    disable_gw_change_power();

    return;
}

/**
 * @description: system start on, zigbee stack is inited completely and
 * manufactury test is ended, the uart and the timer is ok, user can
 * init the application event and any other application variables;
 * @param {type} none
 * @return: none
 */
void dev_system_on_init(void)
{
    user_uart_config_t *config;
    config = mf_test_uart_config();

    user_uart_init(config);

    dev_pre_init();

    // zll_touchlink_enable(TRUE);

    //heartbeat callback
    dev_heartbeat_set(BATTERY_VOLTAGE, BATTERY_REPORT_PERIOD);//BATTERY_REPORT_PERIOD

    tuya_print("\r\ndevice power on: %x",get_reset_reason());
}

void zg_ota_evt_callback(ZG_OTA_EVT_T evt)
{
    switch(evt) {
        case ZG_OTA_EVT_START: {
            zg_poll_interval_change(POLL_SPEED_UP_MS);
            zg_poll_start();
            break;
        }
        default: {
            zg_poll_interval_change(POLL_NORMAL_SPEED_MS);
            zg_poll_end();
            break;
        }
    }
}

/**
 * @description: network state changed callback, when network state is changed,
 * the stack will call this function to notify the user;
 * @param {state} the new network state
 * @return: none
 */
void nwk_state_changed_callback(NET_EVT_T state)
{
    zg_info_t zg_info;
    zigbee_get_net_info(&zg_info);
    tuya_print("\r\nzg_info ch:%d, panId:%2X, nodeId:%2X, state:%d\r\n", zg_info.channel,zg_info.pan_id,zg_info.short_addr,state);
    switch(state) {
        case NET_POWER_ON_LEAVE: {
            dev_timer_stop(EVT_ZLL_HEART_BEAT);
            set_group_info(0xffff);
            break;
        }
        case NET_JOIN_START: {
            dev_change_power(11, 11);
            dev_timer_stop(EVT_ZLL_HEART_BEAT);
            break;
        }
        case NET_JOIN_TIMEOUT: {
            break;
        }
        case NET_POWER_ON_ONLINE:
        case NET_JOIN_OK: {
             led_op(LED_OP_JOIN_OK);
            if(TRUE == zg_is_zll_net())
            {
                zg_sdk_info_join_type_distribute_set();
                dev_timer_start_with_callback(EVT_ZLL_HEART_BEAT, ZLL_HEART_BEAT,dev_evt_callback);
            } else {
                dev_heartbeat_set(BATTERY_VOLTAGE, BATTERY_REPORT_PERIOD);
            }
            break;
        }
        case NET_REJOIN_OK: {
            dev_timer_stop(EVT_FORCE_REJOIN_MANUAL);
            if(TRUE == zg_is_zll_net())
            {
                zg_sdk_info_join_type_distribute_set();
                dev_timer_start_with_callback(EVT_ZLL_HEART_BEAT,ZLL_HEART_BEAT,dev_evt_callback);
            } else {
                dev_heartbeat_set(BATTERY_VOLTAGE, BATTERY_REPORT_PERIOD);
            }
            if(target_factory_new == TRUE)
            {
                target_factory_new = FALSE;
                zll_joined_device_info(zll_target_info);
            }
            break;
        }
        case NET_LOST: {
            break;
        }
        case NET_REMOTE_LEAVE: {
            set_group_info(0xffff);
            dev_timer_stop(EVT_ZLL_HEART_BEAT);
            break;
        }
        case NET_LOCAL_LEAVE: {
            emberAfGroupsClusterClearGroupTableCallback(1);//clear group info
            dev_timer_stop(EVT_ZLL_HEART_BEAT);
            break;
        }
        case NET_ZLL_JOINED: {
            zg_sdk_info_join_type_distribute_set();
            break;
        }
        case NET_ZLL_LEAVE: {
            dev_timer_start_with_callback(EVT_ZLL_RX_OFF_DELAY, 100, dev_evt_callback);
            dev_timer_stop(EVT_ZLL_HEART_BEAT);
            break;
        }
        case NET_MF_TEST_LEAVE: {
            g_work_st = DEV_WORK_ST_NORMAL;
            zg_poll_interval_change(500);
            zg_poll_end();
            zg_sleep();
            break;
        }
        default: {
            break;
        }
    }
    zigbee_get_net_info(&zg_info);
    tuya_print("\r\nzg_info ch:%d,panId:%2X,nodeId:%2X,state:%d\r\n",
                        zg_info.channel,zg_info.pan_id,zg_info.short_addr,state);
}

/**
 * @description: device manufactury test callback, when device is in manufactury test model,
 * sdk will use this callback to notify application the test item and test command;
 * @param {cmd} manufactury test type
 * @param {*args} manufactury test data
 * @param {arg_len} manufactury test data length
 * @return: none
 */
MF_TEST_RET_T dev_mf_test_callback(MF_TEST_CMD_T cmd, uint8_t *args, uint16_t arg_len)
{
    //TODO

    return MF_TEST_SUCCESS;
}

/**
 * @description: device manufactury beacon test callback, when device is in manufactury test mode,
 * sdk will use this callback to notify application to enter unsupervised production test;
 * @param {type} none
 * @return: none
 */
void dev_beacon_test_callback(void)
{
    // TODO
    return;
}

/**
 * @description: device report inforamtion when first power on(if need),;
 * @param {type} none
 * @return: none
 */
void dev_power_on_report_info(void)
{
    //TODO

    return;
}

/**
 * @description: device recovery to factury command handler
 * @param {type} just reset to fdactury or reset to factury and leave
 * @return: none
 */
void dev_recovery_factory(DEV_RESET_TYPE_T type)
{
    if(DEV_RESET_TYPE_TO_FACTORY_AND_LEAVE == type)
    {
       set_group_info(0xffff);
    }
    return;
}

/**
 * @description: device receive message callback
 * @param {*dev_msg} received message information
 * @return: ZCL_CMD_RET_T
 */
ZCL_CMD_RET_T dev_msg_recv_callback(dev_msg_t *dev_msg)
{
    //TODO

    return ZCL_CMD_RET_SUCCESS;
}

/**
 * @description: device recall scene command, recovery the scene add status
 * @param {endpoint} the endpoint of scene recall command
 * @param {*in_data} the data need to be handled of the device status
 * @return: none
 */
void dev_scene_recall_callback(uint8_t endpoint, const scene_save_data_t *in_data)
{
    //TODO

    return;
}

/**
 * @description: device scene add command, store the current status to the scene table
 * @param {endpoint} the endpoint of scene add command
 * param {*out_data} the data need to be store to scene table
 * param {*in_out_len} the data length of the store data
 * @return: none
 */
void dev_scene_add_callback(uint8_t endpoint, uint8_t *out_data, uint8_t *in_out_len)
{
    //TODO

    return;
}

/**
 * @description: write sttribute callback function
 * @param {endpoint} the endpoint of the write attribute
 * @param {cluster} the cluster id of the write attribute
 * @param {attr_id} the attribute id  of the write attribute
 * @return: none
 */
void dev_msg_write_attr_callback(uint8_t endpoint, CLUSTER_ID_T cluster, uint16_t attr_id)
{
    //TODO

    return;
}

/**
 * @description: write sttribute callback function
 * @param {endpoint} the endpoint of the write attribute
 * @param {cluster} the cluster id of the write attribute
 * @param {attr_id} the attribute id  of the write attribute
 * @param {mask} the attribute id mask
 * @param {manufacturerCode} manufacturerCode
 * @param {type} attribute data type
 * @param {size} data length
 * @param {value} attribute value
 * @return: none
 */
void dev_msg_write_attr_callback_ext(
    uint8_t endpoint,
    CLUSTER_ID_T cluster,
    uint16_t attr_id,
    uint8_t mask,
    uint16_t manufacturerCode,
    uint8_t type,
    uint8_t size,
    uint8_t* value)
{
    //TODO
    return;
}

//the device connect by zll will come into this function, to check out the connect status
void zll_joined_device_info_for_debug(zll_device_info_t zll_dev_info)
{
    g_ep_info_node_id = zll_dev_info.short_addr;
    if(ZG_ROUTER != zll_dev_info.dev_type)
    {
        dev_timer_start_with_callback(EVT_EP_INFO_DELAY_REQUEST, 7000, dev_evt_callback);
    }
    else//Router 
    {
        target_factory_new = TRUE;
        zll_target_info.dev_type = ZG_ROUTER;
        zll_target_info.short_addr = g_ep_info_node_id;
    }
    tuya_print("\r\nDebug target addr:%2x, type:%d\r\n", zll_dev_info.short_addr, zll_dev_info.dev_type);
}

/**
 * @description: zll  joined device callback, every device connect to remote will come into this function
 * @param {zll  target device information}
 * @return: none
 */
void zll_joined_device_info(zll_device_info_t zll_dev_info)
{
    uint8_t return_eui64[8];
    frame_info_t frame_info;
    zg_info_t zg_info;
    NET_EVT_T st;
    device_mac_get(return_eui64);
    frame_info.source_endpoint = 1;
    frame_info.dest_endpoint =  1;
    frame_info.dest_address = zll_dev_info.short_addr;
    zigbee_get_net_info(&zg_info);
    tuya_print("\r\ntarget addr:%2x, type:%d\r\n", zll_dev_info.short_addr, zll_dev_info.dev_type);
    led_op(LED_OP_JOIN_OK);
    if(ZG_ROUTER == zll_dev_info.dev_type)
    {
        if(STATT_TO_CONNECT_FLAG == g_touch_link_action)
        {
            st = nwk_state_get();
            if(st == NET_LOST ){//touch-link
                dev_timer_start_with_callback(EVT_FORCE_REJOIN_MANUAL, 1000, dev_evt_callback);
                target_factory_new = TRUE;
                zll_target_info.dev_type = ZG_ROUTER;
                zll_target_info.short_addr = zll_dev_info.short_addr;
            }
            else{
              group_id = get_group_info();
              remote_control_add_group(zll_dev_info.short_addr, 0xff, group_id);
              pre_target_id = zll_dev_info.short_addr;
              target_factory_new = FALSE;
            }
        }
        //on off
        dev_bind_table_entry entry;
        entry.cluster_id = CLUSTER_ON_OFF_CLUSTER_ID;
        entry.local = 1;     //local's value is the ep
        entry.remote = 1;    //remote's value is the target device ep
        entry.type = DEV_BIND_TYPE_MULTICAST;
        entry.identifier[0] = group_id & 0x00ff;
        entry.identifier[1] = (group_id >> 8) & 0x00ff;
        dev_bind_table_add(&entry);
        //level control
        entry.cluster_id = CLUSTER_LEVEL_CONTROL_CLUSTER_ID;
        entry.local = 1;    //local's value is the ep
        entry.remote = 1;   //remote's value is the target device ep
        entry.type = DEV_BIND_TYPE_MULTICAST;
        entry.identifier[0] = group_id & 0x00ff;
        entry.identifier[1] = (group_id >> 8) & 0x00ff;
        dev_bind_table_add(&entry);

        sendCommandZllCommissioningClusterEndpointInformation(return_eui64,
                                                        zg_info.short_addr,
                                                        1,
                                                        ZHA_PROFILE_ID,
                                                        ZG_DEVICE_ID_DIMMER_SWITCH,
                                                        1,
                                                        &frame_info);
    }
}

/**
 * @description: zll  touch link scan failed callback
 * @param {none}
 * @return: none
 */
void zll_touch_link_failed(void)
{
    //for reset
    if((g_touch_link_action == RESET_FLAG) && (g_zll_rst_cnt < 2))
    {
        g_zll_rst_cnt++;
        dev_timer_start_with_callback(EVT_TOUCH_LINK_RETRY, 200 , dev_evt_callback);
    }//for link
    else if((g_touch_link_action == STATT_TO_CONNECT_FLAG) && (g_zll_touch_link_cnt < 2))
    {
        g_zll_touch_link_cnt++;
        dev_timer_start_with_callback(EVT_TOUCH_LINK_RETRY, 200 , dev_evt_callback);
    }
}

/**
 * @description: mf test uart register, aotomatic generated, not edit
 * @param {void} none
 * @return: user_uart_config_t mf uart config information
 */
user_uart_config_t config;
user_uart_config_t* mf_test_uart_config(void)
{
    memset(&config, 0, sizeof(user_uart_config_t));

    if (MODULE_NAME == TYZS2R)
    {
        user_uart_config_t default_config = TYZS2R_USART_CONFIG_DEFAULT;
        memcpy(&config, &default_config, sizeof(user_uart_config_t));
    }
    else if(MODULE_NAME == TYZS5)
    {
        user_uart_config_t default_config = TYZS5_USART_CONFIG_DEFAULT;
        memcpy(&config, &default_config, sizeof(user_uart_config_t));
    }
    else if((MODULE_NAME == ZS3L) || (MODULE_NAME == ZSLC5) || (MODULE_NAME == ZS2S))
    {
        user_uart_config_t default_config = ZS3L_USART_CONFIG_DEFAULT;
        memcpy(&config, &default_config, sizeof(user_uart_config_t));
    }
    else
    {
        user_uart_config_t default_config = TYZS3_USART_CONFIG_DEFAULT;
        memcpy(&config, &default_config, sizeof(user_uart_config_t));
    }
    return &config;
}
