/*
 * @file: app_common.c
 * @brief: 
 * @author: Arien
 * @date: 2021-08-31 18:00:00
 * @email: Arien.ye@tuya.com
 * @copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @company: http://www.tuya.com
 */
#include "config.h"
#include "app_common.h"
#include "ias_zone_cluster.h"
#include "device_register.h"

// key lock variable
bool_t g_key_lock_flg = TRUE;
// mftest uart config struct
user_uart_config_t uart_config;
// alarm config information struct
sensor_t alarm_info;
// tamper config information struct
sensor_t tamper_info;

// key configure list
const gpio_config_t gpio_key_config[] = {
    KEYS_IO_LIST
};

// led io configure list
const gpio_config_t gpio_output_config[] = {
    LEDS_IO_LIST
};

// sensor io configure list
const gpio_config_t gpio_sensor_config[] = {
    SENSORS_IO_LIST
};

// tamper io configure list
const gpio_config_t gpio_tamper_config[] = {
    TAMPER_IO_LIST
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
    ias_sensor_init();
    gpio_output_init((gpio_config_t *)gpio_output_config, get_array_len(gpio_output_config));
    gpio_button_init((gpio_config_t *)gpio_key_config, get_array_len(gpio_key_config), 50, keys_evt_handler); 
    power_sampler_init();
    tuya_print( "per init ok....\r\n");
}

/**
 * @description: ias zone sensor init
 * @param {type} none
 * @return: none
 */
void ias_sensor_init(void)
{
    ias_zone_status_bits.ias_zone_status = 0;
    //IAS_ZONE_EP:1    IAS_ZONE_TYPE:0x15(Contact switch)
    ias_zone_info_init(IAS_ZONE_EP, IAS_ZONE_TYPE);

    //init alarm config information
    memcpy(&alarm_info.sensor_io_st, &gpio_sensor_config, sizeof(gpio_config_t));

    (&alarm_info)->gpio_polarity = (DEV_IO_ST_T)ALARM1_GPIO_POLARITY; 

    (&alarm_info)->sensor_func = gpio_sensor_alarm1_interrupt_handler;

    gpio_int_register(&((&alarm_info) -> sensor_io_st), (&alarm_info) -> sensor_func);

    (&alarm_info) -> init_status = (DEV_IO_ST_T)gpio_raw_input_read_status   \
                                    ((&alarm_info) -> sensor_io_st.port, 
                                     (&alarm_info) -> sensor_io_st.pin);


    //init tamper config information
    memcpy(&tamper_info.sensor_io_st, &gpio_tamper_config, sizeof(gpio_config_t));

    (&tamper_info)->gpio_polarity = (DEV_IO_ST_T)TAMPER_GPIO_POLARITY; 

    (&tamper_info)->sensor_func = gpio_sensor_tamper_interrupt_handler;

    gpio_int_register(&((&tamper_info) -> sensor_io_st), (&tamper_info) -> sensor_func);

    (&tamper_info) -> init_status = (DEV_IO_ST_T)gpio_raw_input_read_status   \
                                    ((&tamper_info) -> sensor_io_st.port, 
                                     (&tamper_info) -> sensor_io_st.pin);

    sensor_basic_init_io_status_updata();
}

/**
 * @description: sensor pin sample debounce handler
 * @param {type} none
 * @return: none
 */
void gpio_sensor_debounce_handler(uint8_t evt)
{
    if(evt == GPIO_ALARM1_DEBOUNCE_EVT_ID) {
        dev_timer_stop(GPIO_ALARM1_DEBOUNCE_EVT_ID);
        alarm_info.last_status = alarm_info.new_status;
        if(alarm_info.new_status == (DEV_IO_ST_T)GPIO_SENSOR_INACTIVE) {
                ias_zone_status_bits.zone_bits.alarm1_status_bit0 = IAS_ZONE_STATUS_BIT_INACTIVE;    
        } else {
            ias_zone_status_bits.zone_bits.alarm1_status_bit0 = IAS_ZONE_STATUS_BIT_ACTIVE;
        }

        // alarm1 change perform
        tuya_print("ias_zone_alarm1_pre_change %d\r\n", ias_zone_status_bits.zone_bits.alarm1_status_bit0);
        dev_led_stop_blink(1, DEV_IO_ON);
        dev_timer_start_with_callback(LED_ALARM1_FLASH_EVT_ID, ALARM1_LED_FLASH_TIME, dev_evt_callback);
        ias_zone_status_update();
    }
    if(evt == GPIO_TAMPER_DEBOUNCE_EVT_ID) {
        dev_timer_stop(GPIO_TAMPER_DEBOUNCE_EVT_ID);
        tamper_info.last_status = tamper_info.new_status;
        if(tamper_info.new_status == (DEV_IO_ST_T)GPIO_SENSOR_INACTIVE) {
                ias_zone_status_bits.zone_bits.tamper_status_bit2 = IAS_ZONE_STATUS_BIT_INACTIVE;    
        } else {
            ias_zone_status_bits.zone_bits.tamper_status_bit2 = IAS_ZONE_STATUS_BIT_ACTIVE;
        }
        // tamper change perform
        tuya_print("ias_zone_tamper_pre_change %d\r\n", ias_zone_status_bits.zone_bits.tamper_status_bit2);
        dev_led_stop_blink(2, DEV_IO_ON);
        dev_timer_start_with_callback(LED_TAMPER_FLASH_EVT_ID, TAMPER_LED_FLASH_TIME, dev_evt_callback);
        ias_zone_status_update();
    }
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
                    if(dev_led_is_blink(0) == FALSE) {
                        dev_led_stop_blink(0, DEV_IO_ON);
                    }     
                } else {
                    dev_timer_start_with_callback(NWK_START_ASSOCIATE_EVT_ID, 0, dev_evt_callback); 
                }
            } else { 
                if(push_time < RESET_TIME_MS) {
                    if(dev_led_is_blink(0) == FALSE) {
                        dev_led_stop_blink(0, DEV_IO_OFF);
                    } 
                    NET_EVT_T net_status = nwk_state_get(); 
                    if((NET_JOIN_OK == net_status) || (NET_REJOIN_OK == net_status) || (NET_LOST == net_status)) {
                        ias_zone_status_update();
                        zig_report_battery_voltage(IAS_ZONE_EP);
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
                IAS_ZONE_EP,
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
        case LED_POWER_UP_SYN_EVT_ID: {
            dev_led_stop_blink(0, DEV_IO_OFF);
            g_key_lock_flg = FALSE;       
            dev_timer_stop(LED_POWER_UP_SYN_EVT_ID);  
        }
        break;
        case LED_ALARM1_FLASH_EVT_ID: {
            dev_led_stop_blink(1, DEV_IO_OFF);
            dev_timer_stop(LED_ALARM1_FLASH_EVT_ID);          
        }
        break;
        case LED_TAMPER_FLASH_EVT_ID: {
            dev_led_stop_blink(2, DEV_IO_OFF);
            dev_timer_stop(LED_TAMPER_FLASH_EVT_ID);      
        }
        break;
        case NWK_START_ASSOCIATE_EVT_ID: {
            dev_led_start_blink(0, 250, 250, DEV_LED_BLINK_FOREVER, DEV_IO_OFF);        
        }
        break;
        case NWK_UP_EVT_ID: {
            ias_zone_status_update();
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
            IAS_ZONE_EP,
            CLUSTER_BASIC_CLUSTER_ID,
            ATTR_POWER_SOURCE_ATTRIBUTE_ID,
            &type,
            ATTR_ENUM8_ATTRIBUTE_TYPE
        );
}

/**
 * @description: sensor init gpio set
 * @param {type} none
 * @return: none
 */
uint8_t sensor_init_io_status_set(sensor_t *senor_info)
{
    if (senor_info -> gpio_polarity == senor_info -> init_status) {
        senor_info -> new_status = (DEV_IO_ST_T)GPIO_SENSOR_ACTIVE;
    } else {
        senor_info -> new_status = (DEV_IO_ST_T)GPIO_SENSOR_INACTIVE;
    }
    
    senor_info -> last_status = senor_info -> new_status;
    return 1;
}

/**
 * @description: battery sampling event callback
 * @param {in} data_st: args point
 * @return: none
 */
void battery_sampling_evt_callback(cb_args_t *data_st)
{
    if(data_st -> type == CALLBACK_TYPE_CAPTURE_BATTERY) {
        if(data_st -> args.battery.real_percent <= 20) {
            hal_battery_capture_period_change(60 * 60 * 1000UL);
            dev_heartbeat_set(BATTERY_VOLTAGE, 60 * 60 * 1000);
        } else {
            hal_battery_capture_period_change(BATTERY_REPORT_MINUTE * 60 * 1000UL);
            dev_heartbeat_set(BATTERY_VOLTAGE, BATTERY_REPORT_MINUTE * 60 * 1000UL);
        }
    }
}



