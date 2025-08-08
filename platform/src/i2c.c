/*** 
 * @Author       : stoneBeast
 * @Date         : 2025-07-29 15:17:41
 * @Encoding     : UTF-8
 * @LastEditors  : stoneBeast
 * @LastEditTime : 2025-08-04 14:21:13
 * @Description  : 
 */

#include "platform.h"
#include "platformConfig.h"
#include "ipmi_protocol.h"
#include <string.h>

uint8_t ipmi_recv_buf[IPMI_PROTOCOL_MAX_LEN] = {0};
static uint8_t ipmi_send_buf[IPMI_PROTOCOL_MAX_LEN] = {0};

static void __init_i2c(I2C_TypeDef* i2cx, uint8_t addr);
static void __init_ipmi_i2c_dma(void);

void init_ipmi_i2c(uint8_t addr)
{
    __init_i2c(IPMI_I2C, addr);
    __init_ipmi_i2c_dma();
    I2C_AcknowledgeConfig(IPMI_I2C, ENABLE);
    I2C_it_switch(1);
    I2C_dma_switch(0);
}

void init_sensor_i2c(void)
{
    __init_i2c(SENSOR_I2C, 0);
}

static void __init_i2c(I2C_TypeDef* i2cx, uint8_t addr)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    I2C_InitTypeDef I2C_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

    if(i2cx == IPMI_I2C) {
        RCC_APB1PeriphClockCmd(IPMI_I2C_RCC, ENABLE);
        GPIO_InitStructure.GPIO_Pin = IPMI_I2C_SDA | IPMI_I2C_SCL;
    } else {
        RCC_APB1PeriphClockCmd(SENSOR_I2C_RCC, ENABLE);
        GPIO_InitStructure.GPIO_Pin = SENSOR_I2C_SCL | SENSOR_I2C_SDA;
    }
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    if (i2cx == IPMI_I2C) {
        GPIO_Init(IPMI_I2C_PORT, &GPIO_InitStructure);
    } else {
        GPIO_Init(SENSOR_I2C_PORT, &GPIO_InitStructure);
    }

    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;                                // 标准I2C模式
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;                        // Tlow/Thigh = 2（标准模式）
    if (i2cx == IPMI_I2C && addr != 0x00)
        I2C_InitStructure.I2C_OwnAddress1 = addr;                             // 主机地址可设为0（不使用从机模式）
    else {
        I2C_InitStructure.I2C_OwnAddress1 = 0x00;                             // 主机地址可设为0（不使用从机模式）
    }
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;                               // 使能ACK响应
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit; // 7位地址
    I2C_InitStructure.I2C_ClockSpeed = 100000;                                // 100kHz标准模式

    I2C_Init(i2cx, &I2C_InitStructure); // 应用配置

    if (i2cx == IPMI_I2C) {
        // 配置事件中断通道
        NVIC_InitStructure.NVIC_IRQChannel = I2C1_EV_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);

        I2C_ITConfig(IPMI_I2C, I2C_IT_ERR, ENABLE);
        NVIC_InitStructure.NVIC_IRQChannel = I2C1_ER_IRQn;
        NVIC_Init(&NVIC_InitStructure);

    }

    I2C_Cmd(i2cx, ENABLE);              // 使能I2C2
}

static void __init_ipmi_i2c_dma(void)
{
    DMA_InitTypeDef DMA_InitStructure;
    DMA_DeInit(DMA1_Channel7); // I2C1_RX使用DMA1通道7

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(IPMI_I2C->DR);       // 外设地址(I2C数据寄存器)
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ipmi_recv_buf;             // 内存基地址(接收缓冲区)
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;                          // 外设为数据源
    DMA_InitStructure.DMA_BufferSize = IPMI_PROTOCOL_MAX_LEN;                      // 传输数据量(32字节)
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;        // 外设地址固定
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                 // 内存地址自增
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // 外设数据宽度8位
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;         // 内存数据宽度8位
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                           // 普通模式(非循环)
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;                     // 高优先级
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                            // 禁用内存到内存
    DMA_Init(DMA1_Channel7, &DMA_InitStructure);

    /* 配置I2C DMA发送 */
    DMA_DeInit(DMA1_Channel6);  // I2C1_TX
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ipmi_send_buf;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_Init(DMA1_Channel6, &DMA_InitStructure);

    /* 5. 使能DMA传输完成中断 */
    DMA_ITConfig(DMA1_Channel7, DMA_IT_TC, ENABLE); // 使能传输完成中断
    DMA_ITConfig(DMA1_Channel6, DMA_IT_TC, ENABLE); // 使能传输完成中断

    /* 6. 配置NVIC */
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel7_IRQn;  // DMA1通道7中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // 抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;        // 子优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           // 使能中断
    NVIC_Init(&NVIC_InitStructure);
    
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel6_IRQn; // DMA1通道7中断
    NVIC_Init(&NVIC_InitStructure);

    /* 7. 使能I2C的DMA接收请求 */
    I2C_DMACmd(IPMI_I2C, ENABLE);   // 使能I2C的DMA请求
    DMA_Cmd(DMA1_Channel6, DISABLE); // 使能DMA通道
    DMA_Cmd(DMA1_Channel7, ENABLE); // 使能DMA通道
}

void I2C_it_switch(uint8_t function)
{
    if (function == 0) {
        I2C_ITConfig(IPMI_I2C, I2C_IT_EVT, DISABLE);
    } else {
        I2C_ITConfig(IPMI_I2C, I2C_IT_EVT, ENABLE);
    }
}

void I2C_dma_switch(uint8_t function)
{
    if (function == 0) {
        DMA_Cmd(DMA1_Channel7, DISABLE);
        DMA_SetCurrDataCounter(DMA1_Channel7, IPMI_PROTOCOL_MAX_LEN);
    } else {
        DMA_Cmd(DMA1_Channel7, ENABLE);
    }
}

uint8_t I2C_busy_status(void)
{
    if (I2C_GetFlagStatus(IPMI_I2C, I2C_FLAG_BUSY) == SET) {
        return 1;
    }

    return 0;
}

/***
 * @brief I2C开始发送，这里使用的I2C外设是I2C1
 * @param addr [uint8_t]        发送的目标地址
 * @param data_buf [uint8_t*]   指向发送数据的指针
 * @return [int]                发送成功返回I2C_ERR_OK，否则返回非正错误码
 */
int I2C_satrt_send(uint8_t addr, const uint8_t* data_buf)
{
    uint32_t timeout = 5000;

    /* 关闭中断 */
    I2C_it_switch(0);
    DMA_Cmd(DMA1_Channel6, DISABLE);

    I2C_GenerateSTART(IPMI_I2C, ENABLE);

    /* 判断总线是否被占用 */
    while (RESET == I2C_GetFlagStatus(IPMI_I2C, I2C_FLAG_SB)) {
        if (timeout == 0) {
            return I2C_ERR_SB;
        }
        timeout--;
    }

    timeout = 5000;
    I2C_Send7bitAddress(IPMI_I2C, addr, I2C_Direction_Transmitter);
    while (RESET == I2C_GetFlagStatus(IPMI_I2C, I2C_FLAG_ADDR)) {
        if (timeout == 0) {
            return I2C_ERR_ADDR;
        }
        timeout--;
    }

    CLEAR_ADDRFLAG(IPMI_I2C);

    /* 清空并将发送数据复制到发送使用的buffer中 */
    memset(ipmi_send_buf, 0, IPMI_PROTOCOL_MAX_LEN);
    memcpy(ipmi_send_buf, data_buf, IPMI_PROTOCOL_MAX_LEN);

    /* 重置DMA计数，使能DMA */
    DMA_SetCurrDataCounter(DMA1_Channel6, IPMI_PROTOCOL_MAX_LEN);
    DMA_Cmd(DMA1_Channel6, ENABLE);

    return I2C_ERR_OK;
}

void I2C_reset(void)
{
    // TODO: 需要根据不同原因，指定不同的防锁死的方法
    I2C_GenerateSTOP(IPMI_I2C, ENABLE);
}
