/*
 * @Author       : stoneBeast
 * @Date         : 2025-08-05 18:52:56
 * @Encoding     : UTF-8
 * @LastEditors  : stoneBeast
 * @LastEditTime : 2025-08-13 11:21:39
 * @Description  : 
 */

#ifndef __IPMI_SDR_H
#define __IPMI_SDR_H

#include <stdint.h>

#define SENSOR_NAME_MAX_LEN (10)

#define SENSOR_TYPE_TEMPERATURE             0x01
#define SENSOR_TYPE_VOLTAGE                 0x02
#define SENSOR_TYPE_POWER                   0x08
#define SENSOR_UNIT_CODE_DC                 0x01
#define SENSOR_UNIT_CODE_V                  0x04
#define SENSOR_UNIT_CODE_A                  0x05

typedef struct {
    uint8_t ipmc_addr;
    uint8_t sensor_no;
    uint8_t is_signed;
    uint8_t sensor_type;
    uint16_t read_data;
    uint16_t lower_threshold;
    uint16_t higher_threshold;
    short argM;
    short argK;
    uint8_t unit_code;
    uint8_t name_len;
    char sensor_name[SENSOR_NAME_MAX_LEN];
}ipmi_sdr;

void init_ipmi_sdr(void);
uint8_t get_sdr_by_id(uint8_t id, ipmi_sdr *const sdr);
uint16_t get_sdr_count(void);
void update_sensor(void);

#endif // !__IPMI_SDR_H
