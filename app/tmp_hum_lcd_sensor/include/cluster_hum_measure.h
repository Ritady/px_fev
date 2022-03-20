/*
 * @file: cluster_hum_measure.h
 * @brief: 
 * @author: Arien
 * @date: 2021-10-13 20:00:00
 * @email: Arien.ye@tuya.com
 * @copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @company: http://www.tuya.com
 */
#ifndef __CLUSTER_HUM_MEASURE_H__
#define __CLUSTER_HUM_MEASURE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include "cluster_tmp_measure.h"

/* humidity limit(Multiply by 100) */
#define HUM_MAX_HUN                                             (100*1000)
#define HUM_MIN_HUN                                             0

/* macro used to ensure sane humidity max/min values are stored */
#define HUMIDITY_SANITY_CHECK                                   0

/**
 * @description: humidity measurement attr report with QOS_1
 * @param {type} none
 * @return: none
 */
bool_t humidity_measurement_attr_report(uint8_t ep, uint16_t hum_value);

/**
 * @description: storage humidity data
 * @param {in} ep: endpoint
 * @param {in} hum_percent: humidity*100
 * @return {out} none
 * @return: none
 */
void humidity_data_storage(uint8_t ep, uint16_t hum_percent);

/**
 * @description: cycle timing callback use for threshold report sampling
 * @param {in} t_id: timer id
 * @return {type} none
 */
void sensor_read_according_to_threshold(uint8_t t_id);

/**
 * @description: cycle timing callback use for threshold report
 * @param {in} t_id: timer id
 * @return {type} none
 */
void sensor_report_according_to_threshold(uint8_t t_id);



#ifdef __cplusplus
}
#endif
#endif