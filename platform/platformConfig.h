#ifndef __PLATFORM_CONFIG_H
#define __PLATFORM_CONFIG_H

#include <stdint.h>

#ifndef PLATFROM_GD
    #define PLATFORM_ST
#else
    #define PLATFORM_GD
#endif

extern uint32_t SystemCoreClock;

#define platformCPU_CLOCK_HZ                       ( SystemCoreClock )

#ifdef PLATFORM_ST
    #define PLATFORM_WWDG_IRQHandler            WWDG_IRQHandler
    #define PLATFORM_PVD_IRQHandler             PVD_IRQHandler
    #define PLATFORM_TAMPER_IRQHandler          TAMPER_IRQHandler
    #define PLATFORM_RTC_IRQHandler             RTC_IRQHandler
    #define PLATFORM_FLASH_IRQHandler           FLASH_IRQHandler
    #define PLATFORM_RCC_IRQHandler             RCC_IRQHandler
    #define PLATFORM_EXTI0_IRQHandler           EXTI0_IRQHandler
    #define PLATFORM_EXTI1_IRQHandler           EXTI1_IRQHandler
    #define PLATFORM_EXTI2_IRQHandler           EXTI2_IRQHandler
    #define PLATFORM_EXTI3_IRQHandler           EXTI3_IRQHandler
    #define PLATFORM_EXTI4_IRQHandler           EXTI4_IRQHandler
    #define PLATFORM_DMA1_Channel1_IRQHandler   DMA1_Channel1_IRQHandler
    #define PLATFORM_DMA1_Channel2_IRQHandler   DMA1_Channel2_IRQHandler
    #define PLATFORM_DMA1_Channel3_IRQHandler   DMA1_Channel3_IRQHandler
    #define PLATFORM_DMA1_Channel4_IRQHandler   DMA1_Channel4_IRQHandler
    #define PLATFORM_DMA1_Channel5_IRQHandler   DMA1_Channel5_IRQHandler
    #define PLATFORM_DMA1_Channel6_IRQHandler   DMA1_Channel6_IRQHandler
    #define PLATFORM_DMA1_Channel7_IRQHandler   DMA1_Channel7_IRQHandler
    #define PLATFORM_ADC1_2_IRQHandler          ADC1_2_IRQHandler
    #define PLATFORM_USB_HP_CAN1_TX_IRQHandler  USB_HP_CAN1_TX_IRQHandler
    #define PLATFORM_USB_LP_CAN1_RX0_IRQHandler USB_LP_CAN1_RX0_IRQHandler
    #define PLATFORM_CAN1_RX1_IRQHandler        CAN1_RX1_IRQHandler
    #define PLATFORM_CAN1_SCE_IRQHandler        CAN1_SCE_IRQHandler
    #define PLATFORM_EXTI9_5_IRQHandler         EXTI9_5_IRQHandler
    #define PLATFORM_TIM1_BRK_IRQHandler        TIM1_BRK_IRQHandler
    #define PLATFORM_TIM1_UP_IRQHandler         TIM1_UP_IRQHandler
    #define PLATFORM_TIM1_TRG_COM_IRQHandler    TIM1_TRG_COM_IRQHandler
    #define PLATFORM_TIM1_CC_IRQHandler         TIM1_CC_IRQHandler
    #define PLATFORM_TIM2_IRQHandler            TIM2_IRQHandler
    #define PLATFORM_TIM3_IRQHandler            TIM3_IRQHandler
    #define PLATFORM_TIM4_IRQHandler            TIM4_IRQHandler
    #define PLATFORM_I2C1_EV_IRQHandler         I2C1_EV_IRQHandler
    #define PLATFORM_I2C1_ER_IRQHandler         I2C1_ER_IRQHandler
    #define PLATFORM_I2C2_EV_IRQHandler         I2C2_EV_IRQHandler
    #define PLATFORM_I2C2_ER_IRQHandler         I2C2_ER_IRQHandler
    #define PLATFORM_SPI1_IRQHandler            SPI1_IRQHandler
    #define PLATFORM_SPI2_IRQHandler            SPI2_IRQHandler
    #define PLATFORM_USART1_IRQHandler          USART1_IRQHandler
    #define PLATFORM_USART2_IRQHandler          USART2_IRQHandler
    #define PLATFORM_USART3_IRQHandler          USART3_IRQHandler
    #define PLATFORM_EXTI15_10_IRQHandler       EXTI15_10_IRQHandler
    #define PLATFORM_RTC_Alarm_IRQHandler       RTC_Alarm_IRQHandler
    #define PLATFORM_USBWakeUp_IRQHandler       USBWakeUp_IRQHandler
    #define PLATFORM_TIM8_BRK_IRQHandler        TIM8_BRK_IRQHandler
    #define PLATFORM_TIM8_UP_IRQHandler         TIM8_UP_IRQHandler
    #define PLATFORM_TIM8_TRG_COM_IRQHandler    TIM8_TRG_COM_IRQHandler
    #define PLATFORM_TIM8_CC_IRQHandler         TIM8_CC_IRQHandler
    #define PLATFORM_ADC3_IRQHandler            ADC3_IRQHandler
    #define PLATFORM_FSMC_IRQHandler            FSMC_IRQHandler
    #define PLATFORM_SDIO_IRQHandler            SDIO_IRQHandler
    #define PLATFORM_TIM5_IRQHandler            TIM5_IRQHandler
    #define PLATFORM_SPI3_IRQHandler            SPI3_IRQHandler
    #define PLATFORM_UART4_IRQHandler           UART4_IRQHandler
    #define PLATFORM_UART5_IRQHandler           UART5_IRQHandler
    #define PLATFORM_TIM6_IRQHandler            TIM6_IRQHandler
    #define PLATFORM_TIM7_IRQHandler            TIM7_IRQHandler
    #define PLATFORM_DMA2_Channel1_IRQHandler   DMA2_Channel1_IRQHandler
    #define PLATFORM_DMA2_Channel2_IRQHandler   DMA2_Channel2_IRQHandler
    #define PLATFORM_DMA2_Channel3_IRQHandler   DMA2_Channel3_IRQHandler
    #define PLATFORM_DMA2_Channel4_5_IRQHandler DMA2_Channel4_5_IRQHandler

#endif // !PLATFORM_ST


#endif // !__PLATFORM_CONFIG_H
