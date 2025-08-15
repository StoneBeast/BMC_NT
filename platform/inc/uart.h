/*
 * @Author       : stoneBeast
 * @Date         : 2025-07-29 14:33:46
 * @Encoding     : UTF-8
 * @LastEditors  : stoneBeast
 * @LastEditTime : 2025-08-15 14:24:07
 * @Description  : 
 */
#ifndef __UART_H
#define __UART_H

#include <stdint.h>

void init_sys_usart(void);
void init_debug_usart(void);
void usart_start_send(const uint8_t *msg);
uint8_t usart_is_send_complate(void);

#endif // !__UART_H
