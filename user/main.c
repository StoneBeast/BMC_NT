/*** 
 * @Author       : stoneBeast
 * @Date         : 2025-07-29 14:33:46
 * @Encoding     : UTF-8
 * @LastEditors  : stoneBeast
 * @LastEditTime : 2025-12-05 09:56:21
 * @Description  : 
 */

#include "platform.h"
#include "ipmi.h"
#include "system_interface.h"
#include "ipmi_sdr.h"

// DEBUG
#include "ipmi_protocol.h"
#include <string.h>

// static void blink_task_func (void* arg);

SemaphoreHandle_t uart_mutex;

int main(void)
{
    // BaseType_t ret;

    init_gpio();
    init_debug_usart();

    bmc_init();
    init_sysInterface();

    uart_mutex = xSemaphoreCreateMutex();

    // ret = xTaskCreate(blink_task_func, "blink", 128, NULL, 1, NULL);
    // if (ret != pdPASS) {
    //     PRINTF("blink create err\r\n");
    // }
    
    vTaskStartScheduler();
    
    while(1);
}

// static void blink_task_func (void* arg)
// {
//     while (1) {
//         ledOn();
//         vTaskDelay(pdMS_TO_TICKS(500));
//         ledOff();
//         vTaskDelay(pdMS_TO_TICKS(500));
//     }
// }
