/*** 
 * @Author       : stoneBeast
 * @Date         : 2025-08-14 13:48:04
 * @Encoding     : UTF-8
 * @LastEditors  : stoneBeast
 * @LastEditTime : 2025-08-15 09:44:30
 * @Description  : 
 */
#include "system_interface.h"
#include "ipmi.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// TODO: 当前为了兼容当前版本上位机程序，sys请求长度为变长。后续升级则最好修改为定长

static uint8_t check_msg(const uint8_t* msg);
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
        OS_PRINTF("sys req queue create err\r\n");
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
        if (recv_ret != pdPASS ) {
            OS_PRINTF("recv error ?!\r\n");
        } else {
            OS_PRINTF("recv sys_req\r\n");
        }
        

        // TODO: check request
        // TODO: switch

    }
}

static uint8_t check_msg(const uint8_t* msg)
{
    uint16_t sum = 0;
    uint16_t i = 0;

    for (i = 0; i < SYSTEM_REQUEST_MAX_LEN; i++)
    {
        sum += msg[i];
    }

    if (sum % 0x100 != 0) /* 校验失败 */
        return 1;

    /* 校验成功 */
    return 0;
}
