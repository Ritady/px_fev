/*
 * @Author: xiaojia
 * @email: limu.xiao@tuya.com
 * @LastEditors:
 * @file name: app_common.c
 * @Description: common function include file
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2021.10.09 14:28:01
 * @LastEditTime: 2021.10.09 17:28:01
 */

#include "app_common.h"
#include "config.h"
#include "zigbee_dev_template.h"
#include "device_register.h"
#include "remote_control_command.h"


uint32_t key_press_time[KEY_ID_MAX] = {0};

DEV_WORK_ST_T g_work_st = DEV_WORK_ST_NORMAL;
bool_t g_button_test = FALSE;
uint16_t group_id = 0x4003;
uint8_t g_touch_link_action = RESET_FLAG;
uint8_t g_zll_touch_link_cnt = 0;
uint16_t g_ep_info_node_id = 0xffff;

uint8_t g_zll_rst_cnt = 0;
uint8_t long_dim_up  = 0;
uint8_t long_dim_down  = 0;


const gpio_config_t gpio_input_config[] = {
    {KEY0_PORT, KEY0_PIN, KEY0_MODE, KEY0_OUT, KEY0_DRIVER},
    {KEY1_PORT, KEY1_PIN, KEY1_MODE, KEY1_OUT, KEY1_DRIVER},
    {KEY2_PORT, KEY2_PIN, KEY2_MODE, KEY2_OUT, KEY2_DRIVER},
    {KEY3_PORT, KEY3_PIN, KEY3_MODE, KEY3_OUT, KEY3_DRIVER},
};

const gpio_config_t gpio_ouput_config[] = {
    {LED0_PORT, LED0_PIN, LED0_MODE, LED0_OUT, LED0_DRIVER},
};


void ember_zll_stop_scan(void)
{
    uint8_t status;
    status = emberStopScan();
    tuya_print("\r\nstop scan status:0x%x", status);
}

/**
 * @description: set_group_info
 * @param {group id} group id
 * @return: null;
 */
void set_group_info(uint16_t group_id)
{
    uint8_t group[2];
    group[0] = group_id;
    group[1] = (group_id >> 8);
    user_flash_data_write(group, 2);
}

/**
 * @description: get_group_info
 * @param null
 * @return: {group id} group id;
 */
uint16_t get_group_info(void)
{
    uint8_t group[2];
    user_flash_data_read(group, 2);
    if((group[0] == 0xff) &&(group[1] == 0xff))
    {
        if(zg_is_zll_net()){
          return 0x4003;
        } else { //TC
           return 0x0000;//
        }
    } else {
        return((group[1]<<8) + group[0]);
    }
}


void dev_group_cmd_callback(ext_plugin_args_t *args)
{
    uint16_t group_id;
    tuya_print("\r\nGroup:%d, 0x%2x, %d, %d, %d, %d", args->dest_endpoint,
                    args->cluster,
                    args->direction,
                    args->zcl_id,
                    args->cmd,
                    args->args_len
                    );
    if((args->cluster == CLUSTER_GROUPS_CLUSTER_ID)
        &&(args->direction == ZCL_DATA_DIRECTION_CLIENT_TO_SERVER))
    {
        if(args->cmd == CMD_ADD_GROUP_COMMAND_ID){
            group_id = *(uint8_t *)args->args + (*((uint8_t *)args->args + 1) << 8);
            set_group_info(group_id);
            tuya_print("\r\nadd group:0x%2x, dir:%d\r\n",group_id, args->direction);
            dev_led_stop_blink(LED_NET_ID, DEV_IO_OFF);
            zg_poll_end();
            #if 1
            //on off
            dev_bind_table_entry entry;
            entry.cluster_id = CLUSTER_ON_OFF_CLUSTER_ID;
            entry.local = 1;
            entry.remote = 1;
            entry.type = DEV_BIND_TYPE_MULTICAST;
            entry.identifier[0] = group_id & 0x00ff;
            entry.identifier[1] = (group_id >> 8) & 0x00ff;
            dev_bind_table_add(&entry);
            //level control
            entry.cluster_id = CLUSTER_LEVEL_CONTROL_CLUSTER_ID;
            entry.local = 1;
            entry.remote = 1;
            entry.type = DEV_BIND_TYPE_MULTICAST;
            entry.identifier[0] = group_id & 0x00ff;
            entry.identifier[1] = (group_id >> 8) & 0x00ff;
            dev_bind_table_add(&entry);
            #endif
        }
        else if((args->cmd == CMD_REMOVE_ALL_GROUPS_COMMAND_ID)
            ||(args->cmd == CMD_REMOVE_GROUP_COMMAND_ID)) {
            set_group_info(0xffff);
            tuya_print("\r\nremove group\r\n");
        }
    }
}


void dev_identify_cmd_callback(ext_plugin_args_t *args)
{
    uint16_t identify;
    if((args->cmd == CMD_IDENTIFY_COMMAND_ID)
      &&(args->direction == ZCL_DATA_DIRECTION_CLIENT_TO_SERVER))
    {
        identify = *(uint8_t *)args->args + (*((uint8_t *)args->args + 1) << 8);
        if(identify == 65535) {
            dev_led_start_blink(LED_STATUS_ID, 500, 500, 10, DEV_IO_OFF);
        } else {
            dev_led_start_blink(LED_STATUS_ID, 500, 500, identify, DEV_IO_OFF);
        }
    }
}


void dev_evt_callback(uint8_t evt)
{
    switch(evt) {
        case EVT_ZLL_HEART_BEAT: {
            dev_timer_start_with_callback(EVT_ZLL_HEART_BEAT, ZLL_HEART_BEAT, dev_evt_callback);
            zg_poll_start();
            dev_timer_start_with_callback(EVT_IDENTIFY_CTRL, 1000, dev_evt_callback);
            break;
        }
        case EVT_ZLL_RX_OFF_DELAY: {
            emberZllSetRxOnWhenIdle(0);
            // zll_rx_on_idle_set(0);
            break;
        }
        case EVT_JOIN_LED_DELAY_BLINK: {
            break;
        }
        case EVT_NET_LED_OFF: {
            dev_led_stop_blink(LED_NET_ID, DEV_IO_OFF);
            break;
        }
        case EVT_NET_LED_ON: {
            dev_led_stop_blink(LED_NET_ID, DEV_IO_ON);
            break;
        }
        case EVT_SLEEP_CTRL: {
            zg_wake_up(3600*1000);
            dev_mf_result_notify(MF_TEST_SUCCESS);
            break;
        }
        case EVT_IDENTIFY_CTRL: {
            dev_led_stop_blink(LED_NET_ID, DEV_IO_OFF);
            zg_poll_end();
            break;
        }
        case EVT_TOUCH_LINK_RETRY: {
            if(g_touch_link_action == RESET_FLAG) {
                zll_reset_to_factory_new_request();
            }else{
                zll_touch_link_scan();
            }
            break;
        }
        case EVT_FORCE_REJOIN_MANUAL: {
                NET_EVT_T st = nwk_state_get();
                ember_zll_stop_scan();
                if(st == NET_LOST) {
                    zg_rejoin_manual();
                    tuya_print("\r\nrejoin retry\r\n");
            }
            break;
        }
        case EVT_TOUCH_LINK_START: {
            ember_zll_stop_scan();
            zll_touch_link_scan();
            break;
        }
        case EVT_EP_INFO_DELAY_REQUEST: {
            uint8_t return_eui64[8];
            bool_t result;
            frame_info_t frame_info;
            zg_info_t zg_info;
            zigbee_get_net_info(&zg_info);
            device_mac_get(return_eui64);
            frame_info.source_endpoint = 1;
            frame_info.dest_endpoint =  1;
            frame_info.dest_address = g_ep_info_node_id;
            // 
            result = sendCommandZllCommissioningClusterEndpointInformation(return_eui64,
                                                                  zg_info.short_addr,
                                                                  1,
                                                                  ZHA_PROFILE_ID,
                                                                  ZG_DEVICE_ID_DIMMER_SWITCH,
                                                                   1,
                                                                  &frame_info);
            tuya_print("\r\nzll ep_info target:0x%2x,result=%d,\r\n",g_ep_info_node_id, result);
            break;
        }
        default:{
            break;
        }

    }
}

void led_op_base(uint8_t id, DEV_IO_ST_T st, uint16_t delay_toggle)
{
    uint8_t evt;

    if(st == DEV_IO_ON) {
        if(id == LED_NET_ID) {
            evt = EVT_NET_LED_OFF;
        }
    }
    dev_timer_stop(evt);
    dev_led_stop_blink(id, st);
    dev_timer_start_with_callback(evt, delay_toggle, dev_evt_callback);
}

void led_op(LED_OP_T op)
{
    switch(op) {
        case LED_OP_JOIN_START: {
            dev_timer_start_with_callback(EVT_JOIN_LED_DELAY_BLINK, POWER_ON_LED_ON_TIME, dev_evt_callback);
            break;
        }
        case LED_OP_JOIN_OK: {
            led_op_base(LED_NET_ID, DEV_IO_ON, JOIN_OK_LED_ON_TIME);
            break;
        }
        case LED_OP_JOIN_TIMEOUT: {
            dev_led_stop_blink(LED_NET_ID, DEV_IO_OFF);
            break;
        }
        case LED_OP_KEY_PUSH: {
            dev_led_stop_blink(LED_NET_ID, DEV_IO_ON);
            break;
        }
        case LED_OP_KEY_POP: {
            dev_led_stop_blink(LED_NET_ID, DEV_IO_OFF);
            break;
        }
        case LED_OP_KEY_TOUCH_LINK: {
            dev_led_stop_blink(LED_NET_ID, DEV_IO_OFF);
            break;
        }
        case LED_OP_KEY_TOUCH_RESET: {
            dev_led_stop_blink(LED_NET_ID, DEV_IO_OFF);
            break;
        }
        default: {
            break;
        }
    }
}


/**
 * @description: key event handler
 * @param {key_id} key  id
 * @param {key_st} key status
 * @param {push_time} key push time
 * @return: none
 */
void dev_key_handle(uint32_t key_id, key_st_t key_st, uint32_t push_time)
{
    static bool_t dim_key_combined = FALSE;
    static bool_t on_off_key_combined = FALSE;
    NET_EVT_T net_st;
    if(g_work_st == DEV_WORK_ST_TEST) {
        if((key_st != KEY_ST_PUSH) && (g_button_test)) {
            dev_mf_button_test_notify(key_id);
        }
        return;
    }
    if(key_st == KEY_ST_UP)
    {
        group_id = get_group_info();
        if(key_id == KEY_ON_ID) {
            if((key_press_time[KEY_ON_ID] > ELIMINATE_SHAKING) && (key_press_time[KEY_ON_ID] < LOCAL_CONNECT) && (key_press_time[KEY_OFF_ID] == NO_PRESS)) {
                if(0 == dev_led_is_blink(0)) {
                    remote_control_group_on(group_id, BROADCAST);
                }
            }
            on_off_key_combined = FALSE;
        }
        else if(key_id == KEY_OFF_ID) {
            if((key_press_time[KEY_OFF_ID] > ELIMINATE_SHAKING) && (key_press_time[KEY_OFF_ID] < RESET_AND_LOCAL_LEAVE) && (key_press_time[KEY_ON_ID] == NO_PRESS)) {
                if(0 == dev_led_is_blink(0)){
                    remote_control_group_off(group_id, BROADCAST);
                    tuya_print("\r\nligh off \r\n");
                }
            }
            on_off_key_combined = FALSE;
        }
        else if(key_id == KEY_DIM_DOWN_ID) {
            if(long_dim_down == 1) {
                long_dim_down = 0;
                remote_control_group_stop_level(group_id, BROADCAST);
                tuya_print("\r\nstop level\r\n");
            }
            else if((key_press_time[KEY_DIM_DOWN_ID] > ELIMINATE_SHAKING) && (key_press_time[KEY_DIM_DOWN_ID] < STEP_LEVEL) && (key_press_time[KEY_DIM_UP_ID] == NO_PRESS)) {
                if(0 == dev_led_is_blink(0)){
                    tuya_print("\r\ndim -\r\n");
                    remote_control_group_level_step(group_id, BROADCAST, 1, 51, 10);
                }
            }
            dim_key_combined = FALSE;
        }
        else if(key_id == KEY_DIM_UP_ID) {
            if(long_dim_up == 1) {
                long_dim_up = 0;
                remote_control_group_stop_level(group_id, BROADCAST);
                tuya_print("\r\nstop level\r\n");
            }
            else if((key_press_time[KEY_DIM_UP_ID] > ELIMINATE_SHAKING) && (key_press_time[KEY_DIM_UP_ID] < STEP_LEVEL) && (key_press_time[KEY_DIM_DOWN_ID] == NO_PRESS)) {
                if(0 == dev_led_is_blink(0)) {
                    remote_control_group_level_step(group_id, BROADCAST, 0, 51, 10);
                    tuya_print("\r\ndim +\r\n");
                }
            }
            dim_key_combined = FALSE;
        }
        key_press_time[key_id] = NO_PRESS;
        if((key_press_time[KEY_ON_ID] == NO_PRESS) && (key_press_time[KEY_OFF_ID] == NO_PRESS) && (key_press_time[KEY_DIM_UP_ID] == NO_PRESS) && (key_press_time[KEY_DIM_DOWN_ID] == NO_PRESS)) {
            if(0 == dev_led_is_blink(0)){
                led_op(LED_OP_KEY_POP);
            }
        }
    } else {  //pressed
        key_press_time[key_id] = push_time;
        //dim
        if((key_press_time[KEY_DIM_UP_ID] == MOVE_LEVEL) && (key_press_time[KEY_DIM_DOWN_ID] == NO_PRESS) && (key_press_time[KEY_ON_ID] == NO_PRESS) && (key_press_time[KEY_OFF_ID] == NO_PRESS)) {
            group_id = get_group_info();
            remote_control_group_move_level(group_id, BROADCAST, 0, 51);
            long_dim_up = 1;
            tuya_print("\r\nL-dim +\r\n");
        }
        if((key_press_time[KEY_DIM_DOWN_ID] == MOVE_LEVEL) && (key_press_time[KEY_DIM_UP_ID] == NO_PRESS) && (key_press_time[KEY_ON_ID] == NO_PRESS) && (key_press_time[KEY_OFF_ID] == NO_PRESS)) {
            group_id = get_group_info();
            remote_control_group_move_level(group_id, BROADCAST, 1, 51);
            long_dim_down = 1;
            tuya_print("\r\nL-dim -\r\n");
        }
        if((key_press_time[KEY_ON_ID] < LOCAL_CONNECT) || (key_press_time[KEY_OFF_ID] < RESET_AND_LOCAL_LEAVE) || (key_press_time[KEY_DIM_UP_ID] < DEV_ONLINE_PAIRING) || (key_press_time[KEY_DIM_DOWN_ID] < DEV_ONLINE_PAIRING)) {
            if((0 == dev_led_is_blink(0))){
                led_op(LED_OP_KEY_PUSH);
            }
            //rejoin if net lost
            if((key_press_time[KEY_ON_ID] == MAKE_DEV_REJOIN) || (key_press_time[KEY_OFF_ID] == MAKE_DEV_REJOIN)) {
                net_st = nwk_state_get();
                if(NET_LOST == net_st)
                {
                    zg_rejoin_manual();
                }
                tuya_print("\r\nnet_st:%d\r\n",net_st);
            }
        }
        //Local Networking   use this way can ignore gateway to pair the device
        if(key_press_time[KEY_ON_ID] == LOCAL_CONNECT)//on -3s    
        {
            if(key_press_time[KEY_OFF_ID] == NO_PRESS)
            {
                dev_timer_start_with_callback(EVT_TOUCH_LINK_START, 50,dev_evt_callback);
                dev_led_start_blink(LED_STATUS_ID, 200, 200, 50, DEV_IO_OFF);
                g_touch_link_action = STATT_TO_CONNECT_FLAG;
                g_zll_touch_link_cnt = 0;
            }
        }
        else if((key_press_time[KEY_OFF_ID] == DEV_JOIN_NWK) || (key_press_time[KEY_ON_ID] == DEV_JOIN_NWK)) {
            if((key_press_time[KEY_ON_ID] >= DEV_JOIN_NWK) && (key_press_time[KEY_OFF_ID] >= DEV_JOIN_NWK) && (on_off_key_combined == FALSE)) {
                tuya_print("\r\nStart Join...\r\n");
                dev_zigbee_join_start(ZIGBEE_JOIN_MAX_TIMEOUT);
                set_group_info(0xffff);
                dev_led_start_blink(LED_STATUS_ID, 500, 500, 60, DEV_IO_OFF);
                on_off_key_combined = TRUE;

            }
            else if(key_press_time[KEY_ON_ID] == NO_PRESS) {
                ember_zll_stop_scan();
                zll_reset_to_factory_new_request();
                dev_led_start_blink(LED_STATUS_ID, 100, 100, 75, DEV_IO_OFF);
                g_touch_link_action = RESET_FLAG;
                g_zll_rst_cnt = 0;
            }
        }
        else if((key_press_time[KEY_DIM_UP_ID] == DEV_ONLINE_PAIRING)
                ||(key_press_time[KEY_DIM_DOWN_ID] == DEV_ONLINE_PAIRING)) {
            if((key_press_time[KEY_DIM_DOWN_ID] >= DEV_ONLINE_PAIRING) && (key_press_time[KEY_DIM_UP_ID] >= DEV_ONLINE_PAIRING) && (FALSE == dim_key_combined)) {
                dim_key_combined = TRUE;
                tuya_print("\r\nidentify mode\r\n");
                dev_led_start_blink(LED_STATUS_ID, 200, 200, 150, DEV_IO_OFF);
                zg_poll_start();
                dev_timer_start_with_callback(EVT_IDENTIFY_CTRL, 60 * 1000 , dev_evt_callback);
            }
        }
    }
    return;
}

/**
 * @description: device gpio init
 * @param: none
 * @return: none
 */
void dev_gpio_init(void)
{
    gpio_button_init((gpio_config_t *)gpio_input_config, get_array_len(gpio_input_config), 50, dev_key_handle);
    gpio_output_init((gpio_config_t *)gpio_ouput_config, get_array_len(gpio_ouput_config));
}

/**
 * @description: device battery power init
 * @param {type} none
 * @return: none
 */
void dev_battery_init(void)
{
    battery_cfg_t cfg = {
        20*60*1000UL,          //cap_first_delay_time
        5000,                  //cap_waitting_silence_time
        BATTERY_REPORT_PERIOD, //cap_max_period_time
        3000,                  //cap_max_voltage
        2400,                  //cap_min_voltage
        {ADC_TYPE_VDD, NULL, NULL}
    };

    battery_table_t table[] = {
        BATTERY_DEFAULT_TABLE
    };
    
    hal_battery_config(&cfg, table, get_array_len(table));
}

/**
 * @description: device peripherals init
 * @param {type} none
 * @return: none
 */
void dev_pre_init(void)
{
    dev_battery_init();

    dev_gpio_init();

    //power on led
    led_op_base(LED_NET_ID, DEV_IO_ON, POWER_ON_LED_ON_TIME);

    //turn off continued receive to make the deviece can fall asleep
    dev_timer_start_with_callback(EVT_ZLL_RX_OFF_DELAY, 100 ,dev_evt_callback);
    tuya_print("\r\ndevice peripherals init successful");
}