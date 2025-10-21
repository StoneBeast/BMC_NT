#include "platform.h"

// 系统时钟频率（根据实际配置调整）
#define SYSTEM_CLOCK_FREQ SystemCoreClock
// 定时器分频后频率（1MHz，每个计数1us）
#define TIMER_CLOCK_FREQ 1000000UL
// 1ms对应的计数值
#define TIMER_1MS_COUNT (TIMER_CLOCK_FREQ / 1000)

// 系统时基变量
volatile uint32_t SystemTick = 0;

void BasicTimer_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    // 1. 使能定时器时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

    // 2. 配置定时器时基
    TIM_TimeBaseInitStruct.TIM_Prescaler = (SYSTEM_CLOCK_FREQ / TIMER_CLOCK_FREQ) - 1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_Period = TIMER_1MS_COUNT - 1; // 自动重装载值
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0;

    TIM_TimeBaseInit(TIM6, &TIM_TimeBaseInitStruct);

    // 3. 使能定时器更新中断
    TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);

    // 4. 配置NVIC
    NVIC_InitStruct.NVIC_IRQChannel = TIM6_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    // 5. 使能定时器
    TIM_Cmd(TIM6, ENABLE);
}

// 获取系统时基值
uint32_t GetSystemTick(void)
{
    return SystemTick;
}

// 延时函数（基于系统时基）
void Delay_ms(uint32_t ms)
{
    uint32_t startTick = SystemTick;
    while ((SystemTick - startTick) < ms)
    {
        // 等待
    }
}

// 检查超时
uint8_t IsTimeout(uint32_t startTick, uint32_t timeout_ms)
{
    return ((SystemTick - startTick) >= timeout_ms);
}
