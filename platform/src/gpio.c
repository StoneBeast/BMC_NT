/*** 
 * @Author       : stoneBeast
 * @Date         : 2025-07-29 14:33:46
 * @Encoding     : UTF-8
 * @LastEditors  : stoneBeast
 * @LastEditTime : 2025-10-21 16:59:44
 * @Description  : led、电池控制引脚等gpio初始化、控制函数定义
 */

#include "platform.h"
void init_gpio(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;

    GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void ledOn(void) 
{
    GPIO_ResetBits(GPIOA, GPIO_Pin_8);
}

void ledOff(void) 
{
    GPIO_SetBits(GPIOA, GPIO_Pin_8);
}

static void init_battery_pin(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOD, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
    GPIO_Init(GPIOD, &GPIO_InitStruct);

    GPIO_SetBits(GPIOB, GPIO_Pin_5);
    GPIO_SetBits(GPIOD, GPIO_Pin_2);
}

void sdr_init_battery_pin(void* arg)
{
    (void) arg;
    init_battery_pin();
}

void close_battery_pin(void)
{
    GPIO_ResetBits(GPIOB, GPIO_Pin_5);
}

void battert_warn(void)
{
    GPIO_ResetBits(GPIOD, GPIO_Pin_2);
}
