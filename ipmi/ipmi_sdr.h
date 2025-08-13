/*
 * @Author       : stoneBeast
 * @Date         : 2025-08-05 18:52:56
 * @Encoding     : UTF-8
 * @LastEditors  : stoneBeast
 * @LastEditTime : 2025-08-05 18:54:25
 * @Description  : 
 */

#ifndef __IPMI_SDR_H
#define __IPMI_SDR_H

#include <stdint.h>

#define SENSOR_NAME_MAX_LEN (10)

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

#endif // !__IPMI_SDR_H
