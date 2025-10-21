/*
 * @Author       : stoneBeast
 * @Date         : 2025-07-29 14:33:46
 * @Encoding     : UTF-8
 * @LastEditors  : stoneBeast
 * @LastEditTime : 2025-10-21 16:51:31
 * @Description  : 
 */

#ifndef __GPIO_H
#define __GPIO_H

void init_gpio(void);
void ledOn(void);
void ledOff(void);
void sdr_init_battery_pin(void *arg);
void close_battery_pin(void);
void battert_warn(void);

#endif // !__GPIO_H
