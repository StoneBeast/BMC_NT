/*** 
 * @Author       : stoneBeast
 * @Date         : 2025-10-20 17:02:58
 * @Encoding     : UTF-8
 * @LastEditors  : stoneBeast
 * @LastEditTime : 2025-10-21 15:18:46
 * @Description  : 
 */

#include "nct75.h"
#include "platformConfig.h"
#include "platform.h"

#define NCT75_I2C SENSOR_I2C

// NCT75 Register map
#define NCT75_REG_TEMP 0x00
#define NCT75_REG_CONFIG 0x01
#define NCT75_REG_THYST 0x02
#define NCT75_REG_TOS 0x03

uint16_t read_nct75_row_data(uint8_t dev_addr)
{
    uint8_t buf[2] = {0};

    // Read two bytes from temperature register
    i2c_mem_read(SENSOR_I2C, (dev_addr<<1), NCT75_REG_TEMP, 2, buf, 100);

    // Combine bytes: MSB is temperature sign and integer bits, LSB holds fractional bits
    int16_t raw = (int16_t)((buf[0] << 8) | buf[1]);
    // Shift right by 4 (12-bit resolution)
    raw >>= 4;

    return raw;
}
