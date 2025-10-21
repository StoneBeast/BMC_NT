#ifndef __TIMER_H
#define __TIMER_H

#include <stdint.h>

void BasicTimer_Init(void);
uint32_t GetSystemTick(void);
void Delay_ms(uint32_t ms);
uint8_t IsTimeout(uint32_t startTick, uint32_t timeout_ms);

#endif // !__TIMER_H
