/*
 * @file: driver_sht40.c
 * @brief: 
 * @author: Arien
 * @date: 2021-10-13 20:00:00
 * @email: Arien.ye@tuya.com
 * @copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @company: http://www.tuya.com
 */
#include <stdlib.h>
#include <string.h>
#include "tuya_zigbee_sdk.h"
#include "driver_sht40.h"
#include "cluster_tmp_measure.h"
#include "cluster_hum_measure.h"


sht40_t *sht40_sensor = NULL;
tmp_hum_t *tmp_hum;
static sw_i2c_t hal_i2c[SW_IIC_NUM]; 
// tmp hum sensor io list config!
gpio_config_t i2c_sensor_io_list[] = {
    I2C_SENSOR_IO_LIST
};
uint8_t crc_array[256] = {
    0x00,0x31,0x62,0x53,0xc4,0xf5,0xa6,0x97,0xb9,0x88,0xdb,0xea,0x7d,0x4c,0x1f,0x2e,
    0x43,0x72,0x21,0x10,0x87,0xb6,0xe5,0xd4,0xfa,0xcb,0x98,0xa9,0x3e,0x0f,0x5c,0x6d,
    0x86,0xb7,0xe4,0xd5,0x42,0x73,0x20,0x11,0x3f,0x0e,0x5d,0x6c,0xfb,0xca,0x99,0xa8,
    0xc5,0xf4,0xa7,0x96,0x01,0x30,0x63,0x52,0x7c,0x4d,0x1e,0x2f,0xb8,0x89,0xda,0xeb,
    0x3d,0x0c,0x5f,0x6e,0xf9,0xc8,0x9b,0xaa,0x84,0xb5,0xe6,0xd7,0x40,0x71,0x22,0x13,
    0x7e,0x4f,0x1c,0x2d,0xba,0x8b,0xd8,0xe9,0xc7,0xf6,0xa5,0x94,0x03,0x32,0x61,0x50,
    0xbb,0x8a,0xd9,0xe8,0x7f,0x4e,0x1d,0x2c,0x02,0x33,0x60,0x51,0xc6,0xf7,0xa4,0x95,
    0xf8,0xc9,0x9a,0xab,0x3c,0x0d,0x5e,0x6f,0x41,0x70,0x23,0x12,0x85,0xb4,0xe7,0xd6,
    0x7a,0x4b,0x18,0x29,0xbe,0x8f,0xdc,0xed,0xc3,0xf2,0xa1,0x90,0x07,0x36,0x65,0x54,
    0x39,0x08,0x5b,0x6a,0xfd,0xcc,0x9f,0xae,0x80,0xb1,0xe2,0xd3,0x44,0x75,0x26,0x17,
    0xfc,0xcd,0x9e,0xaf,0x38,0x09,0x5a,0x6b,0x45,0x74,0x27,0x16,0x81,0xb0,0xe3,0xd2,
    0xbf,0x8e,0xdd,0xec,0x7b,0x4a,0x19,0x28,0x06,0x37,0x64,0x55,0xc2,0xf3,0xa0,0x91,
    0x47,0x76,0x25,0x14,0x83,0xb2,0xe1,0xd0,0xfe,0xcf,0x9c,0xad,0x3a,0x0b,0x58,0x69,
    0x04,0x35,0x66,0x57,0xc0,0xf1,0xa2,0x93,0xbd,0x8c,0xdf,0xee,0x79,0x48,0x1b,0x2a,
    0xc1,0xf0,0xa3,0x92,0x05,0x34,0x67,0x56,0x78,0x49,0x1a,0x2b,0xbc,0x8d,0xde,0xef,
    0x82,0xb3,0xe0,0xd1,0x46,0x77,0x24,0x15,0x3b,0x0a,0x59,0x68,0xff,0xce,0x9d,0xac
};

/* set I2C SDA output mode */
#define SW_I2C_SDA_SET_OUTPUT_MODE(status) \
do { \
    hal_i2c[index].sda_io.mode = GPIO_MODE_OUTPUT_PP; \
    hal_i2c[index].sda_io.out = status; \
    gpio_raw_init(hal_i2c[index].sda_io);\
}while(0);

/* set I2C SDA input mode */
#define SW_I2C_SDA_SET_INPUT_MODE(status) \
do { \
    hal_i2c[index].sda_io.mode = GPIO_MODE_INPUT_PULL; \
    hal_i2c[index].sda_io.drive_flag = GPIO_LEVEL_LOW; \
    hal_i2c[index].sda_io.out = status; \
    gpio_raw_init(hal_i2c[index].sda_io);\
}while(0);

/* set I2C SCL level */
#define SW_I2C_SCL_LEVEL_SET(value) \
do { \
    gpio_raw_output_write_status(hal_i2c[index].scl_io.port, hal_i2c[index].scl_io.pin, value);\
}while(0);

/* set I2C SDA level */
#define SW_I2C_SDA_LEVEL_SET(value) \
do { \
    gpio_raw_output_write_status(hal_i2c[index].sda_io.port, hal_i2c[index].sda_io.pin, value);\
}while(0);

/* get I2C SDA level */
#define SW_I2C_SDA_LEVEL_GET(value) \
do { \
    value = gpio_raw_input_read_status(hal_i2c[index].sda_io.port, hal_i2c[index].sda_io.pin);\
}while(0);

/**
 * @description: sht40 chip init 
 * @param {type} none
 * @return {out} init state
 */
int sht40_chip_init(void)
{
    sw_i2c_t sft_i2c = {
        tmp_hum->sensor_i2c.scl_io, 
        tmp_hum->sensor_i2c.sda_io
    };

    sht40_t sht40_sensor_t = {
        sft_i2c,
        tmp_hum->timer.read_data_tmr_id,
        NULL,
        sht40_callback
    };

    sht40_sensor = (sht40_t*)malloc(sizeof(sht40_t));
    memcpy(sht40_sensor, &sht40_sensor_t, sizeof(sht40_t));
	tuya_sw_i2c_init(&sht40_sensor->hal_i2c, IIC_SHT_INDEX);

    return 0;
}

/**
 * @description: sensor data read complete callback
 * @param {in} state: dtat read state
 * @param {in} data: the pointer of sensor data
 * @return {type} none
 */
void sht40_callback(sht40_state_t state, sht40_data_t* data)
{
    static uint8_t counter = 0;

    /* data rationality check */
    if(data->temp_value >= TMP_MAX_MILLI || data->temp_value <= TMP_MIN_MILLI || 
        data->hum_value >= HUM_MAX_HUN || data->hum_value <= HUM_MIN_HUN) {
        state = READ_ERR;
    }

    /* call pre-data hanlde func */
    if (tmp_hum->pre_report_f != NULL) {
        tmp_hum->pre_report_f(state, data);
    }

    if (state == READ_OK) {
        counter = 0;
        dev_timer_stop(tmp_hum->timer.fast_sample_tmr_id);
        /* stroage sensor data. We are single threaded processing 
           so don't worry about global variables being modified */
        sensor_data.curr_hum = data->hum_value;
        sensor_data.curr_temp = data->temp_value;
        temperature_data_storage(tmp_hum->endpoint, sensor_data.curr_temp);
        humidity_data_storage(tmp_hum->endpoint, sensor_data.curr_hum);
        dev_timer_start_with_callback(tmp_hum->timer.fast_sample_tmr_id, 
                                            FAST_SAMPLE_TMR_S, 
                                            sensor_report_according_to_threshold);
    } else {
        if (counter < 5) {
            counter++;
            sht40_soft_reset();
            dev_timer_start_with_callback(tmp_hum->timer.fast_sample_tmr_id, 
                                            FAST_SAMPLE_TMR_S, 
                                            sensor_report_according_to_cycles_tmr);
        }
    }
}

/**
 * @description: SHT40 sensor start asynchronous sample value
 * @param {type} none
 * @return: none
 */
void ty_sht40_start_read(void)
{
    /* send data read command */
    sht40_send_read_command();

    /* delay 15ms for the sensor to finish generating a new value */
    bool_t is_active;
    is_active = dev_timer_get_valid_flag(sht40_sensor->read_data_tmr_id);
    if (!is_active) {
        dev_timer_start_with_callback(sht40_sensor->read_data_tmr_id, SHT40_CONVERSION_TIME_MS, sht40_read_data_callback);
    }
}

/**
 * @description: SHT40 humidity and temperature sensor send read data command
 * @param {out} data_buff:register value
 * @return: none
 */
static void sht40_send_read_command(void)
{
    uint8_t cfg_msg = SHT40_READ_CFG_MSG;

    tuya_sw_i2c_start(IIC_SHT_INDEX);
    tuya_sw_i2c_send_bytes(SHT40_ADDR | I2C_WRITE, &cfg_msg, SHT40_READ_CFG_MSG_LEN, IIC_SHT_INDEX);
    tuya_sw_i2c_stop(IIC_SHT_INDEX);
}

/**
 * @description: SH40 humidity and temperature sensor soft reset. after reset, it must wait for 0.5ms to read
 * @param {type} none
 * @return: none
 */
static void sht40_soft_reset(void)
{
    uint8_t cfg_msg = SHT40_RESET_MSG;
    
    tuya_sw_i2c_start(IIC_SHT_INDEX);
    tuya_sw_i2c_send_bytes(SHT40_ADDR | I2C_WRITE, &cfg_msg, SHT40_RESET_MSG_LEN, IIC_SHT_INDEX);
    tuya_sw_i2c_stop(IIC_SHT_INDEX);
}

/**
 * @description: read sensor data callback
 * @param {in} evt: timer id
 * @return: none
 */
static void sht40_read_data_callback(uint8_t evt)
{
    sht40_state_t status;
    uint8_t buff[SHT40_RESPONSE_LEN];
    sht40_data_t data_ret;

    memset(buff, 0, sizeof(buff));

    /* read sensor register */
    sht40_read_register(buff);
    
    /* read register finsh,to handler data and return status */
    status = sht40_data_handle(buff, &data_ret);

    /* read callback */
    if (sht40_sensor->read_data_callback_f != NULL) {
        sht40_sensor->read_data_callback_f(status, &data_ret);
    } 
}

/**
 * @description: SHT40 humidity and temperature sensor read register value 
 * @param {out} data_buff: register value
 * @return: none
 */
static void sht40_read_register(uint8_t *data_buff)
{
    tuya_sw_i2c_start(IIC_SHT_INDEX);
    tuya_sw_i2c_rcv_bytes(SHT40_ADDR | I2C_READ, data_buff, SHT40_RESPONSE_LEN, IIC_SHT_INDEX);
    tuya_sw_i2c_stop(IIC_SHT_INDEX);
}

/**
 * @description: SHT40 Numerical conversion of temperature and humidity 
 * @param {in} buff:      register value
 * @param {out} data_ret: out data
 * @return: none
 */
static sht40_state_t sht40_data_handle(uint8_t* buff, sht40_data_t* data_ret)
{
    uint16_t temp, hum;
    uint16_t hum_hund;
    int32_t temp_milliC;

    if ((CRC8_Table(&buff[0], 2) != buff[2]) || (CRC8_Table(&buff[3], 2) != buff[5])) {
        return READ_ERR;
    }

    temp = ((uint16_t)buff[0] << 8) | buff[1];
    hum = ((uint16_t)buff[3] << 8) | buff[4];
    
    hum_hund = ((hum * 125 * 100) / 65535) * 10 - 6000;
	temp_milliC = ((temp * 175 * 100) / 65535) * 10 - 45000;

    data_ret->hum_value = hum_hund;
    data_ret->temp_value = temp_milliC;

    return READ_OK;
}

/** 
 * @description: crc
 * @param {in} *p: data address
 * @param {in} counter: data numbers
 * @return: none
 */
uint8_t CRC8_Table(uint8_t *p, uint8_t counter)
{
    uint8_t crc8 = 0xFF;
    for( ; counter > 0; counter--)
    {
       crc8 = crc_array[crc8^*p]; 
       p++;
    }
  return crc8;
}

/**
 * @description: i2c start signal
 * @param {in} index: i2c index
 * @return: none
 */
void tuya_sw_i2c_start(uint8_t index)
{
    SW_I2C_SDA_SET_OUTPUT_MODE(GPIO_DOUT_HIGH);    //SDA output mode

    SW_I2C_SCL_LEVEL_SET(GPIO_DOUT_HIGH);
    SW_I2C_SDA_LEVEL_SET(GPIO_DOUT_HIGH);
    sw_i2c_delay_us(50);

    SW_I2C_SDA_LEVEL_SET(GPIO_DOUT_LOW);
    sw_i2c_delay_us(50);
  
    SW_I2C_SCL_LEVEL_SET(GPIO_DOUT_LOW);
    sw_i2c_delay_us(50);
}

/**
 * @description: i2c stop signal
 * @param {in} index: i2c index
 * @return: none
 */
void tuya_sw_i2c_stop(uint8_t index)
{
    SW_I2C_SDA_SET_OUTPUT_MODE(GPIO_DOUT_LOW);     //SDA input mode

    SW_I2C_SCL_LEVEL_SET(GPIO_DOUT_LOW);
    SW_I2C_SDA_LEVEL_SET(GPIO_DOUT_LOW);
    sw_i2c_delay_us(50);

    SW_I2C_SCL_LEVEL_SET(GPIO_DOUT_HIGH);
    sw_i2c_delay_us(50);

    SW_I2C_SDA_LEVEL_SET(GPIO_DOUT_HIGH);
    sw_i2c_delay_us(50);
}

/**
 * @description: send one byte to i2c bus
 * @param {in} data: one byte send to i2c
 * @param {in} index: i2c index
 * @return: none
 */
void tuya_sw_i2c_send_byte(uint8_t data, uint8_t index)
{
    uint8_t i = 0;

    SW_I2C_SCL_LEVEL_SET(GPIO_DOUT_LOW);
    SW_I2C_SDA_SET_OUTPUT_MODE(GPIO_DOUT_HIGH);
    for(i=0; i<8; i++)
    {
        if(data & 0x80) {
            SW_I2C_SDA_LEVEL_SET(GPIO_DOUT_HIGH);
        }
        else {
            SW_I2C_SDA_LEVEL_SET(GPIO_DOUT_LOW);
        }
        sw_i2c_delay_us(50);
        SW_I2C_SCL_LEVEL_SET(GPIO_DOUT_HIGH);
        sw_i2c_delay_us(50);
        SW_I2C_SCL_LEVEL_SET(GPIO_DOUT_LOW);
        sw_i2c_delay_us(50);
        data <<= 1;
    }
}

/**
 * @description: send bytes to i2c bus
 * @param {in} adderss_cmd: i2c addr | write cmd
 * @param {in} buff:        pointer of date to send
 * @param {in} len:         data len
 * @param {in} index:       i2c index
 * @return: none
 */
void tuya_sw_i2c_send_bytes(uint8_t adderss_cmd, uint8_t *buff, uint8_t len, uint8_t index)
{
    uint8_t i;

    tuya_sw_i2c_send_byte(adderss_cmd, index);
    tuya_sw_i2c_wait_ack(index);

    for(i = 0; i < len; i++) {
        tuya_sw_i2c_send_byte(buff[i], index);
        tuya_sw_i2c_wait_ack(index);
    }
}

/**
 * @description: i2c wait ack
 * @param {in} index: i2c index
 * @return: TRUE: receive ACK
 */
static bool_t tuya_sw_i2c_wait_ack(uint8_t index)
{
    uint8_t cnt = 20;
    uint8_t level;

    SW_I2C_SDA_LEVEL_SET(GPIO_DOUT_HIGH);
    sw_i2c_delay_us(50);

    SW_I2C_SDA_SET_INPUT_MODE(GPIO_DOUT_HIGH);

    SW_I2C_SCL_LEVEL_SET(GPIO_DOUT_LOW);    /* put down SCL ready to cheack SCA status */
    sw_i2c_delay_us(50);
    
    SW_I2C_SCL_LEVEL_SET(GPIO_DOUT_HIGH);
    sw_i2c_delay_us(50);
    SW_I2C_SDA_LEVEL_GET(level);                /* get ack */
    while (level) {
        cnt--;
        if (cnt == 0) {
            SW_I2C_SCL_LEVEL_SET(GPIO_DOUT_LOW);
            return FALSE;
        }
        sw_i2c_delay_us(50);
        SW_I2C_SDA_LEVEL_GET(level);
    }

    SW_I2C_SCL_LEVEL_SET(GPIO_DOUT_LOW);
    sw_i2c_delay_us(50);
    
    return TRUE;
}

/**
 * @description: i2c ack func
 * @param {in} index: i2c index
 * @return: none
 */
static void tuya_sw_i2c_ack(uint8_t index)
{
    SW_I2C_SCL_LEVEL_SET(GPIO_DOUT_LOW);
    sw_i2c_delay_us(50);

    SW_I2C_SDA_SET_OUTPUT_MODE(GPIO_DOUT_LOW);

    SW_I2C_SDA_LEVEL_SET(GPIO_DOUT_LOW);
    
    sw_i2c_delay_us(50);
    
    SW_I2C_SCL_LEVEL_SET(GPIO_DOUT_HIGH);
    sw_i2c_delay_us(50);
    SW_I2C_SCL_LEVEL_SET(GPIO_DOUT_LOW);
    sw_i2c_delay_us(50);
}

/**
 * @description: i2c none ack func
 * @param {in} index: i2c index
 * @return: none
 */
static void tuya_sw_i2c_noack(uint8_t index)
{
    SW_I2C_SDA_SET_OUTPUT_MODE(GPIO_DOUT_HIGH);
    SW_I2C_SDA_LEVEL_SET(GPIO_DOUT_HIGH);

    sw_i2c_delay_us(50);
    SW_I2C_SCL_LEVEL_SET(GPIO_DOUT_HIGH);
    sw_i2c_delay_us(50);
    SW_I2C_SCL_LEVEL_SET(GPIO_DOUT_LOW);
    sw_i2c_delay_us(50);
}

/**
 * @description: recive one byte from i2c bus
 * @param {out} data: pointer of receive data
 * @param {in}  index: i2c index
 * @return: none
 */
void tuya_sw_i2c_rcv_byte(uint8_t *data, uint8_t index)
{
    uint8_t i;
    uint8_t level;
    SW_I2C_SDA_SET_INPUT_MODE(GPIO_DOUT_HIGH);

    sw_i2c_delay_us(50);
    for (i = 0; i < 8; i++) {
        SW_I2C_SCL_LEVEL_SET(GPIO_DOUT_LOW);
        sw_i2c_delay_us(50);

        SW_I2C_SCL_LEVEL_SET(GPIO_DOUT_HIGH);
        *data = *data << 1;
        SW_I2C_SDA_LEVEL_GET(level);
        if(level) {
            *data |= 1;
        }
        sw_i2c_delay_us(50);
    }
    SW_I2C_SCL_LEVEL_SET(GPIO_DOUT_LOW);
}

/**
 * @description: recive bytes from i2c bus,last byte none ack
 * @param {in} adderss_cmd: i2c addr | read cmd
 * @param {in} buff:        pointer of date buff to receive
 * @param {in} len:         data len
 * @param {in} index:       i2c index
 * @return: none
 */
void tuya_sw_i2c_rcv_bytes(uint8_t adderss_cmd, uint8_t *buff, uint8_t len, uint8_t index)
{
    uint8_t i;
    
    tuya_sw_i2c_send_byte(adderss_cmd, index);
    tuya_sw_i2c_wait_ack(index);
    for (i = 0; i < len; i++) {
        tuya_sw_i2c_rcv_byte(&buff[i], index);
        if(i < len - 1) {
            tuya_sw_i2c_ack(index);
        }
        else {
            tuya_sw_i2c_noack(index);
        }
    }
}

/**
 * @description: sorftware i2c inti func
 * @param {in} i2c：i2c hal info
 * @return: status
 */
int tuya_sw_i2c_init(sw_i2c_t *i2c_io, uint8_t index)
{
    if (i2c_io == NULL) {
        return -1;
    }

    memcpy(&hal_i2c[index], i2c_io, sizeof(sw_i2c_t));

    gpio_raw_init(i2c_io->sda_io);
    gpio_raw_init(i2c_io->scl_io);

    SW_I2C_SCL_LEVEL_SET(GPIO_DOUT_HIGH);
    SW_I2C_SDA_LEVEL_SET(GPIO_DOUT_HIGH);

    return 0;
}

/**
 * @description: delay us function
 * @param {in} n：time n us
 * @return: none
 */
void sw_i2c_delay_us(uint32_t n)
{
    while (n--) {
        ;
    }
}