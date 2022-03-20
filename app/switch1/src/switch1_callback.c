/**
 * @Author: qinlang
 * @email: qinlang.chen@tuya.com
 * @LastEditors: qinlang
 * @file name: switch1_callback.c
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
#include "onoff_cluster.h"

#define JOIN_NETWORK_WAIT_SECOND_MIN             2       // s
#define JOIN_NETWORK_WAIT_SECOND_MAX             6       // s
#define JOIN_NETWORK_BLINK_TIME                  250     // ms
#define KEY_PUSH_TIME_FOR_ZIGBEE_JOIN_NEW        3000    // ms

#define NORMAL_TX_POWER_DBM                      11      // dbm
#define MAX_TX_POWER_DBM                         19      // dbm

typedef enum{
    PROCESS_STATUS_NORMAL = 1,
    PROCESS_STATUS_DOWNCNT,
    PROCESS_STATUS_BEFOR_JOINING,
    PROCESS_STATUS_JOINING
}PROCESS_STATUS_T;

typedef enum{
    CONDITION_NORMAL_TO_DOWNCNT = 100,
    CONDITION_DOWNCNT_TO_BEFOR_JOINING,
    CONDITION_DOWNCNT_TO_NORMAL,
    CONDITION_BEFOR_JOINING_TO_JOINING,
    CONDITION_BEFOR_JOINING_TO_NORMAL,
    CONDITION_JOINGING_TO_NORMAL
}CONDITION_T;

static DEV_WORK_ST_T g_work_st = DEV_WORK_ST_NORMAL;

PROCESS_STATUS_T g_process_status = PROCESS_STATUS_NORMAL;

user_uart_config_t g_user_uart_config = USART_CONFIG_DEFAULT;

uint8_t g_wait_cnt = 0;
uint8_t g_key_st = 0;

void join_network_process(CONDITION_T condition);

/**
 * @description: normal to downcnt event function of process 
 * @param {evt} evt num
 * @return: none
 */
void normal_to_downcnt_event_function(uint8_t evt)
{
    if(evt == EVT_LED_BLINK){
        if(g_key_st == 0){
            join_network_process(CONDITION_NORMAL_TO_DOWNCNT);
        }
    }
}

/**
 * @description: wait cntdown function 
 * @param {evt} evt num
 * @return: none
 */
void wait_cntdown_function(uint8_t evt)
{
    if(evt == EVT_JOIN_NETWORK){
        g_wait_cnt++;
        if (g_wait_cnt < JOIN_NETWORK_WAIT_SECOND_MAX) {
            dev_timer_start(EVT_JOIN_NETWORK, 1000);
        } else {
            dev_timer_stop(EVT_JOIN_NETWORK);
            g_wait_cnt = 0;
            join_network_process(CONDITION_DOWNCNT_TO_NORMAL); // The countdown times out and returns to normal
        }
    }
}

/**
 * @description: join network process 
 * @param {condition} conversion condition
 * @return: none
 */
void join_network_process(CONDITION_T condition)
{
    switch(g_process_status){
        case PROCESS_STATUS_NORMAL:
            if(condition == CONDITION_NORMAL_TO_DOWNCNT){
                g_wait_cnt = 0;
                dev_timer_stop(EVT_JOIN_NETWORK);
                dev_timer_start_with_callback(EVT_JOIN_NETWORK, 1000, wait_cntdown_function);
                g_process_status = PROCESS_STATUS_DOWNCNT;
                tuya_print("NORMAL_TO_DOWNCNT \r\n");
            }
            break;
        case PROCESS_STATUS_DOWNCNT:
            if(condition == CONDITION_DOWNCNT_TO_BEFOR_JOINING){
                g_wait_cnt = 0;
                dev_timer_stop(EVT_JOIN_NETWORK);
                g_process_status = PROCESS_STATUS_BEFOR_JOINING;
                tuya_print("DOWNCNT_TO_BEFOR_JOINING \r\n");
            }
            else if(condition == CONDITION_DOWNCNT_TO_NORMAL){
                g_wait_cnt = 0;
                dev_timer_stop(EVT_JOIN_NETWORK);
                g_process_status = PROCESS_STATUS_NORMAL;
                tuya_print("DOWNCNT_TO_NORMAL \r\n");
            }
            break;
        case PROCESS_STATUS_BEFOR_JOINING:
            if(condition == CONDITION_BEFOR_JOINING_TO_JOINING){
                dev_zigbee_join_start(ZIGBEE_JOIN_MAX_TIMEOUT);
                g_process_status = PROCESS_STATUS_JOINING;
                tuya_print("BEFOR_JOINING_TO_JOINING \r\n");
            }
            else if(condition == CONDITION_BEFOR_JOINING_TO_NORMAL){
                g_process_status = PROCESS_STATUS_NORMAL;
                tuya_print("BEFOR_JOINING_TO_NORMAL \r\n");
            }
            break;
        case PROCESS_STATUS_JOINING:
            if(condition == CONDITION_JOINGING_TO_NORMAL){
                g_process_status = PROCESS_STATUS_NORMAL;
                tuya_print("JOINING_TO_NORMAL \r\n");
            }
            break;
        default:
            break;
    }    
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
    uint8_t ep = 1;
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
        case KEY_1_IO_INDEX:{
            if(key_st == KEY_ST_PUSH){ // key press
                if(g_key_st == 0){
                    g_key_st = 1;
                    if((g_wait_cnt >= JOIN_NETWORK_WAIT_SECOND_MIN) && (g_wait_cnt < JOIN_NETWORK_WAIT_SECOND_MAX)){
                        join_network_process(CONDITION_DOWNCNT_TO_BEFOR_JOINING);
                    }
                    else{
                        join_network_process(CONDITION_DOWNCNT_TO_NORMAL);
                    }
                }
                if(push_time == KEY_PUSH_TIME_FOR_ZIGBEE_JOIN_NEW){
                    // Long press for 10s, it starts flashing to remind users that they are about to enter the distribution network
                    dev_led_start_blink(NET_LED_1_IO_INDEX, 500, 500, 3, DEV_IO_OFF);
                    dev_timer_start_with_callback(EVT_LED_BLINK, 3000, normal_to_downcnt_event_function);
                }
            }
            else{   // key up
                g_key_st = 0;
                if(push_time < KEY_PUSH_TIME_FOR_ZIGBEE_JOIN_NEW){
                    dev_switch_op(ep, (DEV_IO_ST_T)!g_relay_onoff_status[ep - 1]);
                    dev_report_onoff_msg(ep, QOS_1);
                    join_network_process(CONDITION_BEFOR_JOINING_TO_NORMAL);
                    
                    user_on_off_time_stop(ep - 1);
                }
                else{ // after releasing it is found that the long press is more than 10s
                    join_network_process(CONDITION_BEFOR_JOINING_TO_JOINING);
                }
            }
            break;
        }
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
    dev_zigbee_init();
    gpio_init();
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
    dev_status_load();
	dev_change_power(NORMAL_TX_POWER_DBM, MAX_TX_POWER_DBM); // set tx power
    user_uart_init(&g_user_uart_config);
    tuya_print("/---------------system up--------------/\r\n");
}

/**
 * @description: handle ientify events  
 * @param {evt} evt num
 * @return: none
 */
void identify_function(uint8_t evt)
{
    uint16_t identify_time = 0;
    uint8_t led_mode = 0;
    if(evt == IDENTIFY_EVT){
        dev_zigbee_read_attribute(EP_1, CLUSTER_IDENTIFY_CLUSTER_ID, ATTR_IDENTIFY_TIME_ATTRIBUTE_ID, &identify_time, 2);
        if(identify_time > 0){
            dev_led_start_blink(LED_1_IO_INDEX, 500, 500, identify_time, DEV_IO_OFF);    // blink status led
            dev_timer_start(IDENTIFY_EVT, identify_time*1000+500);
        }
        else{
            dev_zigbee_read_attribute(EP_1, CLUSTER_ON_OFF_CLUSTER_ID, LED_MODE_ATTRIBUTE_ID, &led_mode, 1); // need state synchronization
            if(led_mode == (uint8_t)LED_MODE_RELAY){
                dev_io_op(LED_1_IO_INDEX, (DEV_IO_ST_T)g_relay_onoff_status[EP_1 - 1]);  // The status lamp is synchronized with the relay
            }
            else if(led_mode == (uint8_t)LED_MODE_POS){
                dev_io_op(LED_1_IO_INDEX, (DEV_IO_ST_T)!g_relay_onoff_status[EP_1 - 1]); // Display position mode status lamp as opposed to relay
            }
            dev_timer_stop(IDENTIFY_EVT);
        }   
    }
}

/**
 * @description: when the identify command is received  
 * @param: {args} the command messages
 * @return: none
 */
void identify_handler(ext_plugin_args_t *args)
{
    tuya_print("receive identify cluster cmd");
    if (args == NULL){
        return;
    }
    switch(args->cmd){
        case CMD_IDENTIFY_COMMAND_ID:
            if(!dev_timer_get_valid_flag(IDENTIFY_EVT)){
                dev_timer_start_with_callback(IDENTIFY_EVT, 0, identify_function);
            }
            break;
        default:
            break;
    }
}

/**
 * @description: data reporting after network access needs to be delayed
 * @param {evt} evt num
 * @return: none
 */
void join_ok_delay_function(uint8_t evt)
{
  uint16_t onTime;
  uint16_t offWaitTime;
  if(evt == JOIN_OK_DELAY_EVT){
     for(uint8_t channel = 0; channel < g_on_off_time.channels; channel++){
        onTime = 0;
        offWaitTime = 0;
        dev_zigbee_read_attribute(channel + 1, CLUSTER_ON_OFF_CLUSTER_ID, ATTR_OFF_WAIT_TIME_ATTRIBUTE_ID, &onTime,      2);
        dev_zigbee_read_attribute(channel + 1, CLUSTER_ON_OFF_CLUSTER_ID, ATTR_ON_TIME_ATTRIBUTE_ID,       &offWaitTime, 2);
        user_on_off_time_rpt(channel, onTime, offWaitTime);
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
    switch(state){
        case NET_POWER_ON_LEAVE:{
            tuya_print("NET_POWER_ON_LEAVE \r\n");
            break;
        }
        case NET_JOIN_START:{
            tuya_print("NET_JOIN_START \r\n");
            dev_led_start_blink(NET_LED_1_IO_INDEX, JOIN_NETWORK_BLINK_TIME, JOIN_NETWORK_BLINK_TIME, DEV_LED_BLINK_FOREVER, DEV_IO_OFF);
            break;
        }
        case NET_JOIN_TIMEOUT:{
            tuya_print("NET_JOIN_TIMEOUT \r\n");
            dev_led_stop_blink(NET_LED_1_IO_INDEX, DEV_IO_ON);
            join_network_process(CONDITION_JOINGING_TO_NORMAL);
            break;
        }
        case NET_POWER_ON_ONLINE:{
            tuya_print("NET_POWER_ON_ONLINE \r\n");
            break;
        }
        case NET_JOIN_OK:{
            tuya_print("NET_JOIN_OK \r\n");
            dev_led_stop_blink(NET_LED_1_IO_INDEX, DEV_IO_OFF);
            join_network_process(CONDITION_JOINGING_TO_NORMAL);
            dev_timer_start_with_callback(JOIN_OK_DELAY_EVT, 5000, join_ok_delay_function); // report time delay 5s
            break;
        }
        case NET_LOST:{
            tuya_print("NET_LOST \r\n");
            break;
        }
        case NET_REMOTE_LEAVE:{
            tuya_print("NET_REMOTE_LEAVE \r\n");
            for(uint8_t channel = 0; channel < g_on_off_time.channels; channel++){
                user_on_off_time_stop(channel);
            }        
            break;
        }
        case NET_LOCAL_LEAVE:{
            tuya_print("NET_LOCAL_LEAVE \r\n");
            for(uint8_t channel = 0; channel < g_on_off_time.channels; channel++){
                user_on_off_time_stop(channel);
            }
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
            dev_led_stop_blink(NET_LED_1_IO_INDEX, DEV_IO_ON);
            dev_led_stop_blink(LED_1_IO_INDEX, DEV_IO_ON);
            break;
        }
        case MF_TEST_LED_OFF_ALL:{
            dev_led_stop_blink(NET_LED_1_IO_INDEX, DEV_IO_OFF);
            dev_led_stop_blink(LED_1_IO_INDEX, DEV_IO_OFF);
            break;
        }
        case MF_TEST_LED_BLINK_ALL:{
            dev_led_start_blink(NET_LED_1_IO_INDEX, 500, 500, 4, DEV_IO_OFF);
            dev_led_start_blink(LED_1_IO_INDEX,     500, 500, 4, DEV_IO_OFF);
            break;
        }
        case MF_TEST_RELAY_ON_ALL:{
            dev_led_stop_blink(RELAY_1_IO_INDEX, DEV_IO_ON);
            break;
        }
        case MF_TEST_RELAY_OFF_ALL:{
            dev_led_stop_blink(RELAY_1_IO_INDEX, DEV_IO_OFF);
            break;
        }
        case MF_TEST_RELAY_BLINK_ALL:{
            dev_led_start_blink(RELAY_1_IO_INDEX, 500, 500, 4, DEV_IO_OFF);
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
    dev_report_onoff_msg(1, QOS_1);
    return;
}

/**
 * @description: device recovery to factury command handler
 * @param {type} none
 * @return: none
 */
void dev_recovery_factory(DEV_RESET_TYPE_T type)
{
    uint8_t st = DEV_IO_OFF;
    uint8_t led_mode = LED_MODE_NONE;
    uint8_t poweron_status = POWERON_STATUS_OFF;
    dev_zigbee_write_attribute( EP_1, CLUSTER_ON_OFF_CLUSTER_ID, ATTR_ON_OFF_ATTRIBUTE_ID,    &st,             ATTR_BOOLEAN_ATTRIBUTE_TYPE);
	dev_zigbee_write_attribute( EP_1, CLUSTER_ON_OFF_CLUSTER_ID, POWERON_STATUS_ATTRIBUTE_ID, &led_mode,       ATTR_ENUM8_ATTRIBUTE_TYPE);
    dev_zigbee_write_attribute( EP_1, CLUSTER_ON_OFF_CLUSTER_ID, LED_MODE_ATTRIBUTE_ID,       &poweron_status, ATTR_ENUM8_ATTRIBUTE_TYPE);
    tuya_print("factory recovery");
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
        case CLUSTER_PRIVATE_TUYA_CLUSTER_ID:{ // private data processing
            // todo:
            break;
        }
        case CLUSTER_ON_OFF_CLUSTER_ID:{       // standard data processin
            onoff_cluster_handler(dev_msg);
        }
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
    switch(in_data->type){
        case SCENE_DATA_TYPE_USER_DEFINE:{
            if(in_data->data[0] == 1){
                dev_switch_op(endpoint, DEV_IO_ON);
                dev_report_onoff_msg(endpoint, QOS_1);
            }
            else if(in_data->data[0] == 0){
                dev_switch_op(endpoint, DEV_IO_OFF);
                dev_report_onoff_msg(endpoint, QOS_1);
            }
            break;
        }
        case SCENE_DATA_TYPE_SERVER_DEFINE: // Parse according to the server configuration format
            break;
        case SCENE_DATA_TYPE_ZIGBEE_DEFINE: // Parse according to zigBee standard protocol format
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
    *in_out_len = 1;
    out_data[0] = g_relay_onoff_status[endpoint - 1];
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
	if((endpoint > CH_SUMS) || (endpoint == 0)){
		return;
	}
    switch(cluster){
        case CLUSTER_ON_OFF_CLUSTER_ID:
            onoff_msg_write_attr_handler(endpoint, attr_id);
            break;
        case CLUSTER_TUYA_ELECTRICIAN_CLUSTER_ID:
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
