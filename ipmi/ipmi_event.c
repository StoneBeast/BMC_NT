/*** 
 * @Author       : stoneBeast
 * @Date         : 2025-08-05 18:37:42
 * @Encoding     : UTF-8
 * @LastEditors  : stoneBeast
 * @LastEditTime : 2025-08-06 10:41:23
 * @Description  : 
 */

#include "ipmi.h"
#include "ipmi_event.h"

#include "FreeRTOS.h"
#include "queue.h"

QueueHandle_t event_queue;

int init_ipmi_event(void)
{
    event_queue = xQueueCreate(IPMI_EVENT_MAX_LEN, sizeof(ipmi_event));

    if (event_queue == NULL)
        return -1;

    return 0;
}

int get_event_item(ipmi_event *const event)
{
    BaseType_t recv_ret = pdFALSE;

    if (event == NULL)
        return -1;

    recv_ret = xQueueReceive(event_queue, event, 0);
    if (recv_ret == pdFALSE)
        return -1;

    return 0;
}
