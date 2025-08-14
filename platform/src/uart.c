/*** 
 * @Author       : stoneBeast
 * @Date         : 2025-07-29 14:33:46
 * @Encoding     : UTF-8
 * @LastEditors  : stoneBeast
 * @LastEditTime : 2025-08-14 16:42:17
 * @Description  : 
 */
#include "platform.h"
#include <stdio.h>

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
    }

    USART_Cmd(usartx, ENABLE);
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
