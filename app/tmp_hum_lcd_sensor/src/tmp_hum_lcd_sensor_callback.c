/*
 * @file: tmp_hum_lcd_sensor_callback.c
 * @brief: 
 * @author: Arien
 * @date: 2021-10-13 20:00:00
 * @email: Arien.ye@tuya.com
 * @copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @company: http://www.tuya.com
 */
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "tuya_zigbee_sdk.h"
#include "app_common.h"
#include "device_register.h"
#include "driver_sht40.h"
#include "driver_vkl144b.h"
#include "hal_gpio.h"

/**
 * @note (MUST) This is the first function after the hardware starts.
 * The CPU and base clock are initialized before calling this function.
 * You need to implement the creation of Zigbee devices and 
 * determine the parameters of Zigbee device behavior.Include device roles(router, end device), 
 * device networking(join), rejoin parameters, and more. Refer to the TUYA Zigbee SDK demo for details.
 * @param none
 * @return none
 */
void dev_power_on_init(void)
{
    // disable key input
    g_key_lock_flg = TRUE;

    // disable beacon mf test
    mf_test_disable_beacon_test();

    // disable gateway from setting local radio power
    disable_gw_change_power();

    // zigbee device network parameter config
    zg_dev_init();
}

/**
 * @note (MUST) This is the first function after system startup. 
 * Before calling this function, Zigbee stack and some basic components have been started.
 * You can use all API except individual ones. API limits refer to the API limits description table
 * @param none
 * @return none
 */
void dev_system_on_init(void)
{
    #ifdef TUYA_DEBUG
    uart_init();
    tuya_print( "/---------------system up--------------/\r\n");
    #endif
    per_dev_init();                                         // peripheral init

    /* set max radio power */
    dev_change_power(NORMAL_TX_POWER, MAX_TX_POWER);

    dev_heartbeat_set(BATTERY_VOLTAGE, BATTERY_REPORT_MINUTE * 60 * 1000);     // set heart beat interval 240minute 240*60*1000
    sdk_cb_register(CALLBACK_TYPE_CAPTURE_BATTERY, battery_sampling_evt_callback);

    dev_led_stop_blink(LED_INDEX_0, DEV_IO_ON);
    dev_timer_start_with_callback(POWER_UP_3S_SYN_EVT_ID, POWER_UP_LED_ON_TIME, dev_evt_callback);
}

/**
 * @note (MUST) This function is invoked when the network state changes.
 * Handling network-related matters at this function is recommended.
 * @param[in] {state} Refer to NET_EVT_T for more detal.
 * @return none
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
            dev_timer_start_with_callback(LCD_REFRESH_EVT_ID, NO_DELAY_TIME, start_net_blink);
        }
        break;
        case NET_JOIN_TIMEOUT: {
            tuya_print("NET_JOIN_TIMEOUT\r\n");
            dev_led_stop_blink(LED_INDEX_0, DEV_IO_OFF);
            stop_net_blink(DEV_IO_OFF); 
        }
        break;
        case NET_POWER_ON_ONLINE:
        case NET_JOIN_OK: {
            tuya_print("NET_JOIN_OK\r\n");
            dev_led_stop_blink(LED_INDEX_0, DEV_IO_OFF);
            stop_net_blink(DEV_IO_ON);
            dev_timer_start_with_callback(NWK_UP_EVT_ID, SENSOR_TIM_START_DELAY_TIME, dev_evt_callback);
        }
        break;
        case NET_LOST: {
            tuya_print("NET_LOST\r\n");
            dev_timer_stop(NWK_UP_EVT_ID);
            dev_led_stop_blink(LED_INDEX_0, DEV_IO_OFF);
            stop_net_blink(DEV_IO_OFF);
        }
        break;
        case NET_REJOIN_OK:{
            tuya_print("NET_REJOIN_OK\r\n");
            dev_led_stop_blink(LED_INDEX_0, DEV_IO_OFF);
            stop_net_blink(DEV_IO_ON);
            dev_timer_start_with_callback(NWK_UP_EVT_ID, SENSOR_TIM_START_DELAY_TIME, dev_evt_callback);
        }
        break;
        case NET_REMOTE_LEAVE: {
            tuya_print("NET_REMOTE_LEAVE\r\n");
            dev_timer_start_with_callback(NWK_START_ASSOCIATE_EVT_ID, NO_DELAY_TIME, dev_evt_callback);
        }
        break;
        case NET_LOCAL_LEAVE: {
            tuya_print("NET_LOCAL_LEAVE\r\n");
            dev_timer_start_with_callback(NWK_START_ASSOCIATE_EVT_ID, NO_DELAY_TIME, dev_evt_callback);
        }
        break;
        default: {
            break;
        }
    }
}

/**
 * @description: device receive message callback
 * @param {*dev_msg} received message information
 * @return: ZCL_CMD_RET_T
 */
ZCL_CMD_RET_T dev_msg_recv_callback(dev_msg_t *dev_msg)
{
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
    return;
}