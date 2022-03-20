/**
 * @Author: qinlang
 * @email: qinlang.chen@tuya.com
 * @LastEditors: qinlang
 * @file name: onoff_cluster.c
 * @Description: this file is a template of the sdk callbacks function,
 * user need to achieve the function yourself
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2021-08-31 17:29:33
 * @LastEditTime: 2021-08-31 15:51:35
 */
#include "onoff_cluster.h"

static ONOFF_TIME_CTL_T sg_on_off_time_ctl[CH_SUMS] = {
    {
        .start_timer_fg = 0,
        .ontime_t = 0,
        .offtime_t = 0,
    }
};

ONOFF_TIME_T g_on_off_time = {
    .channels = 1,
    .timer = LOCAL_ONOFF_EVT,
};

uint8_t g_relay_onoff_status[CH_SUMS + 1] = {0};

/**
 * @description: load attributes, operation indicators and relays
 * @param {void} 
 * @return: none
 */
void dev_status_load(void)
{
    uint8_t poweron_st = 0;
    uint8_t st = 0;
    for(uint8_t i = 0; i < CH_SUMS; i++){
        dev_zigbee_read_attribute(i+1, CLUSTER_ON_OFF_CLUSTER_ID, POWERON_STATUS_ATTRIBUTE_ID, &poweron_st, 1);
        if(poweron_st == POWERON_STATUS_OFF){
            g_relay_onoff_status[i] = DEV_IO_OFF;
        }
        else if(poweron_st == POWERON_STATUS_ON){
            g_relay_onoff_status[i] = DEV_IO_ON;
        }
        else if(poweron_st == POWERON_STATUS_MEMORY){
            dev_zigbee_read_attribute(i + 1, CLUSTER_ON_OFF_CLUSTER_ID, ATTR_ON_OFF_ATTRIBUTE_ID, &st, 1);
            g_relay_onoff_status[i] = st;
        }
        dev_switch_op(i + 1, (DEV_IO_ST_T)g_relay_onoff_status[i]);
    }
}

/**
 * @description: report onoff attributes 
 * @param {endpoint} the endpoint of the write attribute
 * @param {qs} permit loss packages or not
 * @return: none
 */
void dev_report_onoff_msg(uint8_t endpoint, SEND_QOS_T qs)
{
    dev_send_data_t send_data;
	memset(&send_data, 0, sizeof(dev_send_data_t));
	if((endpoint > CH_SUMS) || (endpoint == 0)){
		return;
	}
    send_data.zcl_id = REPORT_ZCL_ID_ONOFF;
    send_data.qos = qs;
    send_data.direction = ZCL_DATA_DIRECTION_SERVER_TO_CLIENT;
    send_data.command_id = CMD_REPORT_ATTRIBUTES_COMMAND_ID;
    send_data.addr.mode = SEND_MODE_GW;
    send_data.addr.type.gw.cluster_id = CLUSTER_ON_OFF_CLUSTER_ID;
    send_data.addr.type.gw.src_ep = endpoint;
    send_data.delay_time = 0;
    send_data.random_time = 0;
    send_data.data.zg.attr_sum = 1;
    send_data.data.zg.attr[0].attr_id = ATTR_ON_OFF_ATTRIBUTE_ID;
    send_data.data.zg.attr[0].type = ATTR_BOOLEAN_ATTRIBUTE_TYPE;
    send_data.data.zg.attr[0].value_size = 1;
    send_data.data.zg.attr[0].value[0] = g_relay_onoff_status[endpoint - 1];
    dev_zigbee_send_data(&send_data, NULL, 1000);
    dev_zigbee_write_attribute(endpoint, CLUSTER_ON_OFF_CLUSTER_ID, ATTR_ON_OFF_ATTRIBUTE_ID, &g_relay_onoff_status[endpoint - 1], ATTR_BOOLEAN_ATTRIBUTE_TYPE);
}


/**
 * @description: report led mode attributes 
 * @param {endpoint} the endpoint of the write attribute
 * @param {led_mode} led mode
 * @param {qs} permit loss packages or not
 * @return: none
 */
void dev_report_led_mode(uint8_t endpoint, uint8_t led_mode, SEND_QOS_T qs)
{
    dev_send_data_t send_data;
    memset(&send_data, 0, sizeof(dev_send_data_t));
	if((endpoint > CH_SUMS) || (endpoint == 0)){
		return;
	}
    send_data.zcl_id = REPORT_ZCL_ID_LED_MODE;
    send_data.qos = qs;
    send_data.direction = ZCL_DATA_DIRECTION_SERVER_TO_CLIENT;
    send_data.command_id = CMD_REPORT_ATTRIBUTES_COMMAND_ID;
    send_data.addr.mode = SEND_MODE_GW;
    send_data.addr.type.gw.cluster_id = CLUSTER_ON_OFF_CLUSTER_ID;
    send_data.addr.type.gw.src_ep = endpoint;
    send_data.delay_time = 0;
    send_data.random_time = 0;
    send_data.data.zg.attr_sum = 1;
    send_data.data.zg.attr[0].attr_id = LED_MODE_ATTRIBUTE_ID;
    send_data.data.zg.attr[0].type = ATTR_ENUM8_ATTRIBUTE_TYPE;
    send_data.data.zg.attr[0].value_size = 1;
    send_data.data.zg.attr[0].value[0] = led_mode;
    dev_zigbee_send_data(&send_data, NULL, 1000);
}

/**
 * @description: report attributes 
 * @param {endpoint} the endpoint of the write attribute
 * @param {led_mode} poweron status
 * @param {qs} permit loss packages or not
 * @return: none
 */
void dev_report_poweron_status(uint8_t endpoint, uint8_t poweron_status, SEND_QOS_T qs)
{
    dev_send_data_t send_data;
    memset(&send_data, 0, sizeof(dev_send_data_t));
	if((endpoint > CH_SUMS) || (endpoint == 0)){
		return;
	}
    send_data.zcl_id = REPORT_ZCL_ID_POWERON_STATUS;
    send_data.qos = qs;
    send_data.direction = ZCL_DATA_DIRECTION_SERVER_TO_CLIENT;
    send_data.command_id = CMD_REPORT_ATTRIBUTES_COMMAND_ID;
    send_data.addr.mode = SEND_MODE_GW;
    send_data.addr.type.gw.cluster_id = CLUSTER_ON_OFF_CLUSTER_ID;
    send_data.addr.type.gw.src_ep = endpoint;
    send_data.delay_time = 0;
    send_data.random_time = 0;
    send_data.data.zg.attr_sum = 1;
    send_data.data.zg.attr[0].attr_id = POWERON_STATUS_ATTRIBUTE_ID;
    send_data.data.zg.attr[0].type = ATTR_ENUM8_ATTRIBUTE_TYPE;
    send_data.data.zg.attr[0].value_size = 1;
    send_data.data.zg.attr[0].value[0] = poweron_status;
    dev_zigbee_send_data(&send_data, NULL, 1000);
}

/**
 * @description: Data about ONOFF cluster was received
 * @param dev_msg
 * @return: none
 */
void onoff_cluster_handler(dev_msg_t *dev_msg)
{
    attr_value_t *attr_list = dev_msg->data.attr_data.attr_value;
    uint8_t attr_sums = dev_msg->data.attr_data.attr_value_sums;
    if(dev_msg == NULL){
        return;
    }
    for(uint8_t i = 0; i < attr_sums; i++){
        switch(attr_list[i].cmd){
            case CMD_OFF_COMMAND_ID:{
                dev_switch_op(dev_msg->endpoint, DEV_IO_OFF);
                dev_report_onoff_msg(dev_msg->endpoint, QOS_0);
                user_on_off_time_stop(dev_msg->endpoint - 1);
                break;
            }
            case CMD_ON_COMMAND_ID:{
                dev_switch_op(dev_msg->endpoint, DEV_IO_ON);
                dev_report_onoff_msg(dev_msg->endpoint, QOS_0);
                user_on_off_time_stop(dev_msg->endpoint - 1);
                break;
            }
            case CMD_TOGGLE_COMMAND_ID:{
                dev_switch_op(dev_msg->endpoint, (DEV_IO_ST_T)!g_relay_onoff_status[dev_msg->endpoint - 1]);
                dev_report_onoff_msg(dev_msg->endpoint, QOS_0);
                user_on_off_time_stop(dev_msg->endpoint - 1);
                break;
            }
            case CMD_ON_WITH_TIMED_OFF_COMMAND_ID:{
                uint16_t ontime = (uint16_t)(attr_list[i].value[2] << 8) + attr_list[i].value[1]; 
                uint16_t offtime = (uint16_t)(attr_list[i].value[4] << 8) + attr_list[i].value[3];
                user_on_off_time_rev_msg(dev_msg->endpoint - 1, ontime, offtime);
                break;
            }                    
            default:
                break;
        }
        break;
    }
}

/**
 * @description: Received data about the onoff cluster "write attribute"
 * @param endpoint
 * @param attr id
 * @return: none
 */
void onoff_msg_write_attr_handler(uint8_t endpoint, uint16_t attr_id)
{
    uint8_t led_mode = 0;
    uint8_t poweron_st = 0;
	if((endpoint > CH_SUMS) || (endpoint == 0)){
		return;
	}
    if(attr_id == LED_MODE_ATTRIBUTE_ID){
        dev_zigbee_read_attribute(endpoint, CLUSTER_ON_OFF_CLUSTER_ID, LED_MODE_ATTRIBUTE_ID, &led_mode, 1);
        if(led_mode == LED_MODE_NONE){
            dev_report_led_mode(endpoint, LED_MODE_NONE, QOS_1);
        }
        else if(led_mode == LED_MODE_RELAY){
            dev_io_op(LED_1_IO_INDEX, (DEV_IO_ST_T)g_relay_onoff_status[endpoint - 1]);  // The status lamp is synchronized with the relay
            dev_report_led_mode(endpoint, LED_MODE_RELAY, QOS_1);
        }
        else if(led_mode == LED_MODE_POS){
            dev_io_op(LED_1_IO_INDEX, (DEV_IO_ST_T)!g_relay_onoff_status[endpoint - 1]); // Display position mode status lamp as opposed to relay
            dev_report_led_mode(endpoint, LED_MODE_POS, QOS_1);
        }
        tuya_print("set led mode %d",led_mode);
    }
    else if(attr_id == POWERON_STATUS_ATTRIBUTE_ID){
        dev_zigbee_read_attribute(endpoint, CLUSTER_ON_OFF_CLUSTER_ID, POWERON_STATUS_ATTRIBUTE_ID, &poweron_st, 1);
        if(poweron_st == POWERON_STATUS_OFF){
                dev_report_poweron_status(endpoint, POWERON_STATUS_OFF, QOS_1);
        }
        else if(poweron_st == POWERON_STATUS_ON){
                dev_report_poweron_status(endpoint, POWERON_STATUS_ON, QOS_1);
        }
        else if(poweron_st == POWERON_STATUS_MEMORY){
                dev_report_poweron_status(endpoint, POWERON_STATUS_MEMORY, QOS_1);
        }
        tuya_print("set power on status %d", poweron_st);
    }
}


/**
 * @description: on off time time out 
 * @param {channel} channel
 * @return: none
 */
void user_on_off_time_time_out(uint8_t channel)
{
	if(g_dev_des[channel].ep <= CH_SUMS){
		// Operate the relay and indicator light according to the onOFF state
		dev_switch_op(g_dev_des[channel].ep, (DEV_IO_ST_T)!g_relay_onoff_status[channel]);
		dev_report_onoff_msg(g_dev_des[channel].ep, QOS_1);
	}
}

/**
 * @description: on off time write attr 
 * @param {channel} channel
 * @param ontime
 * @param offwaittime
 * @return: none
 */
static void user_on_off_time_write(uint8_t channel, uint16_t ontime_t, uint16_t offtime_t)
{
    if(g_dev_des[channel].ep <= CH_SUMS){
        dev_zigbee_write_attribute(g_dev_des[channel].ep, CLUSTER_ON_OFF_CLUSTER_ID, ATTR_OFF_WAIT_TIME_ATTRIBUTE_ID, &ontime_t,      ATTR_INT16U_ATTRIBUTE_TYPE);
        dev_zigbee_write_attribute(g_dev_des[channel].ep, CLUSTER_ON_OFF_CLUSTER_ID, ATTR_ON_TIME_ATTRIBUTE_ID,       &offtime_t,     ATTR_INT16U_ATTRIBUTE_TYPE);
    }
}

/**
 * @description: on off time report
 * @param {channel} channel
 * @param ontime
 * @param offwaittime
 * @return: none
 */
void user_on_off_time_rpt(uint8_t channel, uint16_t ontime_t, uint16_t offtime_t)
{
  if(channel < g_on_off_time.channels){
    dev_send_data_t send_data;
    memset(&send_data, 0, sizeof(dev_send_data_t));
    send_data.zcl_id = REPORT_ZCL_ID_ONOFF_TIME;
    send_data.qos = QOS_1;
    send_data.direction = ZCL_DATA_DIRECTION_SERVER_TO_CLIENT;
    send_data.command_id = CMD_REPORT_ATTRIBUTES_COMMAND_ID;
    send_data.addr.mode = SEND_MODE_GW;
    send_data.addr.type.gw.cluster_id = CLUSTER_ON_OFF_CLUSTER_ID;
    send_data.addr.type.gw.src_ep = g_dev_des[channel].ep;
    send_data.delay_time = 0;
    send_data.random_time = 0;

    send_data.data.zg.attr[send_data.data.zg.attr_sum].attr_id = ATTR_OFF_WAIT_TIME_ATTRIBUTE_ID;
    send_data.data.zg.attr[send_data.data.zg.attr_sum].type = ATTR_INT16U_ATTRIBUTE_TYPE;
    send_data.data.zg.attr[send_data.data.zg.attr_sum].value_size = 2;
    memcpy(&send_data.data.zg.attr[send_data.data.zg.attr_sum].value[0], &ontime_t, 2);
    send_data.data.zg.attr_sum ++;
    send_data.data.zg.attr[send_data.data.zg.attr_sum].attr_id = ATTR_ON_TIME_ATTRIBUTE_ID;
    send_data.data.zg.attr[send_data.data.zg.attr_sum].type = ATTR_INT16U_ATTRIBUTE_TYPE;
    send_data.data.zg.attr[send_data.data.zg.attr_sum].value_size = 2;
    memcpy(&send_data.data.zg.attr[send_data.data.zg.attr_sum].value[0], &offtime_t, 2);
    send_data.data.zg.attr_sum ++;
    dev_zigbee_send_data(&send_data, NULL, 1000);
  }  
}

/**
 * @description: on off time stop
 * @param {channel} channel
 * @return: none
 */
void user_on_off_time_stop(uint8_t channel)
{
    if(channel < g_on_off_time.channels){
        if((sg_on_off_time_ctl[channel].ontime_t)||(sg_on_off_time_ctl[channel].offtime_t)){
            user_on_off_time_rpt(channel, sg_on_off_time_ctl[channel].ontime_t, sg_on_off_time_ctl[channel].offtime_t);
        }
        memset(&sg_on_off_time_ctl[channel], 0x00, sizeof(ONOFF_TIME_CTL_T));
        user_on_off_time_write(channel, sg_on_off_time_ctl[channel].ontime_t, sg_on_off_time_ctl[channel].offtime_t);
    }
}

/**
 * @description: on off timer
 * @param timer id
 * @return: none
 */
static void user_on_off_timer_callback(uint8_t timer)
{
    for(uint8_t i = 0; i < g_on_off_time.channels; i++){
        if(sg_on_off_time_ctl[i].start_timer_fg){
            if((sg_on_off_time_ctl[i].ontime_t) || (sg_on_off_time_ctl[i].offtime_t)){
                --sg_on_off_time_ctl[i].ontime_t;
                sg_on_off_time_ctl[i].offtime_t = sg_on_off_time_ctl[i].ontime_t;
                if(sg_on_off_time_ctl[i].offtime_t >= 1){
                    if(((sg_on_off_time_ctl[i].offtime_t - 1)%60) == 59){
                        user_on_off_time_rpt(i, sg_on_off_time_ctl[i].ontime_t, sg_on_off_time_ctl[i].offtime_t);
                    }
                }
                user_on_off_time_write(i, sg_on_off_time_ctl[i].ontime_t, sg_on_off_time_ctl[i].offtime_t);
                if((!sg_on_off_time_ctl[i].ontime_t)||(!sg_on_off_time_ctl[i].offtime_t)){
                    sg_on_off_time_ctl[i].ontime_t = 0;
                    sg_on_off_time_ctl[i].offtime_t = 0;
                    user_on_off_time_rpt(i, sg_on_off_time_ctl[i].ontime_t, sg_on_off_time_ctl[i].offtime_t);
                }
                if(!(dev_timer_get_valid_flag(g_on_off_time.timer))){
                    dev_timer_start_with_callback(g_on_off_time.timer, 1000, user_on_off_timer_callback); // Timer 1 s
                }
            }
            else{
                user_on_off_time_stop(i);
                user_on_off_time_time_out(i);
            }
        }
        else{
            if((sg_on_off_time_ctl[i].ontime_t) || (sg_on_off_time_ctl[i].offtime_t)){
                memset(&sg_on_off_time_ctl[i], 0x00, sizeof(ONOFF_TIME_CTL_T));
                user_on_off_time_rpt(i, sg_on_off_time_ctl[i].ontime_t, sg_on_off_time_ctl[i].offtime_t);
            }
        }
    }
}

/**
 * @description: on off time message receive  
 * @param {channel} channel
 * @return: none
 */
void user_on_off_time_rev_msg(uint8_t channel, uint16_t onTime, uint16_t offWaitTime)
{
    user_on_off_time_rpt(channel, onTime, offWaitTime); // report ontime or offwaittime
    if((!onTime) || (!offWaitTime)){
        user_on_off_time_stop(channel); // stop timer clear ontime and offtime
        return;
    }
    if(channel < g_on_off_time.channels){
        sg_on_off_time_ctl[channel].start_timer_fg = 1;
        sg_on_off_time_ctl[channel].ontime_t = onTime;
        sg_on_off_time_ctl[channel].ontime_t = offWaitTime;
        if(!(dev_timer_get_valid_flag(g_on_off_time.timer))){
            dev_timer_start_with_callback(g_on_off_time.timer, 1000, user_on_off_timer_callback); // Timer 1 s
        }
    }
}

