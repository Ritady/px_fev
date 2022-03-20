/**
 * @Author: qinlang
 * @email: qinlang.chen@tuya.com
 * @LastEditors: qinlang
 * @file name: cluster_tuya_common.c
 * @Description: this file is a template of the sdk callbacks function,
 * user need to achieve the function yourself
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2021-08-31 17:29:33
 * @LastEditTime: 2021-08-31 15:51:35
 */
#include "cluster_tuya_common.h"

/**
 * @description: response scene add 
 * @param {endpoint} 
 * @param {attribute} 
 * @param {value} payload
 * @param {value_type} type of payload
 * @param {value_size} size of payload
 * @param {qos} The reliable behavior of sending data
 * @return: none
 */
void response_scene_add_cmd(uint8_t endpoint, uint16_t attribute, uint8_t* value, ATTR_TYPE_T value_type, uint8_t value_size, SEND_QOS_T qos)
{
    dev_send_data_t send_data;
    memset(&send_data, 0, sizeof(dev_send_data_t));

    send_data.zcl_id = REPORT_ZCL_ID_RESPONSE_SCENE_ADD;
    send_data.qos = qos;
    send_data.direction = ZCL_DATA_DIRECTION_SERVER_TO_CLIENT;
    send_data.command_id = CMD_REPORT_ATTRIBUTES_COMMAND_ID;
    send_data.addr.mode = SEND_MODE_GW;
    send_data.addr.type.gw.cluster_id = CLUSTER_TUYA_COMMON_CLUSTER_ID;
    send_data.addr.type.gw.src_ep = endpoint;
    send_data.delay_time = 0; // no delay
    send_data.random_time = 0;
    send_data.data.zg.attr_sum = 1;
    send_data.data.zg.attr[0].attr_id = attribute;
    send_data.data.zg.attr[0].type = value_type;
    send_data.data.zg.attr[0].value_size = value_size;
    memcpy(&send_data.data.zg.attr[0].value[0], value, value_size);
    dev_zigbee_clear_send_data(ZG_CLEAR_ALL_ZCL_ID, &send_data.zcl_id);
    dev_zigbee_send_data(&send_data, NULL, 1000);
}

/**
 * @description: Data about tuya common cluster was received
 * @param {dev_msg} msg it's coming in from one level up
 */
void cluster_tuya_common_handler(dev_msg_t *dev_msg)
{
    uint8_t cmd = dev_msg->data.bare_data.commandId;
    uint8_t len = dev_msg->data.bare_data.len;
    uint8_t *data = dev_msg->data.bare_data.data;
    uint8_t endpoint = dev_msg->endpoint;
    uint16_t group_id = 0;
    uint8_t scene_id = 0;
    uint8_t tmp = 0;
    uint8_t ep = 0;
    if(dev_msg == NULL){
        return;
    }
    switch(cmd){
        case CMD_RECEIVE_GROUP_ID_SCENE_ID_COMMAND_ID:
            if(len == 4){ // len is true response sucess
                ep = data[0];
                group_id = (uint16_t)data[2] << 8 | data[1];
                scene_id = data[3];
                tuya_print("ep = %d  gip = %2x  sid = %x \r\n", ep, group_id, scene_id);
                dev_zigbee_write_attribute(ep, CLUSTER_TUYA_COMMON_CLUSTER_ID, ATTR_SAVE_GROUP_ID_ATTRIBUTE_ID, &group_id, ATTR_INT16U_ATTRIBUTE_TYPE);
                dev_zigbee_write_attribute(ep, CLUSTER_TUYA_COMMON_CLUSTER_ID, ATTR_SAVE_SCENE_ID_ATTRIBUTE_ID, &scene_id, ATTR_INT8U_ATTRIBUTE_TYPE);
                tmp = TRUE;
                response_scene_add_cmd(endpoint, CUSTOM_GROUP_SCENE_ATTR_ID, &tmp, ATTR_BOOLEAN_ATTRIBUTE_TYPE, sizeof(tmp), QOS_1);
            }
            else{ // len is false response fail
                tmp = FALSE;
                response_scene_add_cmd(endpoint, CUSTOM_GROUP_SCENE_ATTR_ID, &tmp, ATTR_BOOLEAN_ATTRIBUTE_TYPE, sizeof(tmp), QOS_1);
            }
            break;                 
        default:
            break;
    }
    
}



