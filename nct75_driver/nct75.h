#ifndef __NCT75_H
#define __NCT75_H

#include <stdint.h>

#define SENSOR_NCT75_1_ADDR 0x48
#define SENSOR_NCT75_2_ADDR 0x4A

uint16_t read_nct75_row_data(uint8_t dev_addr);

#endif // !__NCT75_H
