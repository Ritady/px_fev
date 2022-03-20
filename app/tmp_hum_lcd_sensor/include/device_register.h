/*
 * @file: device_register.h
 * @brief: 
 * @author: Arien
 * @date: 2021-10-13 20:00:00
 * @email: Arien.ye@tuya.com
 * @copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @company: http://www.tuya.com
 */
#ifndef __DEVICE_REGISTER_H__
#define __DEVICE_REGISTER_H__

#include "config.h"


#ifdef __cplusplus
extern "C" {
#endif
             
#define ZIG_DEV_DESC_MODEL(a) \
    {1, ZHA_PROFILE_ID, ZG_DEVICE_ID_TEMPERATURE_SENSOR, get_array_len((a)), (cluster_t *)&((a)[0]), 0, NULL},


/* event delay time config! */
#define POWER_UP_LED_ON_TIME            3000
#define SENSOR_TIM_START_DELAY_TIME     3500        
#define NWK_JOIN_LED_BLINK_FREQ         250
#define NO_DELAY_TIME                   0

/* join parameter config! */
#define RESET_TIME_MS                   3000
#define CHANNEL_SW_PER_MS               300
#define JOINED_CONTINUE_POLL_TIME_MS    25000
#define POWER_ON_JOIN                   0
#define RM_LEAVE_JOIN                   1
#define JOIN_TIME_OUT_MS                30000

/* rejoin parameter config! */
#define POWER_ON_REJOIN                 1
#define AUTO_REJOIN_POLL                1
#define REJOINED_CONTINUE_POLL_TIME_MS  4000
#define BEACON_PER_MS                   1000
#define BEACON_TIME                     4
#define NEXT_REJOIN_PER_HOUR            4

/* poll parameter config! */
#define POLL_FOREVER                    0
#define POLL_INTERVAL_MS                1000
#define WAIT_APP_ACK_MS                 4000
#define POLL_MISS_MAX                   6
    
/* ep info parameter config! */
#define DEV_EP_SUM                      1
#define TMP_HUM_EP                      1

/**
 * @description: zigbee device init
 * @param {type} none
 * @return: none
 */
void zg_dev_init(void);




#ifdef __cplusplus
}
#endif
#endif // !__DEVICE_REGISTER_H__