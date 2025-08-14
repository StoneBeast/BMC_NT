/*** 
 * @Author       : stoneBeast
 * @Date         : 2025-08-14 13:48:04
 * @Encoding     : UTF-8
 * @LastEditors  : stoneBeast
 * @LastEditTime : 2025-08-14 16:59:08
 * @Description  : 
 */
#include "system_interface.h"
#include "ipmi.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define SYSTEM_REQUEST_MAX_COUNT    (2)
#define SYSTEM_REQUEST_MAX_LEN      (32)

static void sys_request_handler_task_func(void* arg);
QueueHandle_t sys_req_queue;

uint8_t init_sysInterface(void)
{
    BaseType_t ret;
    init_sys_usart();

    ret = xTaskCreate(sys_request_handler_task_func, "sys", 512, NULL, 4, NULL);
    if (ret != pdPASS) {
        PRINTF("sys handler task create err\r\n");
        return 0; 
    }
    
    sys_req_queue = xQueueCreate(SYSTEM_REQUEST_MAX_COUNT, sizeof(sys_req_msg_t));
    if (sys_req_queue == NULL) {
        PRINTF("sys req queue create err\r\n");
        return 0; 
    }

    return 1;
}

static void sys_request_handler_task_func(void *arg)
{
    (void)arg;
    sys_req_msg_t recv_req;
    BaseType_t recv_ret = pdFALSE;

    while (1) {
        recv_ret = xQueueReceive(sys_req_queue, &recv_req, portMAX_DELAY);
        if (recv_ret == pdFALSE) {
            PRINTF("recv error ?!\r\n");
            continue;
        }

        // TODO: check request
        // TODO: switch

    }
}
