/*
 * @Author       : stoneBeast
 * @Date         : 2025-08-11 13:54:04
 * @Encoding     : UTF-8
 * @LastEditors  : stoneBeast
 * @LastEditTime : 2025-08-11 14:21:58
 * @Description  : 
 */

#ifndef __ADC_H
#define __ADC_H

#include <stdint.h>

void init_adc(void* arg);
uint16_t get_channel_data(uint8_t ch);

#endif // !__ADC_H
