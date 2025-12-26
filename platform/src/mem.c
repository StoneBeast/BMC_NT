#include "platform.h"

#define I2C_TIMEOUT 10000 // 操作超时计数
#define MIN(a, b) (a > b ? b : a)

#define get_ticks() GetSystemTick()

static uint8_t wait_flag_until(I2C_TypeDef *I2Cx, uint32_t I2C_FLAG, FlagStatus status, uint32_t timeout_ms, uint32_t ticks_start);
static uint8_t wait_onMasterAddressFlag_timeout(I2C_TypeDef *I2Cx, uint32_t I2C_FLAG, uint32_t timeout_ms, uint32_t ticks_start);

int i2c_mem_read(I2C_TypeDef *I2Cx, uint8_t dev_addr, uint8_t mem_addr, uint32_t byte_count, uint8_t *buf, uint32_t timeout)
{
    uint32_t recv = 0;
    uint32_t read_count = byte_count;
    uint32_t tick_start = get_ticks();

    if (wait_flag_until(I2Cx, I2C_FLAG_BUSY, SET, timeout, tick_start) != 1)
    {
        return -1;
    }

    I2C_NACKPositionConfig(I2Cx, I2C_NACKPosition_Current);

    I2C_AcknowledgeConfig(I2Cx, ENABLE);
    I2C_GenerateSTART(I2Cx, ENABLE);
    if (wait_flag_until(I2Cx, I2C_FLAG_SB, RESET, timeout, tick_start) != 1)
    {
        return -1;
    }

    I2C_Send7bitAddress(I2Cx, dev_addr, I2C_Direction_Transmitter);
    if (wait_onMasterAddressFlag_timeout(I2Cx, I2C_FLAG_ADDR, timeout, tick_start) != 1)
    {
        return -1;
    }
    CLEAR_ADDRFLAG(I2Cx);

    if (wait_flag_until(I2Cx, I2C_FLAG_TXE, RESET, timeout, tick_start) != 1)
    {
        if (I2C_GetFlagStatus(I2Cx, I2C_FLAG_AF) == SET)
            I2C_GenerateSTOP(I2Cx, ENABLE);
        return -1;
    }

    I2C_SendData(I2Cx, mem_addr);
    if (wait_flag_until(I2Cx, I2C_FLAG_TXE, RESET, timeout, tick_start) != 1)
    {
        if (I2C_GetFlagStatus(I2Cx, I2C_FLAG_AF) == SET)
            I2C_GenerateSTOP(I2Cx, ENABLE);
        return -1;
    }

    I2C_GenerateSTART(I2Cx, ENABLE);
    if (wait_flag_until(I2Cx, I2C_FLAG_SB, RESET, timeout, tick_start) != 1)
    {
        return -1;
    }
    I2C_Send7bitAddress(I2Cx, dev_addr, I2C_Direction_Receiver);
    if (wait_onMasterAddressFlag_timeout(I2Cx, I2C_FLAG_ADDR, timeout, tick_start) != 1)
    {
        return -1;
    }

    if (read_count == 0U)
    {
        /* Clear ADDR flag */
        CLEAR_ADDRFLAG(I2Cx);

        /* Generate Stop */
        I2C_GenerateSTOP(I2Cx, ENABLE);
    }
    else if (read_count == 1U)
    {
        /* Disable Acknowledge */
        I2C_AcknowledgeConfig(I2Cx, DISABLE);

        /* Clear ADDR flag */
        CLEAR_ADDRFLAG(I2Cx);

        /* Generate Stop */
        I2C_GenerateSTOP(I2Cx, ENABLE);
    }
    else if (read_count == 2U)
    {
        /* Enable Pos */
        I2C_NACKPositionConfig(I2Cx, I2C_NACKPosition_Next);

        /* Disable all active IRQs around ADDR clearing and STOP programming because the EV6_3
           software sequence must complete before the current byte end of transfer */
        __disable_irq();

        /* Clear ADDR flag */
        CLEAR_ADDRFLAG(I2Cx);

        /* Disable Acknowledge */
        I2C_AcknowledgeConfig(I2Cx, DISABLE);

        /* Re-enable IRQs */
        __enable_irq();
    }
    else
    {
        /* Enable Acknowledge */
        I2C_NACKPositionConfig(I2Cx, I2C_NACKPosition_Next);
        /* Clear ADDR flag */
        CLEAR_ADDRFLAG(I2Cx);
    }

    while (read_count > 0)
    {
        if (read_count == 1)
        {
            I2C_AcknowledgeConfig(I2Cx, DISABLE);
        }
        if (wait_flag_until(I2Cx, I2C_FLAG_RXNE, RESET, timeout, tick_start) != 1)
        {
            return -1;
        }
        *(buf + recv) = I2C_ReceiveData(I2Cx);
        recv++;
        read_count--;
    }
    I2C_GenerateSTOP(I2Cx, ENABLE);

    return 1;
}

int i2c_mem_write(I2C_TypeDef *I2Cx, uint8_t dev_addr, uint8_t mem_addr, uint32_t byte_count, const uint8_t *buf, uint32_t timeout)
{
    uint32_t write_count = byte_count;
    uint32_t tick_start = get_ticks();

    if (wait_flag_until(I2Cx, I2C_FLAG_BUSY, SET, timeout, tick_start) != 1)
    {
        return -1;
    }

    I2C_NACKPositionConfig(I2Cx, I2C_NACKPosition_Current);

    /* 发送从地址以及存储地址 */
    I2C_GenerateSTART(I2Cx, ENABLE);
    if (wait_flag_until(I2Cx, I2C_FLAG_SB, RESET, timeout, tick_start) != 1)
    {
        return -1;
    }

    I2C_Send7bitAddress(I2Cx, dev_addr, I2C_Direction_Transmitter);
    if (wait_onMasterAddressFlag_timeout(I2Cx, I2C_FLAG_ADDR, timeout, tick_start) != 1)
    {
        return -1;
    }
    CLEAR_ADDRFLAG(I2Cx);

    if (wait_flag_until(I2Cx, I2C_FLAG_TXE, RESET, timeout, tick_start) != 1)
    {
        if (I2C_GetFlagStatus(I2Cx, I2C_FLAG_AF) == SET)
            I2C_GenerateSTOP(I2Cx, ENABLE);
        return -1;
    }

    /* 发送mem addr */
    I2C_SendData(I2Cx, mem_addr);

    while (write_count > 0)
    {
        if (wait_flag_until(I2Cx, I2C_FLAG_TXE, RESET, timeout, tick_start) != 1)
        {
            if (I2C_GetFlagStatus(I2Cx, I2C_FLAG_AF) == SET)
                I2C_GenerateSTOP(I2Cx, ENABLE);
            return -1;
        }
        I2C_SendData(I2Cx, *(buf));
        buf++;
        write_count--;

        if (I2C_GetFlagStatus(I2Cx, I2C_FLAG_BTF) == SET && write_count != 0)
        {
            I2C_SendData(I2Cx, *(buf));
            buf++;
            write_count--;
        }
    }

    if (wait_flag_until(I2Cx, I2C_FLAG_BTF, RESET, timeout, tick_start) != 1)
    {
        if (I2C_GetFlagStatus(I2Cx, I2C_FLAG_AF) == SET)
            I2C_GenerateSTOP(I2Cx, ENABLE);
        return -1;
    }

    I2C_GenerateSTOP(I2Cx, ENABLE);

    return 1;
}

static uint8_t wait_flag_until(I2C_TypeDef *I2Cx, uint32_t I2C_FLAG, FlagStatus status, uint32_t timeout_ms, uint32_t ticks_start)
{
    while (I2C_GetFlagStatus(I2Cx, I2C_FLAG) == (status))
    {
        if ((get_ticks() - ticks_start) > timeout_ms || timeout_ms == 0)
        {
            if (I2C_GetFlagStatus(I2Cx, I2C_FLAG) == (status))
                return 0;
        }
    }

    return 1;
}

static uint8_t wait_onMasterAddressFlag_timeout(I2C_TypeDef *I2Cx, uint32_t I2C_FLAG, uint32_t timeout_ms, uint32_t ticks_start)
{
    while (I2C_GetFlagStatus(I2Cx, I2C_FLAG) == RESET)
    {
        if (I2C_GetFlagStatus(I2Cx, I2C_FLAG_AF) == SET)
        {
            I2C_GenerateSTOP(I2Cx, ENABLE);
            I2C_ClearFlag(I2Cx, I2C_FLAG_AF);
            return 0;
        }
        if ((get_ticks() - ticks_start) > timeout_ms || timeout_ms == 0)
        {
            if (I2C_GetFlagStatus(I2Cx, I2C_FLAG) == RESET)
                return 0;
        }
    }

    return 1;
}

uint8_t i2c_mem_read_soft(uint8_t devAddr, uint8_t regAddr, uint8_t *pData, uint16_t len)
{
    uint16_t i;
    /* 1. 启动 + 写设备地址（写） */
    i2c_Start_soft();
    i2c_SendByte_soft(devAddr);
    if (!i2c_WaitAck_soft())
    {
        i2c_Stop_soft();
        return 1;
    }

    /* 2. 写寄存器地址 */
    i2c_SendByte_soft(regAddr);
    if (!i2c_WaitAck_soft())
    {
        i2c_Stop_soft();
        return 1;
    }

    /* 3. 重启 + 读设备地址（读） */
    i2c_Start_soft();
    i2c_SendByte_soft((devAddr) | 0x01);
    if (!i2c_WaitAck_soft())
    {
        i2c_Stop_soft();
        return 1;
    }

    /* 4. 连续读取 */
    for (i = 0; i < len; i++)
    {
        pData[i] = i2c_RecvByte_soft();
        if (i == len - 1)
            i2c_NAck_soft(); /* 最后一字节回 NACK */
        else
            i2c_Ack_soft();
    }
    i2c_Stop_soft();
    return 0;
}
