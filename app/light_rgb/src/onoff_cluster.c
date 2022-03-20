/*
 * @Author: xiaojia
 * @email: limu.xiao@tuya.com
 * @LastEditors:
 * @file name: onoff_cluster.c
 * @Description: onoff cluster attribute report and gateway cmd handler source file
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2021.8.30 16:49:10
 * @LastEditTime: 2021.8.30 19:21:05
 */

#include "stdio.h"
#include "stdlib.h"
#include "light_tools.h"
#include "dev_register.h"
#include "app_common.h"
#include "light_control.h"
#include "onoff_cluster.h"
#include "color_control_cluster.h"


/***********************************************************************************************************
 * 
 *          device repoet their attribute to the gateway
 * 
 **********************************************************************************************************/

/**
 * @description: report onoff attribute value
 * @param {IN SEND_QOS_T Qos} QOS value
 * @param {IN USHORT_T delay_ms} delay report
 * @param {IN bool_t onoff} on / off
 * @return: none
 */
void report_onoff_value(SEND_QOS_T Qos, UINT_T delay_ms, bool_t onoff)
{
    dev_send_data_t send_data;
    memset(&send_data, 0, sizeof(dev_send_data_t));

    send_data.qos = Qos;
    send_data.delay_time = delay_ms;
    send_data.zcl_id = ZCL_ID_ONOFF;
    send_data.direction = ZCL_DATA_DIRECTION_SERVER_TO_CLIENT;
    send_data.command_id = CMD_REPORT_ATTRIBUTES_COMMAND_ID;
    send_data.addr.mode = SEND_MODE_GW;
    send_data.addr.type.gw.cluster_id = CLUSTER_ON_OFF_CLUSTER_ID;
    send_data.addr.type.gw.src_ep = 1;
    send_data.data.zg.attr_sum = 1;
    send_data.data.zg.attr[0].attr_id = ATTR_ON_OFF_ATTRIBUTE_ID;
    send_data.data.zg.attr[0].type = ATTR_BOOLEAN_ATTRIBUTE_TYPE;
    send_data.data.zg.attr[0].value_size = 1;
    send_data.data.zg.attr[0].value[0] = onoff;

    dev_zigbee_clear_send_data(ZG_CLEAR_ALL_ZCL_ID, &send_data.zcl_id);
    dev_zigbee_send_data(&send_data, NULL, 1000);
    app_print("Report on/off QOS:%d VALUE:%d", Qos, onoff);
}

/**
 * @description: report count down attribute value
 * @param {IN SEND_QOS_T Qos} QOS value
 * @param {IN USHORT_T delay_ms} delay report
 * @param {IN uint32_t count_down} count down value
 * @return: none
 */
void report_count_down_data(SEND_QOS_T Qos, UINT_T delay_ms, UINT_T count_down)
{
    dev_send_data_t send_data;
    memset(&send_data, 0, sizeof(dev_send_data_t));

    send_data.delay_time = delay_ms;
    send_data.qos = Qos;
    send_data.zcl_id = ZCL_ID_COUNT_DOWN;
    send_data.direction = ZCL_DATA_DIRECTION_SERVER_TO_CLIENT;
    send_data.command_id = CMD_REPORT_ATTRIBUTES_COMMAND_ID;
    send_data.addr.mode = SEND_MODE_GW;
    send_data.addr.type.gw.cluster_id = CLUSTER_ON_OFF_CLUSTER_ID;
    send_data.addr.type.gw.src_ep = 1;
    send_data.data.zg.attr_sum = 1;
    send_data.data.zg.attr[0].attr_id = TY_ATTR_COUNT_DOWN_ATTR_ID;
    send_data.data.zg.attr[0].type = ATTR_INT32U_ATTRIBUTE_TYPE;
    send_data.data.zg.attr[0].value_size = 4;
    send_data.data.zg.attr[0].value[0] = count_down;
    send_data.data.zg.attr[0].value[1] = count_down >>8;
    send_data.data.zg.attr[0].value[2] = count_down >> 16;
    send_data.data.zg.attr[0].value[3] = count_down >> 24;

    dev_zigbee_clear_send_data(ZG_CLEAR_ALL_ZCL_ID, &send_data.zcl_id);
    dev_zigbee_send_data(&send_data,NULL,1000);
}


/***********************************************************************************************************
 * 
 *          gateway send cmd to device, device handler the cmd functions
 * 
 **********************************************************************************************************/

/**
 * @note app on/off command handler
 * @param [in]{UCHAR_T} onoff
 * @param [in]{ZIGBEE_CMD_T} single or group transmit cmd
 * @return: none
 */
void app_cmd_set_onoff_handler(ZIGBEE_CMD_T cmd_type,  UINT_T onoff)
{
    SEND_QOS_T QOS = QOS_VIP_0;
    uint32_t delay_ms = 0;

    if(cmd_type == ZIGBEE_CMD_GROUP){
        delay_ms = 10000 + random_ms(10000);
        QOS = QOS_0;
    }
    report_onoff_value(QOS, delay_ms, onoff);

    OPERATE_RET ret2 = op_light_ctrl_data_switch_set(onoff);
    if (ret2 != OPRT_OK) {
        PR_ERR("Zigbee onoff set no effect");
        return;
    }
    op_light_ctrl_data_count_down_set(0);
    //update on/off attribute
    dev_zigbee_write_attribute(1,\
                               CLUSTER_ON_OFF_CLUSTER_ID,\
                               ATTR_ON_OFF_ATTRIBUTE_ID,\
                               &onoff,\
                               ATTR_INT8U_ATTRIBUTE_TYPE);
    
    app_cmd_handle_delay_timer_cb();
}


/**
 * @description: app command set count down handler
 * @param {IN UINT_T count_down} seconds
 * @return: none
 */
void app_cmd_set_count_down_handler(UINT_T count_down)
{
    SEND_QOS_T QOS = QOS_0;

    report_count_down_data(QOS, 0, count_down);
    op_light_ctrl_data_count_down_set(count_down);
    //Reply gateway
    dev_zigbee_write_attribute(1,\
                                CLUSTER_ON_OFF_CLUSTER_ID,\
                                TY_ATTR_COUNT_DOWN_ATTR_ID,\
                                &count_down,\
                                ATTR_INT32U_ATTRIBUTE_TYPE);
}


/**
 * @description: the gateway onoff cmd handler 
 * @param {UCHAR_T cmd} cmd
 * @param {attr_value_t payload} attribute value
 * @param {ZIGBEE_CMD_T cmd_type} type of cmd
 * @return: none
 */
OPERATE_RET onoff_cluster_handler(UCHAR_T cmd, attr_value_t *payload, ZIGBEE_CMD_T cmd_type)
{
    bool_t onoff_value;
    USHORT_T count_down_data = 0;
    attr_value_t attr_data_list;
    memcpy(&attr_data_list, payload, sizeof(attr_value_t));
    app_print("\r\n   %x    \r\n", cmd);
    switch(cmd) {
        case CMD_OFF_COMMAND_ID:{
            app_cmd_set_onoff_handler(cmd_type, FALSE);
            break;
        }
        case CMD_ON_COMMAND_ID:{
            app_cmd_set_onoff_handler(cmd_type, TRUE);
            break;
        }
        case CMD_TOGGLE_COMMAND_ID:{
            op_light_ctrl_data_switch_get(&onoff_value);
            app_cmd_set_onoff_handler(cmd_type, !onoff_value);
            break;
        }
        case TY_CMD_SET_COUNT_DOWN_CMD_ID:{ //tuya count_down
            count_down_data = attr_data_list.value[0] + (attr_data_list.value[1]<<8) + (attr_data_list.value[2]<<16) + (attr_data_list.value[3]<<24) ;
            app_cmd_set_count_down_handler(count_down_data);
            break;
        }
        default:
            return ZCL_CMD_RET_FAILED;
        break;
    }
    return OPRT_OK;
}
