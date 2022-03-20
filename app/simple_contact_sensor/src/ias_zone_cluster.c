/*
 * @file: ias_zone_cluster.c
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
#include "app_common.h"
#include "ias_zone_cluster.h"

// ias zone status union 
ias_zone_status ias_zone_status_bits;


/**
 * @description: ias zone info init
 * @param {in} ep: endpoind
 * @param {in} zone_type: ias zone type
 * @return: none
 */
void ias_zone_info_init(uint8_t ep, uint16_t zone_type)
{
    uint16_t ias_zone_status = 0;
    // write ias zone type
    dev_zigbee_write_attribute(
        ep,
        CLUSTER_IAS_ZONE_CLUSTER_ID,
        ATTR_ZONE_TYPE_ATTRIBUTE_ID,
        &zone_type,
        ATTR_ENUM16_ATTRIBUTE_TYPE
    );

    //write ias zone status
    dev_zigbee_write_attribute(
        ep,
        CLUSTER_IAS_ZONE_CLUSTER_ID,
        ATTR_ZONE_STATUS_ATTRIBUTE_ID,
        &ias_zone_status,
        ATTR_BITMAP16_ATTRIBUTE_TYPE
    );
}

/**
 * @description: ias zone alarm1 gpio interrupt
 * @param {type} none
 * @return: none
 */
void gpio_sensor_alarm1_interrupt_handler(GPIO_PORT_T port, GPIO_PIN_T pin)
{
    uint8_t alarm1_value;
    alarm1_value = gpio_raw_input_read_status(alarm_info.sensor_io_st.port, alarm_info.sensor_io_st.pin);
    if(alarm_info.gpio_polarity == alarm1_value) {
        if(GPIO_SENSOR_ACTIVE == (GPIO_STATE_T)alarm_info.last_status) {
            dev_timer_stop(GPIO_ALARM1_DEBOUNCE_EVT_ID);
            return ;
        }
        alarm_info.new_status = (DEV_IO_ST_T)GPIO_SENSOR_ACTIVE;
        dev_timer_start_with_callback(GPIO_ALARM1_DEBOUNCE_EVT_ID, ALARM1_VIBRATE_TIME, gpio_sensor_debounce_handler);
    } else {
        if(GPIO_SENSOR_INACTIVE == (GPIO_STATE_T)alarm_info.last_status) {
            dev_timer_stop(GPIO_ALARM1_DEBOUNCE_EVT_ID);
            return ;
        }
        alarm_info.new_status = (DEV_IO_ST_T)GPIO_SENSOR_INACTIVE;
        dev_timer_start_with_callback(GPIO_ALARM1_DEBOUNCE_EVT_ID, ALARM1_VIBRATE_TIME, gpio_sensor_debounce_handler);
    }
}

/**
 * @description: ias zone tamper gpio interrupt
 * @param {type} none
 * @return: none
 */
void gpio_sensor_tamper_interrupt_handler(GPIO_PORT_T port, GPIO_PIN_T pin)
{
    uint8_t tamper_value;
    tamper_value = gpio_raw_input_read_status(tamper_info.sensor_io_st.port, tamper_info.sensor_io_st.pin);
    if(tamper_info.gpio_polarity == tamper_value) {
        if(GPIO_SENSOR_ACTIVE == (GPIO_STATE_T)tamper_info.last_status) {
            dev_timer_stop(GPIO_TAMPER_DEBOUNCE_EVT_ID);
            return ;
        }
        tamper_info.new_status = (DEV_IO_ST_T)GPIO_SENSOR_ACTIVE;
        dev_timer_start_with_callback(GPIO_TAMPER_DEBOUNCE_EVT_ID, TAMPER_VIBRATE_TIME, gpio_sensor_debounce_handler);
    } else {
        if(GPIO_SENSOR_INACTIVE == (GPIO_STATE_T)tamper_info.last_status) {
            dev_timer_stop(GPIO_TAMPER_DEBOUNCE_EVT_ID);
            return ;
        }
        tamper_info.new_status = (DEV_IO_ST_T)GPIO_SENSOR_INACTIVE;
        dev_timer_start_with_callback(GPIO_TAMPER_DEBOUNCE_EVT_ID, TAMPER_VIBRATE_TIME, gpio_sensor_debounce_handler);
    }
}

/**
 * @description: updata ias zone status
 * @param {type} none
 * @return: none
 */
void ias_zone_status_update(void)
{
    // update status only if conditions are met
    if(ias_zone_status_pre_update_callback(ias_zone_status_bits.ias_zone_status)) {
        NET_EVT_T nwk_status = nwk_state_get();
        tuya_print("ias_zone_status_update %d\r\n", ias_zone_status_bits.ias_zone_status);
        /* send enroll request only nwk up  */
        if((nwk_status == NET_JOIN_OK) || (nwk_status == NET_REJOIN_OK)) {
            if (get_ias_zone_enroll_state(IAS_ZONE_ENDPOINT) == IAS_ZONE_STATE_NOT_ENROLLED) {
                zig_ias_enroll_request_with_client();
            }
        }

        // IAS_STATUS attr report
        uint8_t i = 0;
        dev_send_data_t send_data;

        memset(&send_data, 0, sizeof(dev_send_data_t));

        send_data.qos = QOS_1;
        send_data.direction = ZCL_DATA_DIRECTION_SERVER_TO_CLIENT;
        send_data.command_id = CMD_ZONE_STATUS_CHANGE_NOTIFICATION_COMMAND_ID;
        send_data.commmand_type = ZCL_COMMAND_CLUSTER_SPEC_CMD;
        send_data.addr.mode = SEND_MODE_GW;
        send_data.addr.type.gw.cluster_id = CLUSTER_IAS_ZONE_CLUSTER_ID;
        send_data.addr.type.gw.src_ep = IAS_ZONE_ENDPOINT;
        send_data.delay_time = 0;
        send_data.random_time = 0;
        send_data.data.private.data[i++] = ias_zone_status_bits.ias_zone_status;
        send_data.data.private.data[i++] = ias_zone_status_bits.ias_zone_status >> 8;
        send_data.data.private.data[i++] = 0;
        send_data.data.private.data[i++] = get_ias_zone_id(1);
        send_data.data.private.data[i++] = 0;
        send_data.data.private.data[i++] = 0;
        send_data.data.private.len = i;
        dev_zigbee_send_data(&send_data, NULL, 3000);
    }
}

/**
 * @description: get ias zone id 
 * @param {in} ep: endpoind
 * @return: none
 */
uint8_t get_ias_zone_id(uint8_t ep)
{
    uint8_t ias_zone_id;

    dev_zigbee_read_attribute(
                ep,
                CLUSTER_IAS_ZONE_CLUSTER_ID,
                ATTR_ZONE_ID_ATTRIBUTE_ID,
                &ias_zone_id,
                sizeof(ias_zone_id)
            );
    
    return ias_zone_id;
}

/**
 * @description: send enroll request func
 * @param {type} none
 * @return: none
 */
void zig_ias_enroll_request_with_client(void)
{
    dev_send_data_t send_data;
    uint16_t ias_zone_type = IAS_ZONE_TYPE;
    uint8_t i = 0;
    memset( &send_data, 0, sizeof(dev_send_data_t) );

    send_data.qos = QOS_1;
    send_data.direction = ZCL_DATA_DIRECTION_SERVER_TO_CLIENT;
    send_data.command_id = CMD_ZONE_ENROLL_REQUEST_COMMAND_ID;
    send_data.commmand_type = ZCL_COMMAND_CLUSTER_SPEC_CMD;
    send_data.addr.mode = SEND_MODE_GW;
    send_data.addr.type.gw.cluster_id = CLUSTER_IAS_ZONE_CLUSTER_ID;
    send_data.addr.type.gw.src_ep = 1;
    send_data.delay_time = 0;
    send_data.random_time = 0;
    send_data.data.private.data[i++] = (uint8_t)ias_zone_type;
    send_data.data.private.data[i++] = ias_zone_type >> 8;
    send_data.data.private.data[i++] = (uint8_t)MANUFACTURER_CODE;
    send_data.data.private.data[i++] = MANUFACTURER_CODE >> 8;
    send_data.data.private.len = i;

    dev_zigbee_send_data(&send_data, NULL, 3000);             
}

/**
 * @description: ias zone sensor basic  gpio init
 * @param {type} none
 * @return: none
 */
void sensor_basic_init_io_status_updata(void)
{
    if((!sensor_init_io_status_set(&alarm_info)) || (!sensor_init_io_status_set(&tamper_info))) {
        return;
    }

    if(alarm_info.new_status == (DEV_IO_ST_T)GPIO_SENSOR_INACTIVE) {
        ias_zone_status_bits.zone_bits.alarm1_status_bit0 = IAS_ZONE_STATUS_BIT_INACTIVE;    
    } else {
        ias_zone_status_bits.zone_bits.alarm1_status_bit0 = IAS_ZONE_STATUS_BIT_ACTIVE;
    }
    if(tamper_info.new_status == (DEV_IO_ST_T)GPIO_SENSOR_INACTIVE) {
        ias_zone_status_bits.zone_bits.tamper_status_bit2 = IAS_ZONE_STATUS_BIT_INACTIVE;    
    } else {
        ias_zone_status_bits.zone_bits.tamper_status_bit2 = IAS_ZONE_STATUS_BIT_ACTIVE;
    }
    
    tuya_print("ias_zone_alarm1_pre_change %d\r\n", ias_zone_status_bits.zone_bits.alarm1_status_bit0);
    dev_led_stop_blink(1, DEV_IO_ON);
    dev_timer_start_with_callback(LED_ALARM1_FLASH_EVT_ID, ALARM1_LED_FLASH_TIME, dev_evt_callback);
    tuya_print("ias_zone_tamper_pre_change %d\r\n", ias_zone_status_bits.zone_bits.tamper_status_bit2);
    dev_led_stop_blink(2, DEV_IO_ON);
    dev_timer_start_with_callback(LED_TAMPER_FLASH_EVT_ID, TAMPER_LED_FLASH_TIME, dev_evt_callback);
    
    ias_zone_status_update();
}

/**
 * @description: ias zone info to init status when stack change to no nwk 
 * @param {in} nwk_status : zigbee network status
 * @return: none
 */
void ias_zone_off_net_status_reset(uint8_t nwk_status)
{
    // leave nwk must to unenroll
    if((nwk_status == NET_LOCAL_LEAVE) || (nwk_status == NET_REMOTE_LEAVE) || (nwk_status == NET_MF_TEST_LEAVE)) {
        ias_zone_unenroll(IAS_ZONE_EP);
    }
}

/**
 * @description: unenroll to clear ias zone info
 * @param {in} ep: endpoind
 * @return: none
 */
void ias_zone_unenroll(uint8_t ep)
{
    uint8_t ieee_address[] = {0, 0, 0, 0, 0, 0, 0, 0};
    uint16_t ias_zone_type = IAS_ZONE_TYPE;

    dev_zigbee_write_attribute(
        ep,
        CLUSTER_IAS_ZONE_CLUSTER_ID,
        ATTR_IAS_CIE_ADDRESS_ATTRIBUTE_ID,
        (uint8_t *)ieee_address,
        ATTR_IEEE_ADDRESS_ATTRIBUTE_TYPE
    );

   dev_zigbee_write_attribute(
        ep,
        CLUSTER_IAS_ZONE_CLUSTER_ID,
        ATTR_ZONE_TYPE_ATTRIBUTE_ID,
        &ias_zone_type,
        ATTR_ENUM16_ATTRIBUTE_TYPE
    );

    set_ias_zone_id(ep, IAS_ZONE_INVALID_ZONE_ID);
    set_ias_zone_enroll_state(ep, IAS_ZONE_STATE_NOT_ENROLLED);
}

/**
 * @description: set ias zone id after receive enroll response
 * @param {in} ep: endpoind
 * @param {in} zoneid: ias_zone id
 * @return: none
 */
static void set_ias_zone_id(uint8_t ep,uint8_t zone_id)
{
    // tuya_print("ias zone id write %d\r\n",zoneid);
    dev_zigbee_write_attribute(
        ep,
        CLUSTER_IAS_ZONE_CLUSTER_ID,
        ATTR_ZONE_ID_ATTRIBUTE_ID,
        &zone_id,
        ATTR_INT8U_ATTRIBUTE_TYPE
    );
}

/**
 * @description: set ias zone state after receive enroll response
 * @param {in} ep: endpoind
 * @param {in} enroll_state: enroll state
 * @return: none
 */
static void set_ias_zone_enroll_state(uint8_t ep, uint8_t enroll_state)
{
    // tuya_print(" ias state write %d\r\n",enrollstate);
    dev_zigbee_write_attribute(
        ep,
        CLUSTER_IAS_ZONE_CLUSTER_ID,
        ATTR_ZONE_STATE_ATTRIBUTE_ID,
        &enroll_state,
        ATTR_ENUM8_ATTRIBUTE_TYPE
    );
}

/**
 * @description: when joined nwk or rejoined nwk will be sync ias zone status 
 * @param {type} none
 * @return: none
 */
void ias_sensor_status_sync(void)
{
    uint8_t value;
    value = gpio_raw_input_read_status(alarm_info.sensor_io_st.port, alarm_info.sensor_io_st.pin);
    if(alarm_info.gpio_polarity == value) {
        ias_zone_status_bits.ias_zone_status |= IAS_ZONE_SENSOR_STATUS_ALARM1;
    } else {
        ias_zone_status_bits.ias_zone_status &= (~IAS_ZONE_SENSOR_STATUS_ALARM1);
    }

    value = gpio_raw_input_read_status(tamper_info.sensor_io_st.port, tamper_info.sensor_io_st.pin);
    if(tamper_info.gpio_polarity == value) {
        ias_zone_status_bits.ias_zone_status |= IAS_ZONE_SENSOR_STATUS_TAMPER;
    } else {
        ias_zone_status_bits.ias_zone_status &= (~IAS_ZONE_SENSOR_STATUS_TAMPER);
    }

    dev_zigbee_write_attribute(
        IAS_ZONE_EP,
        CLUSTER_IAS_ZONE_CLUSTER_ID,
        ATTR_ZONE_STATUS_ATTRIBUTE_ID,
        &ias_zone_status_bits.ias_zone_status,
        ATTR_BITMAP16_ATTRIBUTE_TYPE
    );
}


/**
 * @description: receive ias zone cluster callback
 * @param {in} dev_msg：receive message
 * @return: none
 */
void ias_zone_cluster_handler(dev_msg_t *dev_msg)
{
    attr_value_t *attr_list = dev_msg -> data.attr_data.attr_value;
    uint8_t ep = dev_msg -> endpoint;
    switch(attr_list[0].cmd) {
        case CMD_ZONE_ENROLL_RESPONSE_COMMAND_ID: {
            dev_zigbee_specific_response_handle(dev_msg -> zcl_id, dev_msg -> endpoint, dev_msg -> cluster);
            // tuya_print("ias zone enroll response\r\n");
            if(attr_list[0].value[0] == IAS_ENROLL_RESPONSE_CODE_SUCCESS) {            //enroll response sucess
                set_ias_zone_enroll_state(ep, IAS_ZONE_STATE_ENROLLED);   //write ias zone state enrolled
                set_ias_zone_id(ep, attr_list[0].value[1]);                                 //write valid zone id
            } else {
                set_ias_zone_enroll_state(ep, IAS_ZONE_STATE_NOT_ENROLLED);
                set_ias_zone_id(ep, IAS_ZONE_INVALID_ZONE_ID);
            }
        }
        break;
        case CMD_INITIATE_NORMAL_OPERATION_MODE_COMMAND_ID: {
            //TODO
        }
        break;
        case CMD_INITIATE_TEST_MODE_COMMAND_ID: {
            //TODO
        }
        break;
        default: 
        break;
    }
}

/**
 * @description: receive write attribute ias zone cluster callback
 * @param {in} attr_id: attribute ID
 * @return: none
 */
void ias_zone_write_attr_handler(uint16_t attr_id)
{
    switch(attr_id) {
        case ATTR_IAS_CIE_ADDRESS_ATTRIBUTE_ID: {
            if(get_ias_zone_enroll_state(IAS_ZONE_EP) == IAS_ZONE_STATE_NOT_ENROLLED) {
                zig_ias_enroll_request_with_client();
            }
        }
        break;
        default: 
        break;
    }
}

/**
 * @description: get ias zone state
 * @param {in} ep: endpoind
 * @return: none
 */
uint8_t get_ias_zone_enroll_state(uint8_t ep)
{
    uint8_t zone_sate = IAS_ZONE_STATE_NOT_ENROLLED;
    dev_zigbee_read_attribute(
                ep,
                CLUSTER_IAS_ZONE_CLUSTER_ID,
                ATTR_ZONE_STATE_ATTRIBUTE_ID,
                &zone_sate,
                sizeof(zone_sate)
            );
    // tuya_print("ias zone enroll status get %d\r\n",zone_sate);
    return zone_sate;
}

/**
 * @description: ias zone sensor status pre-report callback
 * @param {in} status: ias zone status
 * @return: none
 */
bool_t ias_zone_status_pre_update_callback(uint16_t status)
{
    dev_zigbee_write_attribute(
        IAS_ZONE_ENDPOINT,
        CLUSTER_IAS_ZONE_CLUSTER_ID,
        ATTR_ZONE_STATUS_ATTRIBUTE_ID,
        &status,
        ATTR_BITMAP16_ATTRIBUTE_TYPE
    );
    return TRUE;
}
