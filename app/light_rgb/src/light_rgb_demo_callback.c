/**************************************************************
 * @file common main file for light
 * @version 1.0.0
 * @author Xiao Jia
 * @data 2021.09.08
 *************************************************************/
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "config.h"
#include "hal_flash.h"
#include "dev_register.h"
#include "onoff_cluster.h"
#include "level_cluster.h"
#include "color_control_cluster.h"
#include "app_common.h"
#include "light_control.h"
#include "light_tools.h"
#include "tuya_zigbee_stack.h"


LIGHT_MODE_E pre_recall_mode = MODE_MAX;
void identify_handler(ext_plugin_args_t *args);
ext_plugin_cmd_callback_struct_t SDK_CALLBACK_CMD_LIST[] = {
    {CLUSTER_IDENTIFY_CLUSTER_ID, identify_handler}
};


user_uart_config_t* mf_test_uart_config(void);


/**
 * @description: identify command processing
 * @param {args} identify command parameter
 * @return: none
 */ 
void identify_handler(ext_plugin_args_t *args)
{
    app_print("it's in  identify_handler");
    switch (args->cmd) {
        case CMD_IDENTIFY_COMMAND_ID:
        case CMD_TRIGGER_EFFECT_COMMAND_ID:{
#if 0  //zigbee 3.0
            USHORT_T tims_s = *(USHORT_T*)args->args;
            if (tims_s != 0) {
                opLightCtrlBreathingStart(BREATH_COLOR_CURRENT, tims_s*2 ,500);
            } else {
                opLightCtrlBreathingStop();
            }
#endif

#if 1  //tuya
			light_ctrl_identify_handler(1);
#endif
            break;
        }
        default:
            break;
    }
}

/**
 * @description: When receiving a command, control whether to write to Flash
 * @param {save} 1 yes,0 no
 * @return {type} none
 */
void light_system_need_save_set(bool_t save)
{
    cmd_control_shade_flag = save;
}

/**
 * @description: Get whether it is currently in production test mode
 * @param {type} none
 * @return {bool_t} TRUE:yes; FALSE:no.
 */
bool_t light_system_is_product_testing(void)
{
    if (mf_test_flag) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/**
 * @description: Determine whether the network is currently being configured
 * @param {type} none
 * @return {bool_t} TRUE:yes; FALSE:no.
 */
bool_t light_system_is_network_streeing(void)
{
    NET_EVT_T nwk_state = nwk_state_get();
    if (nwk_state == NET_JOIN_START) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/**
 * @description: Get the current network type
 * @param {type} none
 * @return: network type
 */
static UCHAR_T get_nwk_type(void)
{
    UCHAR_T nwk_type = 5;

    nwk_type = zg_get_join_type();
    if (nwk_type != ZG_JOIN_TYPE_NO_NETWORK) {
        if (zg_is_zll_net() == TRUE) {
            nwk_type = ZG_JOIN_TYPE_ZLL;
        }
    }
    return nwk_type;
}

static void clear_recall_scene_report(void)
{
    UCHAR_T zcl_id = ZCL_ID_ONOFF;
    dev_zigbee_clear_send_data(ZG_CLEAR_ALL_ZCL_ID, &zcl_id);
    zcl_id = ZCL_ID_APP_DATA;
    dev_zigbee_clear_send_data(ZG_CLEAR_ALL_ZCL_ID, &zcl_id);
}

/**
 * @description: start connect network
 * @param {none}
 * @return: none
 */
void network_join_start(void)
{
    if (mf_test_flag == 1) {
        return;
    }
    dev_zigbee_join_start(ZIGBEE_JOIN_MAX_TIMEOUT);
    nwk_disable_self_recovery_once();
}


/**
 * @description: device first power on, user can register device and init the gpio
 * @param {type} none
 * @return: none
 */

void dev_power_on_init(void)
{ 
    //zigbee deive type and cluster register
    dev_zigbee_init();
#ifdef APP_DEBUG
    user_uart_config_t *default_config = mf_test_uart_config();
    user_uart_init(default_config);
#endif

    ext_plugin_identify_server_enable();
    ext_plugin_green_power_client_enable();
    ext_plugin_register_cmd_handle(SDK_CALLBACK_CMD_LIST, get_array_len(SDK_CALLBACK_CMD_LIST));

}

/**
 * @description: dev_init_before_mf_test,
 * @param {type} none
 * @return: none
 */
void dev_init_before_mf_test(void)
{
    OPERATE_RET ret;
    app_print("light start-----------");    
    
    ret = dev_light_init();
    if (ret != OPRT_OK) {
        PR_ERR("light init ERROR");
    }
    dev_load_attr_data();
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
    dev_change_power(11, 11);  //  dev_change_power(normal_db, max_db),this function can set the device Signal strength

    dev_read_attr_data();

    dev_reset_cnt_proc(); //Go back directly, or set the CTRL parameter to off-grid data and save it to start network configuration.

    op_light_ctrl_proc();

    dev_heartbeat_set(APP_VERSION, ZIGBEE_HEAR_BEAT_DURATION);
    
    return;
}

/**
 * @description: network join ok handler
 * @param {none}
 * @return: none
 */
static void network_join_ok_handler(void)
{
    UCHAR_T onoff = 1;
    LIGHT_MODE_E mode = 1;

    op_light_ctrl_data_mode_set(mode);
 
    remember_light_proc();

    op_light_ctrl_proc();

    dev_load_attr_data();

    report_onoff_value(QOS_1, 500, onoff);  ///<  report data need
    report_mode_value(QOS_1, 1000, mode);
    report_count_down_data(QOS_1, 1200, 0);
}

/**
 * @description: network state changed callback, when network state is changed,
 * the stack will call this function to notify the user;
 * @param {state} the new network state
 * @return: none
 */
void nwk_state_changed_callback(NET_EVT_T state)
{
    UCHAR_T mode = 1;
    g_nwk_type = get_nwk_type();
    switch(state) {
        case NET_POWER_ON_LEAVE: { //device power on leave network
            PR_ERR("Network state: NET_POWER_ON_LEAVE");
            break;
        }
        case NET_POWER_ON_ONLINE:{ //device power on and is online
            app_print("Network state: NET_POWER_ON_ONLINE");
            network_join_ok_handler();
            op_light_ctrl_proc();
            break;
        }
        case NET_JOIN_START: {     //device start join network
            PR_ERR("Network state: NET_JOIN_START");
            op_light_ctrl_blink_start(1000);
            break;
        }
        case NET_JOIN_TIMEOUT: {   //device join network outtime
            PR_ERR("Network state: NET_JOIN_TIMEOUT");
            op_light_ctrl_blink_stop();
        }
            break;
        case NET_JOIN_OK: {        //device join network successful
            PR_ERR("Network state: NET_JOIN_OK");
            op_light_ctrl_blink_stop();
            network_join_ok_handler();
            op_light_ctrl_data_auto_save();
            op_light_ctrl_nwk_join_ok_notify();
        }
            break;
        case NET_LOST: {           //sleeping node: parent node lost
            PR_ERR("Network state: NET_LOST");
        }
            break;
        case NET_REJOIN_OK: {      //sleeping node: rejoin successful
            PR_ERR("Network state: NET_REJOIN_OK");
        }
            break;
        case NET_REMOTE_LEAVE: {   //APP leave
            mode = 1;
            PR_ERR("Network state: NET_REMOTE_LEAVE");
            dev_load_attr_data();
            op_light_ctrl_data_auto_save();
            op_light_ctrl_data_switch_set(TRUE);
            op_light_ctrl_data_mode_set((LIGHT_MODE_E)mode);
            op_light_ctrl_proc();
            network_join_start();
            break;
        }
        case NET_LOCAL_LEAVE: {    //local leave
            PR_ERR("Network state: NET_LOCAL_LEAVE");
            break;
        }
        case NET_MF_TEST_LEAVE:{   //MF TEST leave
            PR_ERR("Network state: NET_MF_TEST_LEAVE");
            break;
        }

        default: {
            break;
        }
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
MF_TEST_RET_T dev_mf_test_callback(MF_TEST_CMD_T cmd, UCHAR_T *args, USHORT_T arg_len)
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
    //TODO
    return;
}

/**
 * @description: device report inforamtion when first power on(if need),;
 * @param {type} none
 * @return: none
 */
void dev_power_on_report_info(void)
{
    UCHAR_T onoff;
    
    report_count_down_data(QOS_1, 0, 0);
    op_light_ctrl_data_switch_get(&onoff);
    report_onoff_value(QOS_1, 0, onoff);
    return;
}

/**
 * @description: device recovery to factury command handler
 * @param {type} none
 * @return: none
 */
void dev_recovery_factory(DEV_RESET_TYPE_T type)
{
    //TODO
    return;
}

/**
 * @description: device receive message callback
 * @param {*dev_msg} received message information
 * @return: ZCL_CMD_RET_T
 */
ZCL_CMD_RET_T dev_msg_recv_callback(dev_msg_t *dev_msg)
{
   attr_value_t *attr_list = dev_msg->data.attr_data.attr_value;
   ZIGBEE_CMD_T app_cmd_type = ZIGBEE_CMD_SINGLE;

   if ( dev_msg->msg_src == MSG_SRC_GW_UNICAST ||     //Gateway unicast
       dev_msg->msg_src == MSG_SRC_DEV_UNICAST) {     //Device unicast
		app_cmd_type = ZIGBEE_CMD_SINGLE;
       PR_ERR("receive single message");
   } else {
		app_cmd_type = ZIGBEE_CMD_GROUP;
       PR_ERR("receive group message");
   }
   cmd_control_shade_flag = 1;       //command ctrl
   switch(dev_msg->cluster) {
       case CLUSTER_ON_OFF_CLUSTER_ID:{
           onoff_cluster_handler(attr_list[0].cmd, attr_list, app_cmd_type);
           break;
       }
       case CLUSTER_LEVEL_CONTROL_CLUSTER_ID:{   
           level_cluster_handler(attr_list[0].cmd, attr_list, app_cmd_type);
           break;
       }
       case CLUSTER_COLOR_CONTROL_CLUSTER_ID:{
           color_control_cluster_handler(attr_list[0].cmd, attr_list, app_cmd_type);
           break;
       }
       default:
           return ZCL_CMD_RET_FAILED;
   }
   return ZCL_CMD_RET_SUCCESS;
}

/**
 * @description: device recall scene command, recovery the scene add status
 * @param {endpoint} the endpoint of scene recall command
 * @param {*in_data} the data need to be handled of the device status
 * @return: none
 */
void dev_scene_recall_callback(UCHAR_T endpoint, const scene_save_data_t *in_data)
{
    UCHAR_T onoff;
    UCHAR_T mode;
    UCHAR_T  mode_attr;

    uint32_t delay_ms = 0;
    OPERATE_RET ret = OPRT_OK;

    COLOR_HSV_T app_hsv;

    PR_ERR("RECALL SCENE DATA LEN = %d", in_data->data_len);
    PR_ERR("RECALL SCENE DATA: ");

    for(UCHAR_T i=0; i < in_data->data_len; i++) {
        PR_ERR("%d ",in_data->data[i]);
    }
    PR_ERR("\r\n");

    onoff = in_data->data[0];
    mode = in_data->data[1];
    app_hsv.Hue = (in_data->data[2] << 8) | in_data->data[3];
    app_hsv.Saturation = (in_data->data[4] << 8) | in_data->data[5];
    app_hsv.Value = (in_data->data[6] << 8) | in_data->data[7];

    delay_ms = 10000 + random_ms(10000);

    clear_recall_scene_report();

    op_light_ctrl_data_mode_get(&pre_recall_mode); //save mode, before change
    ret = op_light_ctrl_data_switch_set(onoff);

    if (onoff != TRUE) {
        dev_zigbee_write_attribute(1,\
                                    CLUSTER_ON_OFF_CLUSTER_ID,\
                                    ATTR_ON_OFF_ATTRIBUTE_ID,\
                                    &onoff,\
                                    ATTR_INT8U_ATTRIBUTE_TYPE);
        ret |= op_set_onoff_with_countdown_cmd(onoff, QOS_1, delay_ms); //here will determine whether it is necessary to report onoff
        if (ret != OPRT_OK) { //It is already in the off state, and wake up another off state, do nothing
            return;
        }
    } else {
        //Wake up a light-on state
        dev_zigbee_write_attribute(1,\
                                    CLUSTER_ON_OFF_CLUSTER_ID,\
                                    ATTR_ON_OFF_ATTRIBUTE_ID,\
                                    &onoff,\
                                    ATTR_INT8U_ATTRIBUTE_TYPE);

        op_set_onoff_with_countdown_cmd(onoff, QOS_1, delay_ms); //here will determine whether it is necessary to report onoff

        if (in_data->data_len >= 16) {
            switch(mode) {
              case COLOR_MODE:{
                  if (mode == COLOR_MODE_HSV) {
                      mode_attr = 0x00;
                      op_light_ctrl_data_hsv_set(&app_hsv);
                  } else {
                      mode_attr = 0x01;
                  }
                  op_light_ctrl_data_mode_set(COLOR_MODE);
                  break;
              }
            default:
                  break;
            }
        }
        else
        {
            mode_attr = 0x01;
            op_light_ctrl_data_mode_set(COLOR_MODE);
        }
        //Update standard mode attributes
        dev_zigbee_write_attribute(1,\
                                    CLUSTER_COLOR_CONTROL_CLUSTER_ID,\
                                    ATTR_COLOR_CONTROL_COLOR_MODE_ATTRIBUTE_ID,\
                                    &mode_attr,\
                                    ATTR_ENUM8_ATTRIBUTE_TYPE);
    }

    light_data_move_complete_cb();
    dev_timer_start_with_callback(APP_CMD_HANDLE_DELAY_TIMER_ID, APP_CMD_HANDLE_DELAY_TIME, (timer_func_t)app_cmd_handle_delay_timer_cb);

    return;
}

/**
 * @description: device scene add command, store the current status to the scene table
 * @param {endpoint} the endpoint of scene add command
 * param {*out_data} the data need to be store to scene table
 * param {*in_out_len} the data length of the store data
 * @return: none
 */
void dev_scene_add_callback(UCHAR_T endpoint, UCHAR_T *out_data, UCHAR_T *in_out_len)
{
    UCHAR_T onoff;
    LIGHT_MODE_E mode;
    COLOR_MODE_E color_mode;
    COLOR_HSV_T hsv;
    UCHAR_T index = 0;

    op_light_ctrl_data_switch_get(&onoff);
    op_light_ctrl_data_mode_get(&mode);
    op_light_ctrl_data_color_mode_get(&color_mode);
    op_light_ctrl_data_hsv_get(&hsv);


    out_data[index++] = onoff;
    out_data[index++] = mode;
    out_data[index++] = color_mode;

    out_data[index++] = hsv.Hue >> 8;
    out_data[index++] = hsv.Hue;

    out_data[index++] = hsv.Saturation >> 8;
    out_data[index++] = hsv.Saturation;

    out_data[index++] = hsv.Value >> 8;
    out_data[index++] = hsv.Value;

    *in_out_len = index;
/*
    memcpy(out_data+1, &app_data, sizeof(app_data_t));
    *in_out_len = 1 + sizeof(app_data_t);
*/
    // PR_ERR("ADD SCENE DATA LEN = 0x%x", *in_out_len);
    // PR_ERR("ADD SCENE DATA : ");
    // for(UCHAR_T i=0;i<(*in_out_len);i++)
    // {
    //     PR_ERR("0x%x ",out_data[i]);
    // }
    // PR_ERR("\r\n");
    return;
}

/**
 * @description: write attribute callback function
 * @param {endpoint} the endpoint of the write attribute
 * @param {cluster} the cluster id of the write attribute
 * @param {attr_id} the attribute id  of the write attribute
 * @return: none
 */
void dev_msg_write_attr_callback(UCHAR_T endpoint, CLUSTER_ID_T cluster, USHORT_T attr_id)
{
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

    if (MODULE_NAME == TYZS2R)
    {
        user_uart_config_t default_config = TYZS2R_USART_CONFIG_DEFAULT;
        memcpy(&config, &default_config, sizeof(user_uart_config_t));
    }
    else if (MODULE_NAME == TYZS5)
    {
        user_uart_config_t default_config = TYZS5_USART_CONFIG_DEFAULT;
        memcpy(&config, &default_config, sizeof(user_uart_config_t));
    }
    else if (MODULE_NAME == ZS3L)
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
