/**
 * @Author: qinlang
 * @email: qinlang.chen@tuya.com
 * @LastEditors: qinlang
 * @file name: app_common.c
 * @Description: this file is a template of the sdk callbacks function,
 * user need to achieve the function yourself
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2021-08-31 17:29:33
 * @LastEditTime: 2021-08-31 15:51:35
 */
#include "app_common.h"

const gpio_config_t gpio_input_config[] =   // input IO pin configuration table
{
    {KEY_1_PORT, KEY_1_PIN, KEY_1_MODE, KEY_1_OUT, KEY_1_DRIVER},
};

const gpio_config_t gpio_ouput_config[] =   // output IO pin configuration table
{
    {LED_1_PORT, LED_1_PIN, LED_1_MODE, LED_1_DOUT, LED_1_DRIVE},
    {NET_LED_2_PORT, NET_LED_2_PIN, NET_LED_2_MODE, NET_LED_2_DOUT, NET_LED_2_DRIVE},
    {RELAY_1_PORT, RELAY_1_PIN, RELAY_1_MODE, RELAY_1_DOUT, RELAY_1_DRIVE},
};


/**
 * @description: gpio init
 * @param: none
 * @return: none
 */
void gpio_init(void)
{
    gpio_button_init((gpio_config_t *)gpio_input_config, get_array_len(gpio_input_config), 50, dev_key_handle);
    gpio_output_init((gpio_config_t *)gpio_ouput_config, get_array_len(gpio_ouput_config));
}

/**
 * @description: operate led and relay io
 * @param {endpoint} the endpoint of the write attribute
 * @param {st} on or off
 * @return: none
 */
void dev_switch_op(uint8_t endpoint, DEV_IO_ST_T st)
{
    uint8_t led_mode = 0;
	if((endpoint > CH_SUMS) || (endpoint == 0)){
		return;
	}
    g_relay_onoff_status[endpoint - 1] = st;
    switch(endpoint){
        case 1:{
            dev_io_op(RELAY_1_IO_INDEX, (DEV_IO_ST_T)g_relay_onoff_status[endpoint - 1]);
            dev_zigbee_read_attribute(endpoint, CLUSTER_ON_OFF_CLUSTER_ID, LED_MODE_ATTRIBUTE_ID, &led_mode, 1);
            if(led_mode == (uint8_t)LED_MODE_RELAY){
                dev_io_op(LED_1_IO_INDEX, (DEV_IO_ST_T)g_relay_onoff_status[endpoint - 1]);  // The status lamp is synchronized with the relay
            }
            else if(led_mode == (uint8_t)LED_MODE_POS){
                dev_io_op(LED_1_IO_INDEX, (DEV_IO_ST_T)!g_relay_onoff_status[endpoint - 1]); // Display position mode status lamp as opposed to relay
            }
            break;
        }    
        default:
            break;
    }
}
