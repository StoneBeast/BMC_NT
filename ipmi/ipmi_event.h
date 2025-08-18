/*
 * @Author       : stoneBeast
 * @Date         : 2025-08-05 18:37:50
 * @Encoding     : UTF-8
 * @LastEditors  : stoneBeast
 * @LastEditTime : 2025-08-18 14:08:18
 * @Description  : 
 */
#ifndef __IPMI_EVENT_H
#define __IPMI_EVENT_H

#include <stdint.h>
#include "ipmi_sdr.h"

/* 把事件定义成结构体，传输也是在同平台之间传输，故无需考虑对齐问题 */
typedef ipmi_sdr ipmi_event;

int init_ipmi_event(void);
int get_event_item(ipmi_event *const event);
void add_event(const ipmi_event *event);
void is_over_value(const ipmi_sdr *sdr);

#endif // !__IPMI_EVENT_H
