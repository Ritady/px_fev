/*
 * @file: driver_sht40.h
 * @brief: 
 * @author: Arien
 * @date: 2021-10-13 20:00:00
 * @email: Arien.ye@tuya.com
 * @copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @company: http://www.tuya.com
 */
#ifndef __DRIVER_SHT40_H_
#define __DRIVER_SHT40_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "hal_gpio.h"

#define I2C_WRITE                       0               
#define I2C_READ                        1

#define SHT40_ADDR                      (0x44<<1)       /* I2C write hearder */
#define SHT40_READ_CFG_MSG              0xfd            /* read data config cmd */
#define SHT40_READ_CFG_MSG_LEN          1 
#define SHT40_RESET_MSG                 0x94
#define SHT40_RESET_MSG_LEN             1
#define SHT40_RESPONSE_LEN              6               
#define SHT40_CONVERSION_TIME_MS        15

/** 
 * the mask is used to indicate to the application 
 * layer which data needs to be reported 
 */
#define HUM_DATA_MASK                   0x01
#define TMP_DATA_MASK                   0x02
#define ALL_DATA_MASK                   (HUM_DATA_MASK|TMP_DATA_MASK)

/* repeated reading is required after data check error*/
#define FAST_SAMPLE_TMR_S               5*1000UL
#define CIR_SAMPLE_TMR_S                60U
#define CIR_REPORT_TMR_S                60U

#define SW_IIC_NUM                      2
#define IIC_SHT_INDEX                   0

/* sensor pin num*/
#define I2C_SCL_PORT        PORT_A
#define I2C_SCL_PIN         PIN_3
#define I2C_SCL_MODE        GPIO_MODE_OUTPUT_PP
#define I2C_SCL_OUT         GPIO_DOUT_HIGH
#define I2C_SCL_DRIVER      GPIO_LEVEL_LOW

#define I2C_SDA_PORT        PORT_A
#define I2C_SDA_PIN         PIN_4
#define I2C_SDA_MODE        GPIO_MODE_OUTPUT_PP
#define I2C_SDA_OUT         GPIO_DOUT_HIGH
#define I2C_SDA_DRIVER      GPIO_LEVEL_LOW

#define I2C_SENSOR_IO_LIST \
    {I2C_SCL_PORT, I2C_SCL_PIN, I2C_SCL_MODE, I2C_SCL_OUT, I2C_SCL_DRIVER, }, \
    {I2C_SDA_PORT, I2C_SDA_PIN, I2C_SDA_MODE, I2C_SDA_OUT, I2C_SDA_DRIVER, }

/* sensor chip type(not use) */
typedef enum {
    SHT40_TYPE = 1
}SENSOR_TYPE;

/* I2C io config struct */
typedef struct {
    gpio_config_t   scl_io;
    gpio_config_t   sda_io;
}sw_i2c_t;

/**
 * sensor timer id struct
 * thrd_read_tmr_id:    timer for periodic sampling
 * cycler_read_tmr_id:  timer for periodic direct reporting
 * thrd_rept_tmr_id: 
 * cycler_rept_tmr_id: 
 * fast_sample_tmr_id: timer for fast sampling when data check error
 */
typedef struct {
    uint8_t read_data_tmr_id;
    uint8_t thrd_read_tmr_id;
    uint8_t cycler_read_tmr_id;
    uint8_t thrd_rept_tmr_id;
    uint8_t cycler_rept_tmr_id;
    uint8_t fast_sample_tmr_id;
}tmp_hum_tmr_t;

/* sht40 read data state */
typedef enum {
    READ_ERR = 0,
    READ_OK
}sht40_state_t;

/**
 * SHt40 temperature value and humidity value
 * temp_value: temperature value * 1000
 * hum_value:  humidity value * 100
 */
typedef struct {
    int32_t temp_value;
    uint32_t hum_value;
}sht40_data_t;

/**
 * temperature and humidity sensor struct
 * type:    the type of sensor chip
 * sensor_i2c:the io number of software i2c 
 * endpoint: the endpoint of sensor
 * tmp_tld: the threshold value of the temperature reporting(6 is 6%)
 * hum_tld: the threshold value of the humidity reporting(6 is 0.6℃)
 * sample_s: the seconds of sampling period(if set 0, will not start threshold sample report)
 * report_s: the seconds of force reporting period
 * 
 */
typedef struct {
    SENSOR_TYPE type;
    sw_i2c_t sensor_i2c;
    uint8_t  endpoint;
    uint8_t  tmp_tld;
    uint8_t  hum_tld;
    uint16_t sample_s;
    uint16_t report_s;
    tmp_hum_tmr_t timer;
    bool_t (*pre_report_f)(sht40_state_t state, sht40_data_t* data);
    bool_t (*report_f)(uint8_t ep, sht40_data_t* data, uint8_t mask);
}tmp_hum_t;

/**
 * SHt40 sensor info
 * hal_i2c:    SHT40 sensor software pin
 * per_data_read_callback_f:  pointer of pre-read data callback function
 * read_data_callback_f:      pointer of function which send data to user
 */
typedef struct {
    sw_i2c_t hal_i2c;
    uint8_t  read_data_tmr_id;
    bool_t (*per_data_read_callback_f)(sht40_state_t state, sht40_data_t* data);
    void (*read_data_callback_f)(sht40_state_t state, sht40_data_t* data);
}sht40_t;

extern sht40_t *sht40_sensor;
extern tmp_hum_t *tmp_hum;
extern gpio_config_t i2c_sensor_io_list[];

/**
 * @description: sht40 chip init 
 * @param {type} none
 * @return {out} init state
 */
int sht40_chip_init(void);

/**
 * @description: sensor data read complete callback
 * @param {in} state: dtat read state
 * @param {in} data: the pointer of sensor data
 * @return {type} none
 */
void sht40_callback(sht40_state_t state, sht40_data_t* data);

/**
 * @description: SHT40 sensor start asynchronous sample value
 * @param {type} none
 * @return: none
 */
void ty_sht40_start_read(void);

/**
 * @description: SHT40 humidity and temperature sensor send read data command
 * @param {out} data_buff:register value
 * @return: none
 */
static void sht40_send_read_command(void);

/**
 * @description: SHT30 humidity and temperature sensor soft reset. after reset, it must wait for 0.5ms to read
 * @param {type} none
 * @return: none
 */
static void sht40_soft_reset(void);

/**
 * @description: read sensor data callback
 * @param {in} evt: timer id
 * @return: none
 */
static void sht40_read_data_callback(uint8_t evt);

/**
 * @description: SHT40 humidity and temperature sensor read register value 
 * @param {out} data_buff: register value
 * @return: none
 */
static void sht40_read_register(uint8_t *data_buff);

/**
 * @description: SHT40 Numerical conversion of temperature and humidity 
 * @param {in} buff:      register value
 * @param {out} data_ret: out data
 * @return: none
 */
static sht40_state_t sht40_data_handle(uint8_t* buff, sht40_data_t* data_ret);

/** 
 * @description: crc
 * @param {in} *p: data address
 * @param {in} counter: data numbers
 * @return: none
 */
uint8_t CRC8_Table(uint8_t *p, uint8_t counter);

/**
 * @description: i2c start signal
 * @param {in} index: i2c index
 * @return: none
 */
void tuya_sw_i2c_start(uint8_t index);

/**
 * @description: i2c stop signal
 * @param {in} index: i2c index
 * @return: none
 */
void tuya_sw_i2c_stop(uint8_t index);

/**
 * @description: send one byte to i2c bus
 * @param {in} data: one byte send to i2c
 * @param {in} index: iic index
 * @return: none
 */
void tuya_sw_i2c_send_byte(uint8_t data, uint8_t index);

/**
 * @description: send bytes to i2c bus
 * @param {in} adderss_cmd: i2c addr | write cmd
 * @param {in} buff:        pointer of date to send
 * @param {in} len:         data len
 * @param {in} index:       i2c index
 * @return: none
 */
void tuya_sw_i2c_send_bytes(uint8_t adderss_cmd, uint8_t *buff, uint8_t len, uint8_t index);

/**
 * @description: i2c wait ack
 * @param {in} index:       i2c index
 * @return: TRUE: receive ACK
 */
static bool_t tuya_sw_i2c_wait_ack(uint8_t index);

/**
 * @description: i2c ack func
 * @param {in} index: i2c index
 * @return: none
 */
static void tuya_sw_i2c_ack(uint8_t index);

/**
 * @description: i2c none ack func
 * @param {in} index: i2c index
 * @return: none
 */
static void tuya_sw_i2c_noack(uint8_t index);

/**
 * @description: recive one byte from i2c bus
 * @param {out} data: pointer of receive data
 * @param {in} index: i2c index
 * @return: none
 */
void tuya_sw_i2c_rcv_byte(uint8_t *data, uint8_t index);

/**
 * @description: recive bytes from i2c bus,last byte none ack
 * @param {in} adderss_cmd: i2c addr | read cmd
 * @param {in} buff:        pointer of date buff to receive
 * @param {in} len:         data len
 * @param {in} index:       i2c index
 * @return: none
 */
void tuya_sw_i2c_rcv_bytes(uint8_t adderss_cmd, uint8_t *buff, uint8_t len, uint8_t index);

/**
 * @description: sorftware i2c inti func
 * @param {in} i2c：i2c hal info
 * @return: status
 */
int tuya_sw_i2c_init(sw_i2c_t *i2c_io, uint8_t index);

/**
 * @description: delay us function
 * @param {in} n：time n us
 * @return: none
 */
void sw_i2c_delay_us(uint32_t n);


#ifdef __cplusplus
}
#endif
#endif