/*
 * @file: driver_vkl144b.h
 * @brief: 
 * @author: Arien
 * @date: 2021-10-13 20:00:00
 * @email: Arien.ye@tuya.com
 * @copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @company: http://www.tuya.com
 */
#ifndef __DRIVER_VKL144B_H_
#define __DRIVER_VKL144B_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "hal_gpio.h"

#define LCD_TEMP_MAX                    6000            //  60.00°C
#define LCD_TEMP_MIN                   -1000            //  -10.00°C
#define LCD_HUM_MAX                     10000           //  100.0%
#define LCD_HUM_MIN                     0               //  00.0%
#define LCD_DELAY_REFRESH_TIME          50              //  50 ms
#define LCD_NET_ICON_BLINK_TIME         250             //  250ms

#define ADDR                            0x7C            // lcd(slave) address 
#define SORESE                          0xEA
#define DISCTL                          0xBF
#define ADSET                           0x0

#define IIC_LCD_INDEX                   1

#define LCD_I2C_SCL_PORT                PORT_B
#define LCD_I2C_SCL_PIN                 PIN_0
#define LCD_I2C_SCL_MODE                GPIO_MODE_OUTPUT_PP
#define LCD_I2C_SCL_OUT                 GPIO_DOUT_HIGH
#define LCD_I2C_SCL_DRIVER              GPIO_LEVEL_LOW

#define LCD_I2C_SDA_PORT                PORT_D
#define LCD_I2C_SDA_PIN                 PIN_1
#define LCD_I2C_SDA_MODE                GPIO_MODE_OUTPUT_PP
#define LCD_I2C_SDA_OUT                 GPIO_DOUT_HIGH
#define LCD_I2C_SDA_DRIVER              GPIO_LEVEL_LOW

#define I2C_LCD_IO_LIST \
    {LCD_I2C_SCL_PORT, LCD_I2C_SCL_PIN, LCD_I2C_SCL_MODE, LCD_I2C_SCL_OUT, LCD_I2C_SCL_DRIVER, },\
    {LCD_I2C_SDA_PORT, LCD_I2C_SDA_PIN, LCD_I2C_SDA_MODE, LCD_I2C_SDA_OUT, LCD_I2C_SDA_DRIVER, }

#define VKL144_WRITE_COMMAND            0x80
#define VKL144_WRITE_SRAM               0x00
#define VKL144_MODE_SET_ADDR            0x40
#define VKL144_ADSET_ADDR               0x00        // Address set command,Used for writing DDRAM
#define VKL144_DISCTL_ADDR              0x20
#define VKL144_ICSET_ADDR               0x68
#define VKL144_BLKCTL_ADDR              0x70
#define VKL144_APCTL_ADDR               0x7C

#define MODESET_DISPLAY_ON_BIT       (1 << 3)
#define MODESET_DISPLAY_OFF_BIT      (0 << 3)
#define MODESET_BIAS_1_2_BIT         (1 << 2)
#define MODESET_BIAS_1_3_BIT         (0 << 2)
// Working current:Normal mode > Power save mode1 > Power save mode2 > Power save mode3
#define DISCTL_PWR_FR_NORMAL_BIT     (0 << 3)   //bit4-bit3=00   FR NORMAL 
#define DISCTL_PWR_FR_MODE1_BIT      (1 << 3)   //bit4-bit3=01   FR POWER SAVE MODE1
#define DISCTL_PWR_FR_MODE2_BIT      (2 << 3)   //bit4-bit3=10   FR POWER SAVE MODE2
#define DISCTL_PWR_FR_MODE3_BIT      (3 << 3)   //bit4-bit3=11   FR POWER SAVE MODE3
// waveform LINE The working current of the flip > FRAME Overturned current
#define DISCTL_PWR_FR_LINE_FLIP_BIT  (0 << 2)
#define DISCTL_PWR_FR_FRAME_FLIP_BIT (1 << 2)
// SR relates to display quality,Mode1< Mode2 < Normal < High ,High mode: VDD - VLCD >= 3.0V
#define DISCTL_PWR_SR_MODE1_BIT      (0 << 0) //SR POWER SAVE MODE1
#define DISCTL_PWR_SR_MODE2_BIT      (1 << 0) //SR POWER SAVE MODE2
#define DISCTL_PWR_SR_NORMAL_BIT     (2 << 0) //SR POWER SAVE MODE1
#define DISCTL_PWR_SR_HIGH_BIT       (3 << 0) //SR NORMAL
// Chip system Setup
#define ICSET_SOFTRESET_EN_BIT       (1 << 1)
#define ICSET_SOFTRESET_DIS_BIT      (0 << 1)
#define ICSET_OSC_INTERNAL_BIT       (0 << 0)
#define ICSET_OSC_EXTERNAL_BIT       (1 << 0)
// Scintillation set
#define BLKCTL_BLINK_OFF_BIT         (0 << 0)
#define BLKCTL_BLINK_0_5_HZ_BIT      (1 << 0)
#define BLKCTL_BLINK_1_0HZ_BIT       (2 << 0)
#define BLKCTL_BLINK_2_0HZ_BIT       (3 << 0)
// controls
#define APCTL_APOFF_NORMAL_BIT       (0 << 0)
#define APCTL_APOFF_PIXELS_OFF_BIT   (1 << 0)
#define APCTL_APON_NORMAL_BIT        (0 << 1)
#define APCTL_APON_PIXELS_ON_BIT     (1 << 1)

#define VKL144_SOFTRST              (VKL144_ICSET_ADDR  | VKL144_WRITE_COMMAND | ICSET_SOFTRESET_EN_BIT  | ICSET_OSC_INTERNAL_BIT)
#define VKL144_DIS_TYPICAL          (VKL144_DISCTL_ADDR | VKL144_WRITE_COMMAND | DISCTL_PWR_FR_MODE3_BIT | DISCTL_PWR_SR_MODE1_BIT  | DISCTL_PWR_FR_FRAME_FLIP_BIT)
#define VKL144_DIS_NORMAL           (VKL144_DISCTL_ADDR | VKL144_WRITE_COMMAND | DISCTL_PWR_FR_NORMAL_BIT| DISCTL_PWR_SR_NORMAL_BIT | DISCTL_PWR_FR_LINE_FLIP_BIT)
#define VKL144_DIS_LOWPOWER         (VKL144_DISCTL_ADDR | VKL144_WRITE_COMMAND | DISCTL_PWR_FR_MODE3_BIT | DISCTL_PWR_SR_MODE1_BIT  | DISCTL_PWR_FR_FRAME_FLIP_BIT)

#define VKL144_MODESET_1_3_ON  	    (VKL144_MODE_SET_ADDR | VKL144_WRITE_COMMAND | MODESET_DISPLAY_ON_BIT)    // 0xc8 Open the display 1/3bias
#define VKL144_MODESET_1_3_OFF 	    (VKL144_MODE_SET_ADDR | VKL144_WRITE_COMMAND | MODESET_DISPLAY_OFF_BIT)   //0xc0 Close the display 1/3bias
#define VKL144_MODESET_1_2_ON  	    (VKL144_MODE_SET_ADDR | VKL144_WRITE_COMMAND | MODESET_DISPLAY_ON_BIT  | MODESET_BIAS_1_2_BIT)  //0xcc Open the display 1/2bias
#define VKL144_MODESET_1_2_OFF 	    (VKL144_MODE_SET_ADDR | VKL144_WRITE_COMMAND | MODESET_DISPLAY_OFF_BIT | MODESET_BIAS_1_2_BIT)  //0xc4 Close the display 1/2bias

#define TUYA_SENSOR_PRIVATE_CLUSTER         (CLUSTER_ID_T)0xE002
#define ATTR_TEMPERATURE_THESHOLD_ATTRIBUTE 0xE00B

/**
 * @brief Useful to reference a single bit of a byte.
 */
#define BIT(x)                      (1U << (x))  // Unsigned avoids compiler warnings re BIT(15)

/**
 * @brief Sets \c bit in the \c reg register or byte.
 * @note Assuming \c reg is an IO register, some platforms (such as the
 * AVR) can implement this in a single atomic operation.
 */
#define SETBIT(reg, bit)            (reg) |= BIT(bit)

/**
 * @brief Clears a bit in the \c reg register or byte.
 * @note Assuming \c reg is an IO register, some platforms (such as the AVR)
 * can implement this in a single atomic operation.
 */
#define CLEARBIT(reg, bit)          (reg) &= ~(BIT(bit))

typedef union {
    uint8_t value;
    struct {
        uint8_t low : 4;
        uint8_t high : 4;
    } bits;
}lcd_display_t;


/**
* @brief init lcd vkl144b driver
* @param[type] none
* @return none
*/
void lcd_vkl144b_init(void);

/**
* @brief dispaly battery status
* @param[in] per battery percent
* @return none
*/
void lcd_battery_display(uint8_t per);

/**
 * @brief lcd_refresh_sram lcd sram event callback
 * @param[in] evt_id humidity to display
 */
static void lcd_refresh_sram(uint8_t evt_id);

/**
* @brief dispaly battery status
* @param[in] hum_value humidity to display
* @return get display status 
*     @retval 0 success
*     @retval 1 out of limit and set to E1
*/
uint32_t lcd_humidity_display(uint16_t hum_value);

/**
 * @brief dispaly battery status
 * @param[in] temp_value temperature to display
 * @return get display status
 *     @retval 0 success
 *     @retval 1 out of limit and set to E1
 */
uint32_t lcd_temperature_display(int16_t temp_value);

/**
* @brief net icon blink
* @param[type] none
* @return none
*/
void start_net_blink(uint8_t event);

/**
* @brief stop net icon blink
* @param[in] stop_state: DEV_IO_OFF:OFF DEV_IO_ON:ON
* @return none
*/
void stop_net_blink(DEV_IO_ST_T stop_state);


#ifdef __cplusplus
}
#endif
#endif