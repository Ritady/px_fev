/*
 * @file: cluster_tmp_measure.h
 * @brief: 
 * @author: Arien
 * @date: 2021-10-13 20:00:00
 * @email: Arien.ye@tuya.com
 * @copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @company: http://www.tuya.com
 */
#ifndef __CLUSTER_TMP_MEASURE_H__
#define __CLUSTER_TMP_MEASURE_H__
#ifdef __cplusplus
extern "C" {
#endif
#include <stdbool.h>
#include <stdint.h>
#include "driver_sht40.h"

/* temperature limit(Multiply by 1000) */
#define TMP_MAX_MILLI                                           (60*1000)
#define TMP_MIN_MILLI                                           (-20*1000)

/* macro used to ensure sane temperature max/min values are stored */
#define TEMPERATURE_SANITY_CHECK    0
#define SENSOR_DELAY_HANDLE_MS      250


/**
 * sensor data struct
 *
 * curr_hum:  current humidity
 * last_hum:  last humidity
 * curr_temp: current temperature
 * last_temp: last temperature
 */
typedef struct{
    uint16_t curr_hum;
    uint16_t last_hum;
    int32_t  curr_temp;
    int32_t  last_temp;
}tmp_hum_data_t;

extern tmp_hum_data_t sensor_data;

/**
 * @description: temperature measurement attr report with QOS_1
 * @param {type} none
 * @return: none
 */
bool_t temperature_measurement_attr_report(uint8_t ep, int16_t temp_value);

/**
 * @description: storage temperature data
 * @param {in} ep: endpoint
 * @param {in} tmp_milliC: temperature*1000
 * @return {out} none
 */
void temperature_data_storage(uint8_t ep, int32_t tmp_milliC);

/**
 * @description: cycle timing callback use for force report
 * @param {in} t_id: timer id
 * @return {type} none
 */
void sensor_report_according_to_cycles_tmr(uint8_t t_id);

/**
 * @description: cycle timing callback use for force report sampling
 * @param {in} t_id: timer id
 * @return {type} none
 */
void sensor_read_according_to_cycles_tmr(uint8_t t_id);





#ifdef __cplusplus
}
#endif
#endif