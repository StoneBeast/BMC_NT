
/*** 
 * @Author       : stoneBeast
 * @Date         : 2025-07-29 15:14:19
 * @Encoding     : UTF-8
 * @LastEditors  : stoneBeast
 * @LastEditTime : 2025-08-06 16:22:06
 * @Description  : 
 */

#include "ipmi_protocol.h"
#include "i2c.h"
#include "task.h"
#include <string.h>
#include "message_buffer.h"

/* 
    IPMI Message Package
    | LENGTH 32Byte   |                 |                    |      |                  |
    | Msg Type(1Byte) | Msg Code(1Byte) | Data Length(1Byte) | Data | Check sum(1Byte) |
*/

// TODO: send和recv的接收策略需要进一步确认
/* 
    FIXME:  接收时，主机因故不产生stop标志，导致总线锁死，clk线被一直拉低。之后即使总线复位，但仍不产生
            stop位，会导致接收端继续阻塞，因为busy被置位，这时的解决方法目前只发现重置接收端的I2C控制器
*/
// TODO: busy次数到达设定值之后最好进行外设复位，防止总线无限锁死。

#define RECV_FROM_MSG_BUFFER(p_data, wait_ms) \
    xMessageBufferReceive(req_msgBuffer, p_data, IPMI_PROTOCOL_MAX_LEN, pdMS_TO_TICKS(wait_ms));

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

/*** 
 * @brief ipmi请求函数
 * @param addr [uint8_t]        请求地址
 * @param code [uint8_t]        请求代码
 * @param msg [uint8_t*]        请求体
 * @param msg_len [uint16_t]    请求体长度
 * @param timeout_ms [uint32_t] 超时时间，单位ms
 * @param res_buf [uint8_t *]   响应：1Byte长度+nByte响应体
 * @return [int]                成功返回IPMI_ERR_OK或错误码
 */
int ipmi_request(uint8_t addr, uint8_t code, const uint8_t* msg, uint16_t msg_len, uint32_t timeout_ms, uint8_t *const res_buf)
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
        memcpy(res_buf, &(temp_req_buf[IPMI_PROTOCOL_MSG_DATA_LEN_OFFSET]), (IPMI_PROTOCOL_MSG_DATA_LEN_LEN + temp_req_buf[IPMI_PROTOCOL_MSG_DATA_LEN_OFFSET]));
        return IPMI_ERR_OK;
    }

    return IPMI_ERR_RES_TIMEOUT;
}

/***
 * @brief 发送ipmi消息
 * @param addr [uint8_t]        目标地址
 * @param type [uint8_t]        消息类型代码
 * @param code [uint8_t]        消息功能码
 * @param msg [uint8_t*]        消息体
 * @param msg_len [uint16_t]    消息体长度
 * @param timeout_ms [uint32_t] 超时时间，单位ms
 * @return [int]                成功返回IPMI_ERR_OK或错误码
 */
static int ipmi_msg_send(uint8_t addr, uint8_t type, uint8_t code, const uint8_t* msg, uint16_t msg_len, uint32_t timeout_ms)
{
    int ret = IPMI_ERR_OK;
    int i2c_ret = I2C_ERR_OK;
    uint32_t send_ret = pdFALSE;
    uint16_t try_count = 0;
    uint8_t send_msg[IPMI_PROTOCOL_MAX_LEN] = {0};

    /* 判断msg */
    if (msg_len > IPMI_PROTOCOL_DATA_MAX_LEN) {
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
    if (msg != NULL)
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

/*** 
 * @brief 校验消息是否有误
 * @param msg [uint8_t*]    被校验的消息  
 * @return [int]            0：校验通过
 */
int check_msg(const uint8_t* msg)
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

/*** 
 * @brief 向消息中直接添加校验码
 * @param msg [uint8_t*]    指向消息的指针
 * @return [void]
 */
static void get_chksum(uint8_t *msg)
{
    uint16_t sum = 0;
    uint8_t chk = 0;

    for (uint16_t i = 0; i < (IPMI_PROTOCOL_MAX_LEN-1); i++)
        sum += msg[i];

    chk = (0x100 - sum % 0x100);

    msg[IPMI_PROTOCOL_MSG_CHK_OFFSET] = chk;
}
