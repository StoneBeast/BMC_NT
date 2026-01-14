/*** 
 * @Author       : stoneBeast
 * @Date         : 2025-07-29 14:33:46
 * @Encoding     : UTF-8
 * @LastEditors  : stoneBeast
 * @LastEditTime : 2026-01-14 11:05:47
 * @Description  : 
 */

#include "platform.h"
#include "ipmi.h"
#include "system_interface.h"
#include "ipmi_sdr.h"

SemaphoreHandle_t uart_mutex;

int main(void)
{

    init_gpio();

#if DEBUG_LOG
    init_debug_usart();
#endif

    bmc_init();
    init_sysInterface();

    uart_mutex = xSemaphoreCreateMutex();

    vTaskStartScheduler();
    
    while(1);
}
