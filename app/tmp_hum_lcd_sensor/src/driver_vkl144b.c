/*
 * @file: driver_vkl144b.c
 * @brief: 
 * @author: Arien
 * @date: 2021-10-13 20:00:00
 * @email: Arien.ye@tuya.com
 * @copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @company: http://www.tuya.com
 */
#include "driver_vkl144b.h"
#include "driver_sht40.h"
#include "app_common.h"

#define LCD_DELAY_REFRESH()                                                                 \
    do {                                                                                    \
        dev_timer_start_with_callback(LCD_REFRESH_EVT_ID, LCD_DELAY_REFRESH_TIME, lcd_refresh_sram); \
    } while (0)


static uint8_t lcd_cfg_msg[100];

sw_i2c_t i2c_lcd_io_list = {
    I2C_LCD_IO_LIST
};

static uint8_t vkl144_sram_shadow[8] = {
    0x00,									//0
    0x00,									//1
    0x00,									//2
    0x00,									//3
    0x00,									//4
    0x00,							        //5
    0x00,									//6
    0x00,									//7
};


const uint8_t lcd_number[] = {
    0xD7,  // 0
    0X06,  // 1
    0XE3,  // 2
    0XA7,  // 3
    0X36,  // 4
    0XB5,  // 5
    0XF5,  // 6
    0X07,  // 7
    0XF7,  // 8
    0XB7,  // 9
    0XF1,  // E
};


const uint8_t lcd_number_point[] = {
    0xDF,  // 0.
    0X0E,  // 1.
    0XEB,  // 2.
    0XAF,  // 3.
    0X3E,  // 4.
    0XBD,  // 5
    0XFD,  // 6.
    0X0F,  // 7.
    0XFF,  // 8.
    0XBF,  // 9.
    0XF9,  // E.
};

void lcd_vkl144b_init(void)
{
    tuya_sw_i2c_init(&i2c_lcd_io_list, IIC_LCD_INDEX);
    sw_i2c_delay_us(100);
    tuya_sw_i2c_stop(IIC_LCD_INDEX);

    memset(lcd_cfg_msg, 0, 100);
    uint8_t i = 0;
    lcd_cfg_msg[i] = VKL144_SOFTRST;
    lcd_cfg_msg[++i] = VKL144_ICSET_ADDR | VKL144_WRITE_COMMAND; 
    lcd_cfg_msg[++i] = VKL144_ADSET_ADDR;

    for (uint8_t j = 0; j < sizeof(vkl144_sram_shadow) - 1; j++) {
        lcd_cfg_msg[i + j + 1] = 0xff;
    }


    tuya_sw_i2c_start(IIC_LCD_INDEX);
    tuya_sw_i2c_send_bytes(ADDR | I2C_WRITE, lcd_cfg_msg, 15, IIC_LCD_INDEX);
    tuya_sw_i2c_stop(IIC_LCD_INDEX);
    sw_i2c_delay_us(500);

    i = 0;
    lcd_cfg_msg[i] = VKL144_MODESET_1_3_ON;
    lcd_cfg_msg[++i] = VKL144_DIS_LOWPOWER;
    tuya_sw_i2c_start(IIC_LCD_INDEX);
    tuya_sw_i2c_send_bytes(ADDR | I2C_WRITE, lcd_cfg_msg, 2, IIC_LCD_INDEX);
    tuya_sw_i2c_stop(IIC_LCD_INDEX);

    lcd_battery_display(100);
}

/**
* @brief dispaly battery status
* @param[in] per battery percent
* @return none
*/
void lcd_battery_display(uint8_t per) 
{
    SETBIT(vkl144_sram_shadow[7], 7);//S6
    SETBIT(vkl144_sram_shadow[7], 4);//S7
    SETBIT(vkl144_sram_shadow[7], 5);//S8
    SETBIT(vkl144_sram_shadow[7], 6);//S9
    LCD_DELAY_REFRESH();

    if (per > 80) {
        return;
    } else if (per > 50) {
        CLEARBIT(vkl144_sram_shadow[7], 6);
        return;
    } else if (per > 20) {
        CLEARBIT(vkl144_sram_shadow[7], 5);
        CLEARBIT(vkl144_sram_shadow[7], 6);
        return;
    } else if (per > 0) {
        CLEARBIT(vkl144_sram_shadow[7], 5);
        CLEARBIT(vkl144_sram_shadow[7], 6);
        CLEARBIT(vkl144_sram_shadow[7], 7);
        return;
    }

    return;
}

/**
 * @brief lcd_refresh_sram lcd sram event callback
 * @param[in] evt_id humidity to display
 */
static void lcd_refresh_sram(uint8_t evt_id) 
{
    tuya_sw_i2c_init(&i2c_lcd_io_list, IIC_LCD_INDEX);
    tuya_sw_i2c_stop(IIC_LCD_INDEX);
    tuya_sw_i2c_start(IIC_LCD_INDEX);

    memset(lcd_cfg_msg, 0, sizeof(lcd_cfg_msg));
    lcd_cfg_msg[0] = VKL144_ADSET_ADDR;
    memcpy(lcd_cfg_msg + 1, vkl144_sram_shadow, sizeof(vkl144_sram_shadow));

    tuya_sw_i2c_send_bytes(ADDR | I2C_WRITE, lcd_cfg_msg, sizeof(vkl144_sram_shadow) + 1, IIC_LCD_INDEX);
    tuya_sw_i2c_stop(IIC_LCD_INDEX);
}

/**
* @brief dispaly battery status
* @param[in] hum_value humidity to display
* @return get display status 
*     @retval 0 success
*     @retval 1 out of limit and set to E1
*/
uint32_t lcd_humidity_display(uint16_t hum_value) 
{
    if (hum_value >= LCD_HUM_MAX) {
        vkl144_sram_shadow[4] = lcd_number[10];
        vkl144_sram_shadow[5] = lcd_number[1];
        vkl144_sram_shadow[6] = 0x08;
        LCD_DELAY_REFRESH();
        return 1;
    }
    // X0.0%
    if (hum_value / 1000) {
        vkl144_sram_shadow[4] = lcd_number[hum_value / 1000];
    } else {
        vkl144_sram_shadow[4] = 0x00;
    }
    // 0X.0%
    vkl144_sram_shadow[5] = lcd_number_point[hum_value % 1000 / 100];
    // 00.X%
    vkl144_sram_shadow[6] = lcd_number_point[hum_value % 100 / 10];

    LCD_DELAY_REFRESH();

    return 0;
}

/**
 * @brief dispaly battery status
 * @param[in] temp_value temperature to display
 * @return get display status
 *     @retval 0 success
 *     @retval 1 out of limit and set to E1
 */
uint32_t lcd_temperature_display(int16_t temp_value) 
{
    if ((temp_value >= LCD_TEMP_MAX) || (temp_value <= LCD_TEMP_MIN)) {
        vkl144_sram_shadow[0] = lcd_number[10];
        vkl144_sram_shadow[1] = lcd_number[1];
        vkl144_sram_shadow[2] = 0x00;
        vkl144_sram_shadow[3] = 0x08;
        LCD_DELAY_REFRESH();
        return 1;
    }

    uint8_t is_Negative;  ///< 0:Positive 1: Negative

    is_Negative = temp_value < 0 ? 1 : 0;
    temp_value = temp_value < 0 ? -temp_value : temp_value;

    if (is_Negative) {
        SETBIT(vkl144_sram_shadow[4], 3);
    } else {
        CLEARBIT(vkl144_sram_shadow[4], 3);
    }
    // X0.00℃
    if (temp_value / 1000) {
        vkl144_sram_shadow[0] = lcd_number[temp_value / 1000];
    } else {
        vkl144_sram_shadow[0] = 0x00;
    }
    // 0X.00℃
    vkl144_sram_shadow[1] = lcd_number_point[temp_value % 1000 / 100];
    // 00.X0℃
    vkl144_sram_shadow[2] = lcd_number[temp_value % 100 / 10];
    // 00.0X℃
    vkl144_sram_shadow[3] = lcd_number_point[temp_value % 10];

    LCD_DELAY_REFRESH();
    return 0;
}

/**
* @brief net icon blink
* @param[type] none
* @return none
*/
void start_net_blink(uint8_t event) 
{
    static uint32_t status;
    if (status % 2 == 0) {
        SETBIT(vkl144_sram_shadow[7], 0);   // S2
        SETBIT(vkl144_sram_shadow[7], 1);   // S3
        SETBIT(vkl144_sram_shadow[7], 2);   // S4
        SETBIT(vkl144_sram_shadow[7], 3);   // S5
    } else {
        CLEARBIT(vkl144_sram_shadow[7], 0);
        CLEARBIT(vkl144_sram_shadow[7], 1);
        CLEARBIT(vkl144_sram_shadow[7], 2);
        CLEARBIT(vkl144_sram_shadow[7], 3);
    }
    status++;
    dev_timer_start_with_callback(LCD_NET_BLINK_EVT_ID, LCD_NET_ICON_BLINK_TIME, start_net_blink);

    LCD_DELAY_REFRESH();

    return;
};

/**
* @brief stop net icon blink
* @param[in] stop_state: DEV_IO_OFF:OFF DEV_IO_ON:ON
* @return none
*/
void stop_net_blink(DEV_IO_ST_T stop_state) 
{
    if (stop_state) {
        SETBIT(vkl144_sram_shadow[7], 0);   // S2
        SETBIT(vkl144_sram_shadow[7], 1);   // S3
        SETBIT(vkl144_sram_shadow[7], 2);   // S4
        SETBIT(vkl144_sram_shadow[7], 3);   // S5
    } else {
        CLEARBIT(vkl144_sram_shadow[7], 0);   // S2
        CLEARBIT(vkl144_sram_shadow[7], 1);   // S3
        CLEARBIT(vkl144_sram_shadow[7], 2);   // S4
        CLEARBIT(vkl144_sram_shadow[7], 3);   // S5
    }
    dev_timer_stop(LCD_NET_BLINK_EVT_ID);
    LCD_DELAY_REFRESH();
};