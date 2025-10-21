#ifndef __PLATFORM_H
#define __PLATFORM_H

#include "stm32f10x.h"
#include <stdint.h>
#include <stdio.h>
#include "gpio.h"
#include "i2c.h"
#include "uart.h"
#include "adc.h"
#include "mem.h"
#include "nct75.h"
#include "timer.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

extern SemaphoreHandle_t uart_mutex;

#define PRINTF(...) printf(__VA_ARGS__)
#define OS_PRINTF(...)                         \
    xSemaphoreTake(uart_mutex, portMAX_DELAY); \
    printf(__VA_ARGS__);                       \
    xSemaphoreGive(uart_mutex);

#endif // !__PLATFORM_H
