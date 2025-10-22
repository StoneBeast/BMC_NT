/*
 * @Author       : stoneBeast
 * @Date         : 2025-08-14 13:48:20
 * @Encoding     : UTF-8
 * @LastEditors  : stoneBeast
 * @LastEditTime : 2025-10-20 10:46:31
 * @Description  : 
 */
#ifndef __SYSTEM_INTERFACE_H
#define __SYSTEM_INTERFACE_H

#include "platform.h"

#define SYSTEM_REQUEST_MAX_COUNT    (2)
#define SYSTEM_REQUEST_MAX_LEN      (32)
#define SYSTEM_RESPONSE_LEN         (256u)

#define SYS_PKG_HEADER_MAGIC        (0xAA5555AA)

#define SYS_PKG_HEADER_OFFSET   0
#define SYS_PKG_HEADER_LENGTH   4
#define SYS_PKG_LEN_OFFSET      4
#define SYS_PKG_LEN_LENGTH      2
#define SYS_PKG_SEQUENCE_ID_OFFSET  6
#define SYS_PKG_SEQUENCE_ID_LENGTH  2
#define SYS_MSG_TYPE_OFFSET     8
#define SYS_MSG_TYPE_LENGTH     1
#define SYS_MSG_CODE_OFFSET     9
#define SYS_MSG_CODE_LENGTH     1
#define SYS_MSG_LEN_OFFSET      10
#define SYS_MSG_LEN_LENGTH      2
#define SYS_MSG_DATA_OFFSET     12
#define SYS_MSG_RES_CHK_OFFSET (SYSTEM_RESPONSE_LEN-1)
#define SYS_MSG_CHK_LENGTH      1
#define SYS_MSG_FORMAT_LENGTH   (SYS_PKG_HEADER_LENGTH + SYS_MSG_TYPE_LENGTH + SYS_MSG_CODE_LENGTH + SYS_MSG_LEN_LENGTH + SYS_MSG_CHK_LENGTH + SYS_PKG_LEN_LENGTH + SYS_PKG_SEQUENCE_ID_LENGTH)
#define SYS_MSG_RES_LENGTH      (SYSTEM_RESPONSE_LEN-SYS_MSG_FORMAT_LENGTH)
#define SYS_MSG_REQ_LENGTH      (SYSTEM_REQUEST_MAX_LEN-SYS_MSG_FORMAT_LENGTH)

#define SYS_MSG_TYPE_REQ        0x01
#define SYS_MSG_TYPE_RES        0x02
#define SYS_CMD_DEVICE_LIST     0x01
#define SYS_CMD_DEVICE_SENSOR   0x02
#define SYS_CMD_GET_EVENT       0x81
#define SYS_CMD_UNKNOW          0xFF

typedef struct {
    uint16_t msg_len;
    uint8_t msg[SYSTEM_REQUEST_MAX_LEN];
}sys_req_msg_t;

uint8_t init_sysInterface(void);

#endif // !__SYSTEM_INTERFACE_H
