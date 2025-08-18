/*** 
 * @Author       : stoneBeast
 * @Date         : 2025-08-05 18:37:42
 * @Encoding     : UTF-8
 * @LastEditors  : stoneBeast
 * @LastEditTime : 2025-08-18 13:49:23
 * @Description  : 
 */

#include "platform.h"

#include "ipmi.h"
#include "ipmi_event.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "timers.h"

QueueHandle_t event_queue;
static TimerHandle_t event_handler_timer;
static void event_handler_callback(TimerHandle_t xTimer);

int init_ipmi_event(void)
{
    event_queue = xQueueCreate(IPMI_EVENT_MAX_LEN, sizeof(ipmi_event));

    if (event_queue == NULL)
        return -1;

    event_handler_timer = xTimerCreate("eve", pdMS_TO_TICKS(500), pdTRUE, (void *)0, event_handler_callback);
    if (event_handler_timer != NULL) {
        PRINTF("create event_handler");
    }
    xTimerStart(event_handler_timer, 0);
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

void add_event(const ipmi_event* event)
{
    xQueueSend(event_queue, event, 0);
}

void is_over_value(const ipmi_sdr* sdr)
{
    /* 是否超出阈值 */
    if (sdr->is_signed == 0) { /* 无符号 */
        if ((sdr->read_data < sdr->lower_threshold) || (sdr->read_data > sdr->higher_threshold)) {
            /* 超出阈值，产生事件 */
            add_event((ipmi_event*)sdr);
        }
    } else { /* 有符号 */
        if (((short)(sdr->read_data) < (short)(sdr->lower_threshold)) || ((short)(sdr->read_data) > (short)(sdr->higher_threshold))) {
            /* 超出阈值，产生事件 */
            add_event((ipmi_event*)sdr);
        }
    }
}

// DEBUG: 添加测试使用的事件处理函数
static void event_handler_callback(TimerHandle_t xTimer)
{
    ipmi_event item;

    while (get_event_item(&item) == 0) {
        OS_PRINTF("event:\r\naddr: %#02x\r\nname: %s\r\n", item.ipmc_addr, item.sensor_name);
    }
}
