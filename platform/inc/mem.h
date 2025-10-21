#ifndef __MEM_H
#define __MEM_H

#include <stdint.h>

int i2c_mem_read(I2C_TypeDef *I2Cx, uint8_t dev_addr, uint8_t mem_addr, uint32_t byte_count, uint8_t *buf, uint32_t timeout);
int i2c_mem_write(I2C_TypeDef *I2Cx, uint8_t dev_addr, uint8_t mem_addr, uint32_t byte_count, const uint8_t *buf, uint32_t timeout);

#endif // !__MEM_H
