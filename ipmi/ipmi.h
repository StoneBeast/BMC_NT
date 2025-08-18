#ifndef __IPMI_H
#define __IPMI_H

#include <stdint.h>

#define BMC_ADDR                0x20
#define IPMC_BASE_ADDR          0x40
#define IPMI_EVENT_MAX_LEN      10
#define IPMI_REQ_QUEUE_MAX_LEN  5
#define SENSOR_MAX_NUMBER       4
#define SLOT_COUNT              8

#define IPMI_MSG_CODE_SCAN      (0x01)
#define IPMI_MSG_CODE_GET_SDR   (0x02)

#define VPX_HARDWARE_ADDR(GA)   ((unsigned char)(IPMC_BASE_ADDR + GA))
#define VPX_IPMB_ADDR(ha)       ((unsigned char)(ha<<1))

void bmc_init(void);
uint8_t scan_card(uint8_t *const card_list);
void get_all_sdr(uint8_t addr);

#endif // !__IPMI_H
