/*
 * @Author: xiaojia
 * @email: limu.xiao@tuya.com
 * @LastEditors:
 * @file name: app_common.h
 * @Description: common function include file
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2021.10.09 14:28:01
 * @LastEditTime: 2021.10.09 17:28:01
 */

#ifndef  __APP_COMMON_H__
#define  __APP_COMMON_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "config.h"
#include "tuya_zigbee_modules.h"
#include "tuya_zigbee_sdk.h"


#ifdef __cplusplus
extern "C" {
#endif

#define      USER_DEBUG_PRINTF               //debug uart print
#ifdef       USER_DEBUG_PRINTF
#define      tuya_print(...)                 uart_printf(UART_ID_UART0, __VA_ARGS__)

#else
#define      tuya_print(...)
#endif


#define      LED_NET_ID                      0
#define      LED_STATUS_ID                   0

#define      POWER_ON_LED_ON_TIME            1000
#define      JOIN_OK_LED_ON_TIME             2000

#define      EVT_JOIN_LED_DELAY_BLINK        DEV_EVT_1
#define      EVT_NET_LED_OFF                 DEV_EVT_2
#define      EVT_NET_LED_ON                  DEV_EVT_3
#define      EVT_SLEEP_CTRL                  DEV_EVT_4
#define      EVT_IDENTIFY_CTRL               DEV_EVT_5
#define      EVT_TOUCH_LINK_RETRY            DEV_EVT_6
#define      EVT_TOUCH_LINK_START            DEV_EVT_7
#define      EVT_EP_INFO_DELAY_REQUEST       DEV_EVT_8
#define      EVT_FORCE_REJOIN_MANUAL         DEV_EVT_9
#define      EVT_ZLL_RX_OFF_DELAY            DEV_EVT_10
#define      EVT_ZLL_HEART_BEAT              DEV_EVT_11
#define      EVT_MF_WAKE_TIME                DEV_EVT_12


#define      KEY_ON_ID                       0
#define      KEY_OFF_ID                      1
#define      KEY_DIM_UP_ID                   2
#define      KEY_DIM_DOWN_ID                 3
#define      KEY_ID_MAX                      4

#define      ON_MODE                         0
#define      SCENE_MODE                      1

#define      BROADCAST                       0xff

#define      NO_PRESS                        0     // 0 Second
#define      ELIMINATE_SHAKING               20    // 20 Millisecond
#define      MAKE_DEV_REJOIN                 100   // 100 Millisecond
#define      MOVE_LEVEL                      800   // 800 Millisecond
#define      STEP_LEVEL                      3000  // 3 Second
#define      LOCAL_CONNECT                   3000  // 3 Second
#define      RESET_AND_LOCAL_LEAVE           5000  // 5 Second
#define      DEV_JOIN_NWK                    5000  // 5 Second
#define      DEV_ONLINE_PAIRING              5000  // 5 Second


typedef enum {
    DEV_WORK_ST_NORMAL = 0,
    DEV_WORK_ST_TEST
}DEV_WORK_ST_T;

typedef enum {
    RESET_FLAG = 0,
    STATT_TO_CONNECT_FLAG,
}DEV_CONNECT_STATUS;

typedef enum {
    LED_OP_INIT,
    LED_OP_JOIN_START,
    LED_OP_JOIN_OK,
    LED_OP_JOIN_TIMEOUT,
    LED_OP_KEY_PUSH,
    LED_OP_KEY_POP,
    LED_OP_KEY_TOUCH_LINK,
    LED_OP_KEY_TOUCH_RESET,
}LED_OP_T;

typedef struct {
  uint8_t nwkType;
  uint8_t sceneId;
}sceneType_t;

extern const gpio_config_t gpio_input_config[];

extern const gpio_config_t gpio_ouput_config[];

extern DEV_WORK_ST_T g_work_st;
extern bool_t g_button_test;
extern uint16_t group_id;
extern uint8_t g_touch_link_action;
extern uint8_t g_zll_touch_link_cnt;
extern uint8_t g_zll_rst_cnt;
extern uint16_t g_ep_info_node_id;

void ember_zll_stop_scan(void);

/**
 * @description: set_group_info
 * @param {group id} group id
 * @return: null;
 */
void set_group_info(uint16_t group_id);

/**
 * @description: get_group_info
 * @param null
 * @return: {group id} group id;
 */
uint16_t get_group_info(void);

/**
 * @description: device gpio init
 * @param: none
 * @return: none
 */
void dev_gpio_init(void);

/**
 * @description: device battery power init
 * @param {type} none
 * @return: none
 */
void dev_battery_init(void);

/**
 * @description: device peripherals init
 * @param {type} none
 * @return: none
 */
void dev_pre_init(void);

void dev_group_cmd_callback(ext_plugin_args_t *args);

void dev_identify_cmd_callback(ext_plugin_args_t *args);

void dev_evt_callback(uint8_t evt);

void led_op_base(uint8_t id, DEV_IO_ST_T st, uint16_t delay_toggle);

void led_op(LED_OP_T op);

extern void dev_key_handle(uint32_t key_id, key_st_t key_st, uint32_t push_time);


extern void zg_sdk_info_join_type_distribute_set(void);
extern void zg_sdk_info_join_type_centralized_set(void);
extern void zg_force_sleep(uint32_t durationMs); //sleep
extern void dev_zigbee_enable_zigbee30_test_mode(bool_t enable_flag);
extern void sdk_config_multi_key_join(bool_t use_flag);
extern void dev_zigbee_recovery_network_set(bool_t enable);
extern NET_EVT_T nwk_state_get(void);
extern void zll_touchlink_enable(bool_t en);
extern void zll_rx_on_idle_set(bool_t enable);
extern void zll_touch_link_stop(void);
extern void dev_zigbee_clear_group(uint8_t ep);


#ifdef __cplusplus
}
#endif

#endif