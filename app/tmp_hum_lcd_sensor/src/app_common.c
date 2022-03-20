/*
 * @file: app_common.c
 * @brief: 
 * @author: Arien
 * @date: 2021-10-13 20:00:00
 * @email: Arien.ye@tuya.com
 * @copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @company: http://www.tuya.com
 */
#include "config.h"
#include "app_common.h"
#include "device_register.h"
#include "driver_vkl144b.h"


// key lock variable
bool_t g_key_lock_flg = TRUE;
// mftest uart config struct
user_uart_config_t uart_config;

// key configure list
const gpio_config_t gpio_key_config[] = {
    KEYS_IO_LIST
};

// led io configure list
const gpio_config_t gpio_output_config[] = {
    LEDS_IO_LIST
};


/**
 * @description: uart init func
 * @param {type} none
 * @return: none
 */
void uart_init(void)
{
    mf_test_uart_config();
    user_uart_init((user_uart_config_t *)&uart_config);
}

/**
 * @description: mf test uart register, aotomatic generated, not edit
 * @param {void} none
 * @return: user_uart_config_t mf uart config information
 */
user_uart_config_t* mf_test_uart_config(void)
{
    memset(&uart_config, 0, sizeof(user_uart_config_t));
    user_uart_config_t default_config = ZS3L_USART_CONFIG_DEFAULT;
    memcpy(&uart_config, &default_config, sizeof(user_uart_config_t));
    return &uart_config;
}

/**
 * @description: device peripherals init
 * @param {type} none
 * @return: none
 */
void per_dev_init(void)
{
    gpio_output_init((gpio_config_t *)gpio_output_config, get_array_len(gpio_output_config));
    gpio_button_init((gpio_config_t *)gpio_key_config, get_array_len(gpio_key_config), 50, keys_evt_handler); 
    temperature_humidity_sensor_init();
    lcd_vkl144b_init();
    power_sampler_init();
    tuya_print( "per init ok....\r\n");
}

/**
 * @description: temperature and humidity sensor init
 * @param {type} none
 * @return: none
 */
void temperature_humidity_sensor_init(void)
{
    sw_i2c_t sft_i2c = {
        i2c_sensor_io_list[0], 
        i2c_sensor_io_list[1]
    };

    tmp_hum_tmr_t timer = {
        .read_data_tmr_id = SHT40_DALAY_READ_DATA_EVT_ID,
        .thrd_read_tmr_id = THRESHOLD_READ_EVT_ID,
        .cycler_read_tmr_id = CYCLER_READ_EVT_ID,
        .thrd_rept_tmr_id = THRESHOLD_REPORT_EVT_ID,
        .cycler_rept_tmr_id = CYCLER_REPORT_EVT_ID,
        .fast_sample_tmr_id = FAST_SAMPLE_EVT_ID,
    };

    tmp_hum_t tmp_hum_sensor = {
        .type = SHT40_TYPE,
        .sensor_i2c = sft_i2c,
        .endpoint = TMP_HUM_EP,
        .tmp_tld = TMP_THRESHOLD_VALUE,     //The temperature threshold value is 0.6 C
        .hum_tld = HUM_THRESHOLD_VALUE,     //The humidity threshold value is 6%
        .sample_s = CIR_SAMPLE_TMR_S,
        .report_s = CIR_REPORT_TMR_S,
        .timer = timer,
        .pre_report_f = NULL,
        .report_f = sensor_data_report_callback,
    };

    tmp_hum = (tmp_hum_t*)malloc(sizeof(tmp_hum_t));
    memset(tmp_hum, 0, sizeof(tmp_hum_t));
    memcpy(tmp_hum, &tmp_hum_sensor, sizeof(tmp_hum_t));
    
    int ret = sht40_chip_init();
    if (ret != 0) {
        free(tmp_hum);
        return ;
    }

    return ;
}

/**
 * @description: temperature and humidity sensor report callback
 * @param: {in} ep: ep id
 * @param: {in} data: report data
 * @param: {in} mask: tmp or hum data mask
 * @return: none
 */
bool_t sensor_data_report_callback(uint8_t ep, sht40_data_t* data, uint8_t mask)
{
    tuya_print("mask: %d, tmp: %d, hum: %d\r\n", mask, data->temp_value, data->hum_value);
    if ((mask & TMP_DATA_MASK) == TMP_DATA_MASK) {
        lcd_temperature_display(data->temp_value);
        temperature_measurement_attr_report(ep, data->temp_value);
    }
    
    if ((mask & HUM_DATA_MASK) == HUM_DATA_MASK) {
        lcd_humidity_display(data->hum_value);
        humidity_measurement_attr_report(ep, data->hum_value);
    }

    return TRUE;
}

/**
 * @description: Key event handler
 * @param: {in} key_id: key id
 * @param: {in} key_st: key status
 * @param: {in} push_time: key push time
 * @return: none
 */
void keys_evt_handler(uint32_t key_id, key_st_t key_st, uint32_t push_time)
{
    if(g_key_lock_flg) {
        return;
    }
    switch(key_id) {
        case 0: {
            if(KEY_ST_PUSH == key_st) {
                if(push_time < RESET_TIME_MS) {
                    if(dev_led_is_blink(LED_INDEX_0) == FALSE) {
                        dev_led_stop_blink(LED_INDEX_0, DEV_IO_ON);
                    }     
                } else {
                    dev_timer_start_with_callback(NWK_START_ASSOCIATE_EVT_ID, NO_DELAY_TIME, dev_evt_callback); 
                }
            } else { 
                if(push_time < RESET_TIME_MS) {
                    if(dev_led_is_blink(LED_INDEX_0) == FALSE) {
                        dev_led_stop_blink(LED_INDEX_0, DEV_IO_OFF);
                    } 
                    NET_EVT_T net_status = nwk_state_get(); 
                    if((NET_JOIN_OK == net_status) || (NET_REJOIN_OK == net_status) || (NET_LOST == net_status)) {
                        sensor_read_according_to_cycles_tmr(INVALID_TMR_ID);
                        zig_report_battery_voltage(TMP_HUM_EP);
                    }
                } else {
                    dev_zigbee_join_start(JOIN_TIME_OUT_MS);
                }
            }
        } 
        break;
        default:
        break;
    }
}

/**
 * @description: report battery voltage
 * @param {in} ep: endpoind
 * @return: none
 */
void zig_report_battery_voltage(uint8_t ep)
{
    uint8_t battery_percent;
    dev_zigbee_read_attribute(
                TMP_HUM_EP,
                CLUSTER_POWER_CONFIG_CLUSTER_ID,
                ATTR_BATTERY_PERCENTAGE_REMAINING_ATTRIBUTE_ID,
                &battery_percent,
                sizeof(battery_percent)
            );

    dev_send_data_t ret_data;
    attr_key_value_t *ret_attr = ret_data.data.zg.attr;

    memset(&ret_data, 0, sizeof(ret_data));
    /* Don't care
        ret_data.delay_time
        ret_data.random_time
        ret_data.zcl_id
        ret_data.command_type
        ret_data.addr.mode
    */
    ret_data.command_id = CMD_REPORT_ATTRIBUTES_COMMAND_ID;
    ret_data.direction = ZCL_DATA_DIRECTION_SERVER_TO_CLIENT;
    ret_data.qos = QOS_0;
    
    ret_data.addr.type.gw.src_ep = ep;
    ret_data.addr.type.gw.cluster_id = CLUSTER_POWER_CONFIG_CLUSTER_ID;
    ret_attr[0].attr_id = ATTR_BATTERY_PERCENTAGE_REMAINING_ATTRIBUTE_ID;
    ret_attr[0].type = ATTR_INT8U_ATTRIBUTE_TYPE;
    ret_attr[0].value_size = 1;
    ret_attr[0].value[0] = battery_percent;
    ret_data.data.zg.attr_sum++;
    dev_zigbee_send_data(&ret_data, NULL, 2000);
}

/**
 * @description: event handler
 * @param: {in} evt: event id
 * @return: none
 */
void dev_evt_callback(uint8_t evt)
{
    switch(evt) {
        case POWER_UP_3S_SYN_EVT_ID: {
            dev_led_stop_blink(LED_INDEX_0, DEV_IO_OFF);
            g_key_lock_flg = FALSE;       
            dev_timer_stop(POWER_UP_3S_SYN_EVT_ID);  
        }
        break;
        case NWK_START_ASSOCIATE_EVT_ID: {
            dev_led_start_blink(LED_INDEX_0, NWK_JOIN_LED_BLINK_FREQ, NWK_JOIN_LED_BLINK_FREQ, DEV_LED_BLINK_FOREVER, DEV_IO_OFF);        
        }
        break;
        case NWK_UP_EVT_ID: {
            sensor_read_according_to_cycles_tmr(INVALID_TMR_ID);
        }
        break;
        default:
        break;
    }
}

/**
 * @description: power sampler init
 * @param {type} none
 * @return: none
 */
void power_sampler_init(void)
{
    battery_cfg_t battery_cfg = {
        .cap_first_delay_time = 5*60*1000,
        .cap_waitting_silence_time = 5000,
        .cap_max_period_time = BATTERY_REPORT_MINUTE*60*1000,
        .cap_max_voltage = BATTERY_MAX_MILLIV,
        .cap_min_voltage = BATTERY_MIN_MILLIV,
        .adc.adc_type = ADC_TYPE_VDD,
        .adc.pos_pin = NULL,
        .adc.neg_pin = NULL
    };

    battery_table_t battery_table[] = {
        BATTERY_DEFAULT_TABLE
    };

    battery_report_policy_t bat_policy = {
        .type = BATTERY_TYPE_DRY_BATTERY,
        .level = DEV_BUSY_LEVEL_IDLE,
        .report_no_limits_first = TRUE,
        .limits = BATTERY_REPORT_DECREASE_LIMITS_20,
        .ext_limits = 0
    };
    
    hal_battery_config(&battery_cfg, battery_table, get_array_len(battery_table));
    hal_battery_report_policy_config(&bat_policy);
    dev_config_power_source_type(battery_source);           // write power source type
}

/**
 * @description: write battery type property function
 * @param {in} type: power source type
 * @return: none
 */
void dev_config_power_source_type(power_type_t type)
{
    dev_zigbee_write_attribute(
            TMP_HUM_EP,
            CLUSTER_BASIC_CLUSTER_ID,
            ATTR_POWER_SOURCE_ATTRIBUTE_ID,
            &type,
            ATTR_ENUM8_ATTRIBUTE_TYPE
        );
}

/**
 * @description: battery sampling event callback
 * @param {in} data_st: args point
 * @return: none
 */
void battery_sampling_evt_callback(cb_args_t *data_st)
{
    if(data_st -> type == CALLBACK_TYPE_CAPTURE_BATTERY) {
        lcd_battery_display(data_st->args.battery.real_percent);
        if(data_st -> args.battery.real_percent <= 20) {
            hal_battery_capture_period_change(60 * 60 * 1000UL);
            dev_heartbeat_set(BATTERY_VOLTAGE, 60 * 60 * 1000);
        } else {
            hal_battery_capture_period_change(BATTERY_REPORT_MINUTE * 60 * 1000UL);
            dev_heartbeat_set(BATTERY_VOLTAGE, BATTERY_REPORT_MINUTE * 60 * 1000UL);
        }
    }
}