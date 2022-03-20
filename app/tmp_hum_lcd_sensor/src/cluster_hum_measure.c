/*
 * @file: cluster_hum_measure.c
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
#include "cluster_hum_measure.h"
#include "cluster_tmp_measure.h"
/**
 * @description: humidity measurement attr report with QOS_1
 * @param {type} none
 * @return: none
 */
bool_t humidity_measurement_attr_report(uint8_t ep, uint16_t hum_value)
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
    send_data.addr.type.gw.cluster_id = CLUSTER_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_ID;
    send_data.addr.type.gw.src_ep = ep;
    send_data.delay_time = 0;
    send_data.random_time = 0;

    send_data.data.zg.attr_sum = 1;
    send_data.data.zg.attr[0].attr_id = ATTR_RELATIVE_HUMIDITY_ATTRIBUTE_ID;
    send_data.data.zg.attr[0].type = ATTR_INT16U_ATTRIBUTE_TYPE;
    send_data.data.zg.attr[0].value_size = 2;
    send_data.data.zg.attr[0].value[0] = (uint8_t)hum_value;
    send_data.data.zg.attr[0].value[1] = hum_value >> 8;
    dev_zigbee_send_data( &send_data, NULL, 3000);
    return TRUE;
}

/**
 * @description: storage humidity data
 * @param {in} ep: endpoint
 * @param {in} hum_percent: humidity*100
 * @return {out} none
 * @return: none
 */
void humidity_data_storage(uint8_t ep, uint16_t hum_percent)
{
    uint16_t humidity_limit_percent;
    hum_percent /= 10;
    dev_zigbee_write_attribute(ep,
                                CLUSTER_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_ID,
                                ATTR_RELATIVE_HUMIDITY_MEASURED_VALUE_ATTRIBUTE_ID,
                                (uint8_t *) &hum_percent,
                                ATTR_INT16U_ATTRIBUTE_TYPE);

    /* Determine if this is a new minimum measured humidity, and update the
       HUMIDITY_MIN_MEASURED attribute if that is the case. */
    dev_zigbee_read_attribute(ep,
                                CLUSTER_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_ID,
                                ATTR_RELATIVE_HUMIDITY_MIN_MEASURED_VALUE_ATTRIBUTE_ID,
                                (uint8_t *) (&humidity_limit_percent),
                                sizeof(humidity_limit_percent) );

    if ((humidity_limit_percent == HUMIDITY_SANITY_CHECK)
         || (humidity_limit_percent > hum_percent)) {
        dev_zigbee_write_attribute(ep,
                                    CLUSTER_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_ID,
                                    ATTR_RELATIVE_HUMIDITY_MIN_MEASURED_VALUE_ATTRIBUTE_ID,
                                    (uint8_t *) &hum_percent,
                                    ATTR_INT16U_ATTRIBUTE_TYPE);
    }

    /* Determine if this is a new maximum measured humidity, and update the
       HUMIDITY_MAX_MEASURED attribute if that is the case. */
    dev_zigbee_read_attribute(ep,
                                CLUSTER_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_ID,
                                ATTR_RELATIVE_HUMIDITY_MAX_MEASURED_VALUE_ATTRIBUTE_ID,
                                (uint8_t *) (&humidity_limit_percent),
                                sizeof(humidity_limit_percent) );
    
    if ((humidity_limit_percent == HUMIDITY_SANITY_CHECK)
         || (humidity_limit_percent < hum_percent)) {
        dev_zigbee_write_attribute(ep,
                                    CLUSTER_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_ID,
                                    ATTR_RELATIVE_HUMIDITY_MAX_MEASURED_VALUE_ATTRIBUTE_ID,
                                    (uint8_t *) &hum_percent,
                                    ATTR_INT16U_ATTRIBUTE_TYPE);
    }
}

/**
 * @description: cycle timing callback use for threshold report sampling
 * @param {in} t_id: timer id
 * @return {type} none
 */
void sensor_read_according_to_threshold(uint8_t t_id)
{
    ty_sht40_start_read();

    /* wait for data reading to complete */
    dev_timer_start_with_callback(tmp_hum->timer.thrd_rept_tmr_id, 
                                    SENSOR_DELAY_HANDLE_MS, 
                                    sensor_report_according_to_threshold);
}

/**
 * @description: cycle timing callback use for threshold report
 * @param {in} t_id: timer id
 * @return {type} none
 */
void sensor_report_according_to_threshold(uint8_t t_id)
{
    uint8_t mask = 0;
    sht40_data_t data = {0};

    if (sensor_data.last_temp + tmp_hum->tmp_tld * 100 < sensor_data.curr_temp ||
        sensor_data.last_temp - tmp_hum->tmp_tld * 100 > sensor_data.curr_temp) {
        
        sensor_data.last_temp = sensor_data.curr_temp;
        mask |= TMP_DATA_MASK;
    }

    if(sensor_data.last_hum + tmp_hum->hum_tld * 1000 < sensor_data.curr_hum ||
        sensor_data.last_hum - tmp_hum->hum_tld * 1000 > sensor_data.curr_hum) {

        sensor_data.last_hum = sensor_data.curr_hum;
        mask |= HUM_DATA_MASK;
    }
    
    NET_EVT_T nwk_status = nwk_state_get();
    if (NET_REJOIN_OK == nwk_status || NET_JOIN_OK == nwk_status) {

        data.temp_value = sensor_data.curr_temp / 10;
        data.hum_value = sensor_data.curr_hum / 10;

        if (tmp_hum->report_f != NULL) {
            tmp_hum->report_f(tmp_hum->endpoint, &data, mask);
        }

        dev_timer_start_with_callback(tmp_hum->timer.thrd_read_tmr_id, 
                                        tmp_hum->sample_s * 1000UL,
                                        sensor_read_according_to_threshold);
    }
}