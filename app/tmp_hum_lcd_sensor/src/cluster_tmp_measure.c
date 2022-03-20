/*
 * @file: cluster_tmp_measure.c
 * @brief: 
 * @author: Arien
 * @date: 2021-10-13 20:00:00
 * @email: Arien.ye@tuya.com
 * @copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @company: http://www.tuya.com
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tuya_zigbee_sdk.h"
#include "cluster_tmp_measure.h"


tmp_hum_data_t sensor_data;

/**
 * @description: temperature measurement attr report with QOS_1
 * @param {type} none
 * @return: none
 */
bool_t temperature_measurement_attr_report(uint8_t ep, int16_t temp_value)
{
    dev_send_data_t send_data;

    // if joined not tuya gateway will be report data from report table
    if (ZG_JOIN_GW_TYPE_TUYA != zg_get_join_gw_type()) {
        return TRUE;
    }
    
    memset(&send_data, 0, sizeof(dev_send_data_t));
    
    send_data.zcl_id = 0;
    send_data.qos = QOS_1;
    send_data.direction = ZCL_DATA_DIRECTION_SERVER_TO_CLIENT;
    send_data.command_id = CMD_REPORT_ATTRIBUTES_COMMAND_ID;
    send_data.addr.mode = SEND_MODE_GW;
    send_data.addr.type.gw.cluster_id = CLUSTER_TEMP_MEASUREMENT_CLUSTER_ID;
    send_data.addr.type.gw.src_ep = ep;
    send_data.delay_time = 0;
    send_data.random_time = 0;

    send_data.data.zg.attr_sum = 1;
    send_data.data.zg.attr[0].attr_id = ATTR_CURRENT_TEMPERATURE_ATTRIBUTE_ID ;
    send_data.data.zg.attr[0].type = ATTR_INT16S_ATTRIBUTE_TYPE;
    send_data.data.zg.attr[0].value_size = 2;
    send_data.data.zg.attr[0].value[0] = (uint8_t)temp_value;
    send_data.data.zg.attr[0].value[1] = temp_value >> 8;
    dev_zigbee_send_data(&send_data, NULL, 3000);
    
    return TRUE;
}

/**
 * @description: storage temperature data
 * @param {in} ep: endpoint
 * @param {in} tmp_milliC: temperature*1000
 * @return {out} none
 */
void temperature_data_storage(uint8_t ep, int32_t tmp_milliC)
{
    int16_t temperature_hundC = tmp_milliC / 10;
    int16_t temperature_limitC;
    dev_zigbee_write_attribute(ep,
                                CLUSTER_TEMP_MEASUREMENT_CLUSTER_ID,
                                ATTR_CURRENT_TEMPERATURE_ATTRIBUTE_ID,
                                (uint8_t *) &temperature_hundC,
                                ATTR_INT16S_ATTRIBUTE_TYPE);

    /* determine if this is a new minimum measured temperature, and update the
       TEMP_MIN_MEASURED attribute if that is the case. */
    dev_zigbee_read_attribute(ep,
                                CLUSTER_TEMP_MEASUREMENT_CLUSTER_ID,
                                ATTR_MIN_TEMP_EXPERIENCED_ATTRIBUTE_ID,
                                (uint8_t *) (&temperature_limitC),
                                sizeof(int16_t) );

    if ((temperature_limitC == TEMPERATURE_SANITY_CHECK)
         || (temperature_limitC > temperature_hundC)) {
        dev_zigbee_write_attribute(ep,
                                   CLUSTER_TEMP_MEASUREMENT_CLUSTER_ID,
                                   ATTR_MIN_TEMP_EXPERIENCED_ATTRIBUTE_ID,
                                   (uint8_t *) &temperature_hundC,
                                   ATTR_INT16S_ATTRIBUTE_TYPE);
     }

    /* Determine if this is a new maximum measured temperature, and update the
       TEMP_MAX_MEASURED attribute if that is the case. */
    dev_zigbee_read_attribute(ep,
                                CLUSTER_TEMP_MEASUREMENT_CLUSTER_ID,
                                ATTR_MAX_TEMP_EXPERIENCED_ATTRIBUTE_ID,
                                (uint8_t *) (&temperature_limitC),
                                sizeof(int16_t) );
    
    if ((temperature_limitC == TEMPERATURE_SANITY_CHECK)
         || (temperature_limitC < temperature_hundC)) {
        dev_zigbee_write_attribute(ep,
                                   CLUSTER_TEMP_MEASUREMENT_CLUSTER_ID,
                                   ATTR_MAX_TEMP_EXPERIENCED_ATTRIBUTE_ID,
                                   (uint8_t *) &temperature_hundC,
                                   ATTR_INT16S_ATTRIBUTE_TYPE);
    }
}

/**
 * @description: cycle timing callback use for force report
 * @param {in} t_id: timer id
 * @return {type} none
 */
void sensor_report_according_to_cycles_tmr(uint8_t t_id)
{    
    sht40_data_t data = {0};
    
    sensor_data.last_hum = sensor_data.curr_hum;
    sensor_data.last_temp = sensor_data.curr_temp;

    NET_EVT_T nwk_status = nwk_state_get();
    if( NET_REJOIN_OK == nwk_status ||
        NET_JOIN_OK == nwk_status || 
        NET_LOST == nwk_status) {

        data.temp_value = sensor_data.curr_temp / 10;
        data.hum_value = sensor_data.curr_hum / 10;

        if (tmp_hum->report_f != NULL) {
            tmp_hum->report_f(tmp_hum->endpoint, &data, ALL_DATA_MASK);
        }

        dev_timer_start_with_callback(tmp_hum->timer.cycler_read_tmr_id, 
                                        tmp_hum->report_s * 1000UL, 
                                        sensor_read_according_to_cycles_tmr);
    }
}

/**
 * @description: cycle timing callback use for force report sampling
 * @param {in} t_id: timer id
 * @return {type} none
 */
void sensor_read_according_to_cycles_tmr(uint8_t t_id)
{
    ty_sht40_start_read();

    if (ZG_JOIN_GW_TYPE_TUYA == zg_get_join_gw_type()) {
        /* wait for data reading to complete */
        dev_timer_start_with_callback(tmp_hum->timer.cycler_rept_tmr_id, 
                                        SENSOR_DELAY_HANDLE_MS, 
                                        sensor_report_according_to_cycles_tmr);
    }
}
