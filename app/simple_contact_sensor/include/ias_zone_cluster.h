/**
* @file ias_zone_cluster.h
* @brief ias_zone_cluster module is used to ias_zone_cluster relevant
* @version 1.0
* @date 2021-08-31
*
* @copyright Copyright (c) tuya.inc 2019
* 
*/
#ifndef __IAS_ZONE_CLUSTER_H__
#define __IAS_ZONE_CLUSTER_H__

#include "tuya_zigbee_sdk.h"

#ifdef __cplusplus
extern "C" {
#endif
  
#define IAS_ZONE_INVALID_ZONE_ID                0XFF
#define IAS_ZONE_SENSOR_STATUS_ALARM1           0x0001 
#define IAS_ZONE_SENSOR_STATUS_NO_ALARM1        0x0000
#define IAS_ZONE_SENSOR_STATUS_ALARM2           0x0002
#define IAS_ZONE_SENSOR_STATUS_NO_ALARM2        0x0000
#define IAS_ZONE_SENSOR_STATUS_TAMPER           0x0004
#define IAS_ZONE_SENSOR_STATUS_NO_TAMPER        0x0000
#define IAS_ZONE_SENSOR_STATUS_TROUBLE          0x0040
#define IAS_ZONE_SENSOR_STATUS_NO_TROUBLE       0x0000  
#define IAS_ZONE_ENDPOINT                       0x01  
#define IAS_ZONE_STATUS_BIT_ACTIVE              0x01
#define IAS_ZONE_STATUS_BIT_INACTIVE            0x00

/* zoneinfo parameter config! */
#define IAS_ZONE_TYPE                           21
#define IAS_ZONE_EP                             1
  
/* alarm1info parameter config! */
#define ALARM1_LED_FLASH_TIME                   200
#define ALARM1_GPIO_POLARITY                    0
#define ALARM1_VIBRATE_TIME                     200
#define ALARM1_REPORT_TIME                      0

/* tamperinfo parameter config! */
#define TAMPER_LED_FLASH_TIME                   200
#define TAMPER_GPIO_POLARITY                    0
#define TAMPER_VIBRATE_TIME                     200
#define TAMPER_REPORT_TIME                      0
  
typedef union {
    uint16_t ias_zone_status;
    struct {
        uint16_t alarm1_status_bit0:        1;
        uint16_t alarm2_status_bit1:        1;
        uint16_t tamper_status_bit2:        1;
        uint16_t battery_status_bit3:       1;
        uint16_t supervision_status_bit4:   1;
        uint16_t restore_status_bit5:       1;
        uint16_t trouble_status_bit6:       1;
        uint16_t ac_mains_status_bit7:      1;
    }zone_bits;
}ias_zone_status;

typedef enum {
  IAS_ENROLL_RESPONSE_CODE_SUCCESS          = 0x00,
  IAS_ENROLL_RESPONSE_CODE_NOT_SUPPORTED    = 0x01,
  IAS_ENROLL_RESPONSE_CODE_NO_ENROLL_PERMIT = 0x02,
  IAS_ENROLL_RESPONSE_CODE_TOO_MANY_ZONES   = 0x03,
} iasenrollresponsecode;

typedef enum {
  IAS_ZONE_STATE_NOT_ENROLLED = 0x00,
  IAS_ZONE_STATE_ENROLLED     = 0x01,
} iaszonestate;

extern ias_zone_status ias_zone_status_bits;


/**
 * @description: ias zone info init
 * @param {in} ep: endpoind
 * @param {in} zone_type: ias zone type
 * @return: none
 */
void ias_zone_info_init(uint8_t ep, uint16_t zone_type);

/**
 * @description: ias zone alarm1 gpio interrupt
 * @param {type} none
 * @return: none
 */
void gpio_sensor_alarm1_interrupt_handler(GPIO_PORT_T port, GPIO_PIN_T pin);

/**
 * @description: ias zone tamper gpio interrupt
 * @param {type} none
 * @return: none
 */
void gpio_sensor_tamper_interrupt_handler(GPIO_PORT_T port, GPIO_PIN_T pin);

/**
 * @description: updata ias zone status
 * @param {type} none
 * @return: none
 */
void ias_zone_status_update(void);

/**
 * @description: get ias zone id 
 * @param {in} ep: endpoind
 * @return: none
 */
uint8_t get_ias_zone_id(uint8_t ep);

/**
 * @description: send enroll request func
 * @param {type} none
 * @return: none
 */
void zig_ias_enroll_request_with_client(void);

/**
 * @description: ias zone sensor basic  gpio init
 * @param {type} none
 * @return: none
 */
void sensor_basic_init_io_status_updata(void);

/**
 * @description: ias zone info to init status when stack change to no nwk 
 * @param {in} nwk_status : zigbee network status
 * @return: none
 */
void ias_zone_off_net_status_reset(uint8_t nwk_status);

/**
 * @description: unenroll to clear ias zone info
 * @param {in} ep: endpoind
 * @return: none
 */
void ias_zone_unenroll(uint8_t ep);

/**
 * @description: set ias zone id after receive enroll response
 * @param {in} ep: endpoind
 * @param {in} zoneid: ias_zone id
 * @return: none
 */
static void set_ias_zone_id(uint8_t ep,uint8_t zone_id);

/**
 * @description: set ias zone state after receive enroll response
 * @param {in} ep: endpoind
 * @param {in} enroll_state: enroll state
 * @return: none
 */
static void set_ias_zone_enroll_state(uint8_t ep, uint8_t enroll_state);

/**
 * @description: when joined nwk or rejoined nwk will be sync ias zone status 
 * @param {type} none
 * @return: none
 */
void ias_sensor_status_sync(void);

/**
 * @description: receive ias zone cluster callback
 * @param {in} dev_msg：receive message
 * @return: none
 */
void ias_zone_cluster_handler(dev_msg_t *dev_msg);

/**
 * @description: receive write attribute ias zone cluster callback
 * @param {in} attr_id: attribute ID
 * @return: none
 */
void ias_zone_write_attr_handler(uint16_t attr_id);


/**
 * @description: get ias zone state
 * @param {in} ep: endpoind
 * @return: none
 */
uint8_t get_ias_zone_enroll_state(uint8_t ep);

/**
 * @description: ias zone sensor status pre-report callback
 * @param {in} status: ias zone status
 * @return: none
 */
bool_t ias_zone_status_pre_update_callback(uint16_t status);




#ifdef __cplusplus
}
#endif
#endif
