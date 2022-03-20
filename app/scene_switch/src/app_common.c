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
    {KEY_2_PORT, KEY_2_PIN, KEY_2_MODE, KEY_2_OUT, KEY_2_DRIVER},
    {KEY_3_PORT, KEY_3_PIN, KEY_3_MODE, KEY_3_OUT, KEY_3_DRIVER},
    {KEY_4_PORT, KEY_4_PIN, KEY_4_MODE, KEY_4_OUT, KEY_4_DRIVER},
    {KEY_5_PORT, KEY_5_PIN, KEY_5_MODE, KEY_5_OUT, KEY_5_DRIVER},
    {KEY_6_PORT, KEY_6_PIN, KEY_6_MODE, KEY_6_OUT, KEY_6_DRIVER},
};

const gpio_config_t gpio_ouput_config[] =   // output IO pin configuration table
{
    {NET_LED_PORT, NET_LED_PIN, NET_LED_MODE, NET_LED_DOUT, NET_LED_DRIVE},
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


