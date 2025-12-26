/*
 * @Author       : stoneBeast
 * @Date         : 2025-07-29 14:33:46
 * @Encoding     : UTF-8
 * @LastEditors  : stoneBeast
 * @LastEditTime : 2025-12-19 11:02:18
 * @Description  : 
 */

#ifndef __I2C_H
#define __I2C_H

#include "platform.h"

#define I2C_ERR_OK      (0)
#define I2C_ERR_SB      (-1)
#define I2C_ERR_ADDR    (-2)

#define CLEAR_ADDRFLAG(I2Cx)          \
    do                                \
    {                                 \
        __IO uint32_t tmpreg = 0x00U; \
        tmpreg = I2Cx->SR1;           \
        tmpreg = I2Cx->SR2;           \
        (void)tmpreg;                 \
    } while (0)


void init_ipmi_i2c(uint8_t addr);
void init_sensor_i2c(void);
void I2C_it_switch(uint8_t function);
void I2C_dma_switch(uint8_t function);
uint8_t I2C_busy_status(void);
int I2C_satrt_send(uint8_t addr, const uint8_t *data_buf);
void I2C_reset(void);

void i2c_Start_soft(void);
void i2c_Stop_soft(void);
uint8_t i2c_WaitAck_soft(void);
void i2c_Ack_soft(void);
void i2c_NAck_soft(void);
void i2c_SendByte_soft(uint8_t dat);
uint8_t i2c_RecvByte_soft(void);

#endif // !__I2C_H
