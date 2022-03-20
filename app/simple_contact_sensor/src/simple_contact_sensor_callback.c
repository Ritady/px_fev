/*
 * @file: simple_contact_sensor_callback.c
 * @brief: 
 * @author: Arien
 * @date: 2021-08-31 18:00:00
 * @email: Arien.ye@tuya.com
 * @copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @company: http://www.tuya.com
 */
#include "stdio.h"
#include "stdlib.h"
#include "config.h"
#include "tuya_zigbee_sdk.h"
#include "app_common.h"
#include "device_register.h"
#include "ias_zone_cluster.h"


//APL funtion
/**
 * @description: power on init
 * @param {type} none
 * @return: none
 */
void dev_power_on_init(void)
{
    // zigbee device network parameter config
    zg_dev_init();
    disable_gw_change_power(); 
    mf_test_disable_beacon_test();
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
    #ifdef TUYA_DEBUG
    uart_init();
    tuya_print( "/---------------system up--------------/\r\n");
    #endif

    per_dev_init();                                         // peripheral init
    dev_heartbeat_set(BATTERY_VOLTAGE, 4*60*60*1000UL);     // set heart beat interval 4h 4*60*60
    dev_led_stop_blink(0, DEV_IO_ON);
    dev_timer_start_with_callback(LED_POWER_UP_SYN_EVT_ID, POWER_UP_LED_ON_TIME, dev_evt_callback);
}

/**
 * @description: network state changed callback, when network state is changed, 
 * the stack will call this function to notify the user;
 * @param {state} the new network state
 * @return: none
 */
void nwk_state_changed_callback(NET_EVT_T state)
{
    switch(state) {
        case NET_POWER_ON_LEAVE: {
            tuya_print("NET_POWER_ON_LEAVE\r\n");  
        }
        break;
        case NET_JOIN_START: {
            tuya_print("NET_JOIN_START\r\n");
        }
        break;
        case NET_JOIN_TIMEOUT: {
            tuya_print("NET_JOIN_TIMEOUT\r\n");
            dev_led_stop_blink(0, DEV_IO_OFF);  
        }
        break;
        case NET_POWER_ON_ONLINE:
        case NET_JOIN_OK: {
            tuya_print("NET_JOIN_OK\r\n");
            ias_sensor_status_sync();
            dev_led_stop_blink(0, DEV_IO_OFF);
            dev_timer_start_with_callback(NWK_UP_EVT_ID, 3000, dev_evt_callback);
        }
        break;
        case NET_LOST: {
            tuya_print("NET_LOST\r\n");
            dev_timer_stop(NWK_UP_EVT_ID);
            dev_led_stop_blink(0, DEV_IO_OFF);
        }
        break;
        case NET_REJOIN_OK:{
            tuya_print("NET_REJOIN_OK\r\n");
            ias_sensor_status_sync();
            dev_timer_start_with_callback(NWK_UP_EVT_ID, 2000, dev_evt_callback);
        }
        break;
        case NET_REMOTE_LEAVE: {
            ias_zone_off_net_status_reset(state);
            tuya_print("NET_REMOTE_LEAVE\r\n");
            dev_timer_start_with_callback(NWK_START_ASSOCIATE_EVT_ID, 0, dev_evt_callback);
        }
        break;
        case NET_LOCAL_LEAVE: {
            ias_zone_off_net_status_reset(state);
            tuya_print("NET_LOCAL_LEAVE\r\n");
        }
        break;
        case NET_MF_TEST_LEAVE: {
            ias_zone_off_net_status_reset(state);
            tuya_print("NET_LOCAL_LEAVE\r\n");
        }
        break;
        default: 
        break;      
    }
}

/**
 * @description: device receive message callback
 * @param {*dev_msg} received message information
 * @return: ZCL_CMD_RET_T
 */
ZCL_CMD_RET_T dev_msg_recv_callback(dev_msg_t *dev_msg)
{
    switch(dev_msg -> cluster) {
        case CLUSTER_PRIVATE_TUYA_CLUSTER_ID:{
            //TODO
        }
        break;
        case CLUSTER_IAS_ZONE_CLUSTER_ID:{
            ias_zone_cluster_handler(dev_msg);
        }
        break;
        default:
        break;
    }
    return ZCL_CMD_RET_SUCCESS;
}

/**
 * @description: write attribute callback function
 * @param {endpoint} the endpoint of the write attribute
 * @param {cluster} the cluster id of the write attribute
 * @param {attr_id} the attribute id  of the write attribute
 * @return: none
 */
void dev_msg_write_attr_callback(uint8_t endpoint, CLUSTER_ID_T cluster, uint16_t attr_id)
{
    switch(cluster) {
        case CLUSTER_IAS_ZONE_CLUSTER_ID: {
            ias_zone_write_attr_handler(attr_id);
        }
        break;
        default:
        break;
    }
    return;
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
    return MF_TEST_SUCCESS;
}

/**
 * @description: device report inforamtion when first power on(if need),;
 * @param {type} none
 * @return: none
 */
void dev_power_on_report_info(void)
{
    return;
}

/**
 * @description: device recovery to factury command handler
 * @param {type} none
 * @return: none
 */
void dev_recovery_factory(DEV_RESET_TYPE_T type)
{
    if( DEV_RESET_TYPE_TO_FACTORY_AND_LEAVE==type ) {
        ias_zone_unenroll( IAS_ZONE_EP );
    }
    return;
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
