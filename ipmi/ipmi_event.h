/*
 * @Author       : stoneBeast
 * @Date         : 2025-08-05 18:37:50
 * @Encoding     : UTF-8
 * @LastEditors  : stoneBeast
 * @LastEditTime : 2025-08-06 10:48:18
 * @Description  : 
 */
#ifndef __IPMI_EVENT_H
#define __IPMI_EVENT_H

#include <stdint.h>
#include "ipmi_sdr.h"

/* 把事件定义成结构体，传输也是在同平台之间传输，故无需考虑对齐问题 */
typedef struct
{
    uint8_t addr;
    uint8_t no;
    uint8_t is_signed;
    uint16_t read_date;
    uint16_t lower_threshold;
    uint16_t higher_threshold;
    short M;
    short K;
    uint8_t unit_code;
    uint8_t name_len;
    char name[SENSOR_NAME_MAX_LEN];
}ipmi_event;

int init_ipmi_event(void);
int get_event_item(ipmi_event *const event);

#endif // !__IPMI_EVENT_H
