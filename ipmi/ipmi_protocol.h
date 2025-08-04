#ifndef __IPMI_PROTOCOL_H
#define __IPMI_PROTOCOL_H

#include <stdint.h>

#define IPMI_PROTOCOL_MAX_LEN               32
#define IPMI_PROTOCOL_FORMAT_LEN            3
#define IPMI_PROTOCOL_DATA_MAX_LEN          (IPMI_PROTOCOL_MAX_LEN - IPMI_PROTOCOL_FORMAT_LEN)

#define IPMI_PROTOCOL_MSG_TYPE_OFFSET       0
#define IPMI_PROTOCOL_MSG_TYPE_LEN          1
#define IPMI_PROTOCOL_MSG_CODE_OFFSET       1
#define IPMI_PROTOCOL_MSG_CODE_LEN          1
#define IPMI_PROTOCOL_MSG_DATA_LEN_OFFSET   2
#define IPMI_PROTOCOL_MSG_DATA_LEN_LEN      1
#define IPMI_PROTOCOL_MSG_DATA_OFFSET       3
#define IPMI_PROTOCOL_MSG_CHK_OFFSET        (IPMI_PROTOCOL_MAX_LEN-1)
#define IPMI_PROTOCOL_MSG_CHK_LEN           1

int ipmi_msg_send(uint8_t addr, const uint8_t* msg, uint32_t timeout_ms);
int ipmi_msg_recv(uint8_t * const msg, uint32_t timeout_ms);
int check_msg(const uint8_t *msg);

#define IPMB_BUSY_TRY_COUNT                 (10)
#define IPMB_BUSY_TRY_INTERVAL_MS           (10)

#define IPMI_ERR_OK                         (0)
#define IPMI_ERR_TIMEOUT                    (-1)
#define IPMI_ERR_BUSY                       (-2)
#define IPMI_ERR_NO_DEVICE                  (-3)

#endif // !__IPMI_PROTOCOL_H
