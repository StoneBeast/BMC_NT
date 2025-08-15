#ifndef __SYSTEM_INTERFACE_H
#define __SYSTEM_INTERFACE_H

#include "platform.h"

#define SYSTEM_REQUEST_MAX_COUNT    (2)
#define SYSTEM_REQUEST_MAX_LEN      (32)
#define SYSTEM_RESPONSE_LEN         (256u)

#define SYS_MSG_TYPE_OFFSET     0
#define SYS_MSG_TYPE_LENGTH     1
#define SYS_MSG_CODE_OFFSET     1
#define SYS_MSG_CODE_LENGTH     1
#define SYS_MSG_LEN_OFFSET      2
#define SYS_MSG_LEN_LENGTH      2
#define SYS_MSG_DATA_OFFSET     4
#define SYS_MSG_DATA_LENGTH     1
#define SYS_MSG_FORMAT_LENGTH   5

#define SYS_MSG_TYPE_REQ        0x01
#define SYS_MSG_TYPE_RES        0x02
#define SYS_CMD_DEVICE_LIST     0x01
#define SYS_CMD_DEVICE_SENSOR   0x02
#define SYS_CMD_GET_EVENT       0x81

typedef struct {
    uint16_t msg_len;
    uint8_t msg[SYSTEM_REQUEST_MAX_LEN];
}sys_req_msg_t;

uint8_t init_sysInterface(void);

#endif // !__SYSTEM_INTERFACE_H
