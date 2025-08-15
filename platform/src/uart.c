/*** 
 * @Author       : stoneBeast
 * @Date         : 2025-07-29 14:33:46
 * @Encoding     : UTF-8
 * @LastEditors  : stoneBeast
 * @LastEditTime : 2025-08-15 14:15:23
 * @Description  : 
 */
#include "platform.h"
#include <stdio.h>
#include <string.h>
#include "system_interface.h"

static uint8_t send_buffer[SYSTEM_RESPONSE_LEN] = {0};
static void __init_uart(USART_TypeDef* usartx);

void init_sys_usart(void)
{
    __init_uart(USART1);
}
void init_debug_usart(void)
{
    __init_uart(USART2);
}


/*** 
 * @brief 初始化system接口串口或调试串口
 * @param usartx [uint8_t]    
 * @return []
 */
static void __init_uart(USART_TypeDef* usartx)
{
    uint16_t usart_tx_pin = GPIO_Pin_9;
    uint16_t usart_rx_pin = GPIO_Pin_10;

    //  初始化gpio
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    if (usartx == USART1) {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    } else {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
        usart_tx_pin = GPIO_Pin_2;
        usart_rx_pin = GPIO_Pin_3;
    }

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;

    GPIO_InitStruct.GPIO_Pin = usart_rx_pin;

    GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;

    GPIO_InitStruct.GPIO_Pin = usart_tx_pin;

    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    USART_InitTypeDef USART_InitStruct;
    USART_InitStruct.USART_BaudRate = 115200;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_Init(usartx, &USART_InitStruct);

    if (usartx == USART1) {
        NVIC_InitTypeDef NVIC_InitStruct;
    
        NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
    
        NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x02;
        NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
    
        NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStruct);
    
        // TODO: 之后尽量不使用idel中断，意外因素较多，不可控
        USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
        USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);

        DMA_InitTypeDef DMA_InitStructure;
        DMA_DeInit(DMA1_Channel4);  // USART1_TX 使用 DMA1 通道4
        
        DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART1->DR);  // 外设地址
        DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)send_buffer;        // 内存地址
        DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;                   // 内存到外设
        DMA_InitStructure.DMA_BufferSize = SYSTEM_RESPONSE_LEN;              // 传输数据量
        DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;     // 外设地址固定
        DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;              // 内存地址递增
        DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // 8位
        DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;         // 8位
        DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                       // 普通模式(非循环)
        DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;               // 中优先级
        DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                        // 禁用内存到内存
        DMA_Init(DMA1_Channel4, &DMA_InitStructure);

        USART_DMACmd(usartx, USART_DMAReq_Tx, ENABLE);
    }

    USART_Cmd(usartx, ENABLE);
}

void usart_start_send(const uint8_t *msg)
{
    DMA_Cmd(DMA1_Channel4, DISABLE);
    DMA_SetCurrDataCounter(DMA1_Channel4, SYSTEM_RESPONSE_LEN);

    memcpy(send_buffer, msg, SYSTEM_RESPONSE_LEN);
    DMA_Cmd(DMA1_Channel4, ENABLE);
}

/*** 
 * @brief 查询而非阻塞，可配合外部使用 os_delay
 * @return [uin8_t] 0未完成/1完成
 */
uint8_t usart_is_send_complate(void)
{
    if (DMA_GetFlagStatus(DMA1_FLAG_TC4) == RESET)
        return 0;

    /* 清除传输完成标志 */
    DMA_ClearFlag(DMA1_FLAG_TC4);

    /* 禁用DMA通道 */
    DMA_Cmd(DMA1_Channel4, DISABLE);

    return 1;
}

// printf的重定向到USART2
#pragma import(__use_no_semihosting)
struct __FILE
{
    int handle;
};
FILE __stdout;
void _sys_exit(int x) { x = x; }
//__use_no_semihosting was requested, but _ttywrch was
void _ttywrch(int ch) { ch = ch; }

int fputc(int ch, FILE *f)
{
    // 注意：USART_FLAG_TXE是检查发送缓冲区是否为空，这个要在发送前检查，检查这个提议提高发送效率，但是在休眠的时候可能导致最后一个字符丢失
    // USART_FLAG_TC是检查发送完成标志，这个在发送后检查，这个不会出现睡眠丢失字符问题，但是效率低（发送过程中发送缓冲区已经为空了，可以接收下一个数据了，但是因为要等待发送完成，所以效率低）
    // 不要两个一起用，一起用效率最低

    // 循环等待直到发送缓冲区为空(TX Empty)此时可以发送数据到缓冲区
    while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET)
    {
    }
    USART_SendData(USART2, (uint8_t)ch);

    /* 循环等待直到发送结束*/
    while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
    {
    }

    return ch;
}
