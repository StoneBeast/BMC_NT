#ifndef __IPMI_H
#define __IPMI_H

#include <stdint.h>

#define BMC_ADDR                0x20
#define IPMI_EVENT_MAX_LEN      10

#define IPMI_MSG_CODE_SCAN      (0x01)
#define IPMI_MSG_CODE_GET_SDR   (0x02)

void bmc_init(void);
void get_all_sdr(uint8_t addr);

#endif // !__IPMI_H
