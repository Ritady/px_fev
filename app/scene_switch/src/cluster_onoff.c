/**
 * @Author: qinlang
 * @email: qinlang.chen@tuya.com
 * @LastEditors: qinlang
 * @file name: cluster_onoff.c
 * @Description: this file is a template of the sdk callbacks function,
 * user need to achieve the function yourself
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2021-08-31 17:29:33
 * @LastEditTime: 2021-08-31 15:51:35
 */
#include "cluster_onoff.h"

/**
 * @description: report onoff attributes 
 * @param {endpoint} 
 * @param {qs} The reliable behavior of sending data
 * @return: none
 */
void send_onoff_reversed(uint8_t endpoint, SEND_QOS_T qs)
{
    dev_send_data_t send_data;
	memset(&send_data, 0, sizeof(dev_send_data_t));
	if((endpoint > CH_SUMS) || (endpoint == 0)){
		return;
	}
    send_data.zcl_id = REPORT_ZCL_ID_PRIVATE_CMD;
    send_data.qos = qs;
    send_data.direction = ZCL_DATA_DIRECTION_CLIENT_TO_SERVER;
    send_data.command_id = CMD_KEY_INDEX_COMMAND_ID;
    send_data.commmand_type = ZCL_COMMAND_CLUSTER_SPEC_CMD;
    send_data.addr.mode = SEND_MODE_GW;
    send_data.addr.type.gw.cluster_id = CLUSTER_ON_OFF_CLUSTER_ID;
    send_data.addr.type.gw.src_ep = endpoint;
    send_data.delay_time = 0;
    send_data.random_time = 0;

    send_data.data.private.len = 1;
    send_data.data.private.data[0] = 0;
    dev_zigbee_send_data(&send_data, NULL, 1000);
}

/**
 * @description: Data about onoff cluster was received
 * @param {dev_msg}
 * @return: none
 */
void cluster_onoff_handler(dev_msg_t *dev_msg)
{
    attr_value_t *attr_list = dev_msg->data.attr_data.attr_value;
    uint8_t attr_sums = dev_msg->data.attr_data.attr_value_sums;
    if(dev_msg == NULL){
        return;
    }
    for(uint8_t i = 0; i < attr_sums; i++){
        switch(attr_list[i].cmd){
            case CMD_OFF_COMMAND_ID:{
                // Todo
                break;
            }
            case CMD_ON_COMMAND_ID:{
                // Todo
                break;
            }
            case CMD_TOGGLE_COMMAND_ID:{
                // Todo
                break;
            }                  
            default:
                break;
        }
        break;
    }
}



