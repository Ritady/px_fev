/**
 * @Author: qinlang
 * @email: qinlang.chen@tuya.com
 * @LastEditors: qinlang
 * @file name: scene_switch_callback.c
 * @Description: this file is a template of the sdk callbacks function,
 * user need to achieve the function yourself
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2021-08-31 17:29:33
 * @LastEditTime: 2021-08-31 15:51:35
 */
#include "tuya_mf_test.h"
#include "tuya_zigbee_sdk.h"
#include "zigbee_dev_template.h"
#include "device_register.h"
#include "app_common.h"
#include "cluster_onoff.h"
#include "cluster_tuya_common.h"

#define JOIN_NETWORK_BLINK_TIME                  250     // ms
#define KEY_PRESS_BLINK_TIME                     2000    // ms
#define KEY_PUSH_TIME_FOR_ZIGBEE_JOIN_NEW        3000    // ms
#define NET_LED_BLINK_CNT                        3

#define NORMAL_TX_POWER_DBM                      11      // dbm
#define MAX_TX_POWER_DBM                         19      // dbm


static DEV_WORK_ST_T g_work_st = DEV_WORK_ST_NORMAL;

bool_t g_key_st = FALSE;

/**
 * @description: key short press processing 
 * @param {endpont} endpont
 * @return: none
 */
void key_short_press(uint8_t endpoint)
{
    uint16_t group_id = 0;
    uint8_t scene_id = 0;
    dev_zigbee_read_attribute(endpoint, CLUSTER_TUYA_COMMON_CLUSTER_ID, ATTR_SAVE_GROUP_ID_ATTRIBUTE_ID, &group_id, sizeof(group_id));
    dev_zigbee_read_attribute(endpoint, CLUSTER_TUYA_COMMON_CLUSTER_ID, ATTR_SAVE_SCENE_ID_ATTRIBUTE_ID, &scene_id, sizeof(scene_id));
    dev_scene_recall_send_command_broadcast(endpoint , group_id, scene_id);
    send_onoff_reversed(endpoint, QOS_1);
    dev_led_start_blink(NET_LED_IO_INDEX, JOIN_NETWORK_BLINK_TIME, JOIN_NETWORK_BLINK_TIME, (KEY_PRESS_BLINK_TIME / (JOIN_NETWORK_BLINK_TIME * 2)), DEV_IO_OFF);
}

/**
 * @description: key processing 
 * @param {key_id} the key_id of key
 * @param {key_st} the trigger state of key
 * @param {push_time} press time of key
 * @return: none
 */
void dev_key_handle(uint32_t key_id, key_st_t key_st, uint32_t push_time)
{
    if(CH_SUMS <= key_id){
        return;
    }
    if(g_work_st == DEV_WORK_ST_TEST){
        if(key_st != KEY_ST_PUSH){
            dev_mf_button_test_notify(key_id);
        }
        return;
    }
    switch(key_id){
        case KEY_1_IO_INDEX:
            if(key_st == KEY_ST_PUSH){ // key press
                if(g_key_st == FALSE){
                    g_key_st = TRUE;
                }
                if(push_time == KEY_PUSH_TIME_FOR_ZIGBEE_JOIN_NEW){
                    // Long press for 3s will start netled blink
                    dev_led_start_blink(NET_LED_IO_INDEX, JOIN_NETWORK_BLINK_TIME, JOIN_NETWORK_BLINK_TIME, NET_LED_BLINK_CNT, DEV_IO_OFF);
                }
            }
            else{   // key up
                g_key_st = FALSE;
                if(push_time < KEY_PUSH_TIME_FOR_ZIGBEE_JOIN_NEW){
                    key_short_press(EP_1);
                }
                else{ // Long press for 3s and then release will start join network press
                    dev_zigbee_join_start(ZIGBEE_JOIN_MAX_TIMEOUT);
                }
            }
            break;    
        case KEY_2_IO_INDEX:
            if(key_st == KEY_ST_PUSH){ // key press
            }
            else{   // key up
                if(push_time < KEY_PUSH_TIME_FOR_ZIGBEE_JOIN_NEW){
                    key_short_press(EP_2);
                }
            }
            break;
        case KEY_3_IO_INDEX:
            if(key_st == KEY_ST_PUSH){ // key press
            }
            else{   // key up
                if(push_time < KEY_PUSH_TIME_FOR_ZIGBEE_JOIN_NEW){
                    key_short_press(EP_3);
                }
            }
            break;
        case KEY_4_IO_INDEX:
            if(key_st == KEY_ST_PUSH){ // key press
            }
            else{   // key up
                if(push_time < KEY_PUSH_TIME_FOR_ZIGBEE_JOIN_NEW){
                    key_short_press(EP_4);
                }
            }
            break;
        case KEY_5_IO_INDEX:
            if(key_st == KEY_ST_PUSH){ // key press
            }
            else{   // key up
                if(push_time < KEY_PUSH_TIME_FOR_ZIGBEE_JOIN_NEW){
                    key_short_press(EP_5);
                }
            }
            break;
        case KEY_6_IO_INDEX:
            if(key_st == KEY_ST_PUSH){ // key press
            }
            else{   // key up
                if(push_time < KEY_PUSH_TIME_FOR_ZIGBEE_JOIN_NEW){
                    key_short_press(EP_6);
                }
            }
            break;			
        default:
            break;
    }   
}

/**
 * @description: device first power on, user can register device and init the gpio
 * @param: none
 * @return: none
 */
void dev_power_on_init(void)
{
    gpio_init();
    dev_zigbee_init();
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
    dev_change_power(NORMAL_TX_POWER_DBM, MAX_TX_POWER_DBM); // set tx power
    user_uart_config_t uart_config = ZS3L_USART_CONFIG_USER; // Log: Tx:PD0 Rx:PD1 115200 
    user_uart_init(&uart_config);
    tuya_print("/---------------system up--------------/\r\n");
}

/**
 * @description: network state changed callback, when network state is changed,
 * the stack will call this function to notify the user;
 * @param {state} the new network state
 * @return: none
 */
void nwk_state_changed_callback(NET_EVT_T state)
{
    switch(state){
        case NET_POWER_ON_LEAVE:{
            tuya_print("NET_POWER_ON_LEAVE \r\n");
            break;
        }
        case NET_JOIN_START:{
            tuya_print("NET_JOIN_START \r\n");
            dev_led_start_blink(NET_LED_IO_INDEX, JOIN_NETWORK_BLINK_TIME, JOIN_NETWORK_BLINK_TIME, DEV_LED_BLINK_FOREVER, DEV_IO_OFF);
            break;
        }
        case NET_JOIN_TIMEOUT:{
            tuya_print("NET_JOIN_TIMEOUT \r\n");
            dev_led_stop_blink(NET_LED_IO_INDEX, DEV_IO_ON);
            break;
        }
        case NET_POWER_ON_ONLINE:{
            tuya_print("NET_POWER_ON_ONLINE \r\n");
            break;
        }
        case NET_JOIN_OK:{
            tuya_print("NET_JOIN_OK \r\n");
            dev_led_stop_blink(NET_LED_IO_INDEX, DEV_IO_OFF);
            break;
        }
        case NET_LOST:{
            tuya_print("NET_LOST \r\n");
            break;
        }
        case NET_REMOTE_LEAVE:{
            tuya_print("NET_REMOTE_LEAVE \r\n");   
            break;
        }
        case NET_LOCAL_LEAVE:{
            tuya_print("NET_LOCAL_LEAVE \r\n");
            break;
        }
        default:
            break;
    }
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
    switch(cmd){
        case MF_TEST_LED_ON_ALL:{
            dev_led_stop_blink(NET_LED_IO_INDEX, DEV_IO_ON);
            break;
        }
        case MF_TEST_LED_OFF_ALL:{
            dev_led_stop_blink(NET_LED_IO_INDEX, DEV_IO_OFF);
            break;
        }
        case MF_TEST_LED_BLINK_ALL:{
            dev_led_start_blink(NET_LED_IO_INDEX, JOIN_NETWORK_BLINK_TIME, JOIN_NETWORK_BLINK_TIME, NET_LED_BLINK_CNT, DEV_IO_OFF);
            break;
        }
        case MF_TEST_RELAY_ON_ALL:{
            break;
        }
        case MF_TEST_RELAY_OFF_ALL:{
            break;
        }
        case MF_TEST_RELAY_BLINK_ALL:{
            break;
        }
        case MF_TEST_BUTTON:{
            g_work_st = DEV_WORK_ST_TEST;
            return MF_TEST_DOING;
        }
        case MF_TRANSFER:{
            break;
        }
        default :break;
    }
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
    // TODO
    return;
}

/**
 * @description: device recovery to factury command handler
 * @param {type} none
 * @return: none
 */
void dev_recovery_factory(DEV_RESET_TYPE_T type)
{
    uint16_t group_id = 0;
    uint8_t scene_id = 0;
    uint8_t ep = 0;
    for (uint8_t i = 0; i < CH_SUMS; i++){
        group_id = 0;
        scene_id = 0;
        ep = i + 1;
        dev_zigbee_write_attribute(ep, CLUSTER_TUYA_COMMON_CLUSTER_ID, ATTR_SAVE_GROUP_ID_ATTRIBUTE_ID, &group_id, ATTR_INT16U_ATTRIBUTE_TYPE);
        dev_zigbee_write_attribute(ep, CLUSTER_TUYA_COMMON_CLUSTER_ID, ATTR_SAVE_SCENE_ID_ATTRIBUTE_ID, &scene_id, ATTR_INT8U_ATTRIBUTE_TYPE);
    }
    tuya_print("dev_recovery_factory \r\n");
    return;
}

/**
 * @description: device receive message callback
 * @param {*dev_msg} received message information
 * @return: ZCL_CMD_RET_T
 */
ZCL_CMD_RET_T dev_msg_recv_callback(dev_msg_t *dev_msg)
{
    if(dev_msg == NULL){
        return ZCL_CMD_RET_FAILED;
    }
    switch (dev_msg->cluster){
        case CLUSTER_TUYA_COMMON_CLUSTER_ID:
            cluster_tuya_common_handler(dev_msg);
            break;
        case CLUSTER_PRIVATE_TUYA_CLUSTER_ID: // private data processing
            // todo:
            break;
        case CLUSTER_ON_OFF_CLUSTER_ID:       // standard data processin
            // todo:
            break;
        default:
            break;
    }
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
    tuya_print("scene recall EP = %d \r\n", endpoint);
    switch(in_data->type){
        case SCENE_DATA_TYPE_USER_DEFINE:
            // todo:
            break;
        case SCENE_DATA_TYPE_SERVER_DEFINE: // Parse according to the server configuration format
            // todo:
            break;
        case SCENE_DATA_TYPE_ZIGBEE_DEFINE: // Parse according to zigBee standard protocol format
            // todo:
            break;
        default:
            break;
    }
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
    if((endpoint > CH_SUMS) || (endpoint == 0)){
        return;
	}
    // Todo
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
    uint16_t group_id = 0;
    uint8_t scene_id = 0;
    if((endpoint > CH_SUMS) || (endpoint == 0)){
        return;
    }
    switch(cluster){
        case CLUSTER_ON_OFF_CLUSTER_ID:
            //onoff_msg_write_attr_handler(endpoint, attr_id);
            break;
        case CLUSTER_TUYA_COMMON_CLUSTER_ID:
            switch (attr_id)
            {
                case ATTR_SAVE_GROUP_ID_ATTRIBUTE_ID :{
                    dev_zigbee_read_attribute(endpoint, cluster, ATTR_SAVE_GROUP_ID_ATTRIBUTE_ID, &group_id, sizeof(group_id));
                    tuya_print("EP:%d write attr Group_ID:%2x", endpoint, group_id);
                    }
                    break;
                case ATTR_SAVE_SCENE_ID_ATTRIBUTE_ID :{
                    dev_zigbee_read_attribute(endpoint, cluster, ATTR_SAVE_SCENE_ID_ATTRIBUTE_ID, &scene_id, sizeof(scene_id));
                    tuya_print("EP:%d write attr Scene_ID:%2x", endpoint, scene_id);
                    }
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
    return;
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
    if (MODULE_NAME == TYZS2R){
        user_uart_config_t default_config = TYZS2R_USART_CONFIG_DEFAULT;
        memcpy(&config, &default_config, sizeof(user_uart_config_t));
    }
    else if(MODULE_NAME == TYZS5){
        user_uart_config_t default_config = TYZS5_USART_CONFIG_DEFAULT;
        memcpy(&config, &default_config, sizeof(user_uart_config_t));
    }
    else if(MODULE_NAME == ZS3L){
        user_uart_config_t default_config = ZS3L_USART_CONFIG_DEFAULT;
        memcpy(&config, &default_config, sizeof(user_uart_config_t));
    }
    else{
        user_uart_config_t default_config = TYZS3_USART_CONFIG_DEFAULT;
        memcpy(&config, &default_config, sizeof(user_uart_config_t));
    }
    return &config;
}
