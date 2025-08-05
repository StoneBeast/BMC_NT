
/*** 
 * @Author       : stoneBeast
 * @Date         : 2025-07-29 15:14:19
 * @Encoding     : UTF-8
 * @LastEditors  : stoneBeast
 * @LastEditTime : 2025-08-05 17:04:47
 * @Description  : 
 */

#include "ipmi_protocol.h"
#include "i2c.h"
#include "task.h"
#include <string.h>
#include "message_buffer.h"

// TODO: send和recv的接收策略需要进一步确认

#define RECV_FROM_MSG_BUFFER(p_data, wait_ms) \
    xMessageBufferReceive(req_msgBuffer, p_data, IPMI_PROTOCOL_MAX_LEN, pdMS_TO_TICKS(wait_ms));

static int check_msg(const uint8_t *msg);
static void get_chksum(uint8_t* msg);
static int ipmi_msg_send(uint8_t addr, uint8_t type, uint8_t code, const uint8_t* msg, uint16_t msg_len, uint32_t timeout_ms);


MessageBufferHandle_t req_msgBuffer;
extern uint8_t ipmi_recv_buf[IPMI_PROTOCOL_MAX_LEN];

/*** 
 * @brief 准备ipmi通信需要使用到的结构体、空间等
 * @return [void]
 */
void init_ipmi_protocol(void)
{
    req_msgBuffer = xMessageBufferCreate(IPMI_PROTOCOL_MAX_LEN+8);
}

int ipmi_request(uint8_t addr, uint8_t code, const uint8_t* msg, uint16_t msg_len, uint32_t timeout_ms, uint8_t * req_buf)
{
    int ret = 0;
    uint8_t temp_req_buf[IPMI_PROTOCOL_MAX_LEN] = {0};

    /* 发送请求 */
    ret = ipmi_msg_send(addr, IPMI_MSG_TYPE_REQ, code, msg, msg_len, timeout_ms);
    if (ret != IPMI_ERR_OK)
        return ret;

    /* 等待接收响应 */
    ret = RECV_FROM_MSG_BUFFER(temp_req_buf, timeout_ms);
    if (ret != IPMI_PROTOCOL_MAX_LEN)
        return IPMI_ERR_RES_TIMEOUT;

    if (temp_req_buf[IPMI_PROTOCOL_MSG_CODE_OFFSET] == code) { /* 确认是当前请求的响应 */
        if (check_msg(temp_req_buf) == 0) { /* 消息校验通过 */
            memcpy(req_buf, &(temp_req_buf[IPMI_PROTOCOL_MSG_DATA_LEN_OFFSET]), (IPMI_PROTOCOL_MSG_DATA_LEN_LEN + temp_req_buf[IPMI_PROTOCOL_MSG_DATA_LEN_OFFSET]));
            return IPMI_ERR_OK;
        }
    }

    return IPMI_ERR_RES_TIMEOUT;
}

static int ipmi_msg_send(uint8_t addr, uint8_t type, uint8_t code, const uint8_t* msg, uint16_t msg_len, uint32_t timeout_ms)
{
    int ret = IPMI_ERR_OK;
    int i2c_ret = I2C_ERR_OK;
    uint32_t send_ret = pdFALSE;
    uint16_t try_count = 0;
    uint8_t send_msg[IPMI_PROTOCOL_MAX_LEN] = {0};

    /* 判断msg */
    if (msg == NULL || msg_len > IPMI_PROTOCOL_DATA_MAX_LEN) {
        return IPMI_ERR_MSG_ERROR;
    }

    /* 先判断I2C是否busy */
    while (1 == I2C_busy_status()) {
        if (try_count > IPMB_BUSY_TRY_COUNT) {
            ret = IPMI_ERR_BUSY;
            goto SEND_END;
        }
        vTaskDelay(pdMS_TO_TICKS(IPMB_BUSY_TRY_INTERVAL_MS));
        try_count++;
    }

    /* 填充IPMI消息固定成员 */
    memcpy(&(send_msg[IPMI_PROTOCOL_MSG_TYPE_OFFSET]), &type, IPMI_PROTOCOL_MSG_TYPE_LEN);
    memcpy(&(send_msg[IPMI_PROTOCOL_MSG_CODE_OFFSET]), &code, IPMI_PROTOCOL_MSG_CODE_LEN);
    memcpy(&(send_msg[IPMI_PROTOCOL_MSG_DATA_LEN_OFFSET]), &msg_len, IPMI_PROTOCOL_MSG_DATA_LEN_LEN);
    memcpy(&(send_msg[IPMI_PROTOCOL_MSG_DATA_OFFSET]), msg, msg_len);
    /* 添加校验和 */
    get_chksum(send_msg);

    i2c_ret = I2C_satrt_send(addr, send_msg);
    if (i2c_ret == I2C_ERR_SB) {
        ret = IPMI_ERR_TIMEOUT;
        goto SEND_END;
    } else if (i2c_ret == I2C_ERR_ADDR) {
        ret = IPMI_ERR_NO_DEVICE;
        I2C_reset();
        goto SEND_END;
    }

    send_ret = ulTaskNotifyTakeIndexed(IPMI_SEND_CMP_BIT, pdTRUE, pdMS_TO_TICKS(timeout_ms));
    if (send_ret != pdTRUE) {
        ret = IPMI_ERR_TIMEOUT;
    }

SEND_END:
    return ret;
}

// int ipmi_msg_recv(uint8_t * const msg, uint32_t timeout_ms)
// {
//     int ret = IPMI_ERR_OK;
//     uint32_t recv_ret = pdFALSE;
//     uint16_t try_count = 0;

//     /* 先判断I2C是否busy */
//     while (1 == I2C_busy_status()) {
//         if (try_count > IPMB_BUSY_TRY_COUNT) {
//             ret = IPMI_ERR_BUSY;
//             goto RECV_END;
//         }
//         vTaskDelay(pdMS_TO_TICKS(IPMB_BUSY_TRY_INTERVAL_MS));
//         try_count++;
//     }
    
//     /* 打开I2C中断，准备接收 */
//     // I2C_it_switch(1);
//     // I2C_dma_switch(0);

//     recv_ret = ulTaskNotifyTakeIndexed(0, pdTRUE, pdMS_TO_TICKS(timeout_ms));

//     if (recv_ret == pdTRUE) {
//         memcpy(msg, ipmi_recv_buf, IPMI_PROTOCOL_MAX_LEN);
//         /* 重新打开I2C中断，准备接收 */
//         // I2C_it_switch(1);
//         // I2C_dma_switch(0);
//     } else {
//         ret = IPMI_ERR_TIMEOUT;
//     }

// RECV_END:
//     return ret;
// }

static int check_msg(const uint8_t* msg)
{
    uint16_t sum = 0;
    uint8_t i = 0;

    for (i = 0; i < IPMI_PROTOCOL_MAX_LEN; i++) {
        sum += msg[i];
    }

    if (sum%256 == 0) {
        return 0;
    } else {
        return -1;
    }

}

static void get_chksum(uint8_t *msg)
{
    uint16_t sum = 0;
    uint8_t chk = 0;

    for (uint16_t i = 0; i < (IPMI_PROTOCOL_MAX_LEN-1); i++)
        sum += msg[i];

    chk = (0x100 - sum % 0x100);

    msg[IPMI_PROTOCOL_MSG_CHK_OFFSET] = chk;
}
