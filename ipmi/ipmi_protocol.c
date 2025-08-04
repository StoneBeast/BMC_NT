
/*** 
 * @Author       : stoneBeast
 * @Date         : 2025-07-29 15:14:19
 * @Encoding     : UTF-8
 * @LastEditors  : stoneBeast
 * @LastEditTime : 2025-08-04 13:45:45
 * @Description  : 
 */

#include "ipmi_protocol.h"
#include "i2c.h"
#include "task.h"
#include <string.h>

// TODO: send和recv的接收策略需要进一步确认

extern uint8_t ipmi_recv_buf[IPMI_PROTOCOL_MAX_LEN];

int ipmi_msg_send(uint8_t addr, const uint8_t* msg, uint32_t timeout_ms)
{
    int ret = IPMI_ERR_OK;
    int i2c_ret = I2C_ERR_OK;
    uint32_t send_ret = pdFALSE;
    uint16_t try_count = 0;

    /* 先判断I2C是否busy */
    while (1 == I2C_busy_status()) {
        if (try_count > IPMB_BUSY_TRY_COUNT) {
            ret = IPMI_ERR_BUSY;
            goto SEND_END;
        }
        vTaskDelay(pdMS_TO_TICKS(IPMB_BUSY_TRY_INTERVAL_MS));
        try_count++;
    }

    i2c_ret = I2C_satrt_send(addr, msg);
    if (i2c_ret == I2C_ERR_SB) {
        ret = IPMI_ERR_TIMEOUT;
        goto SEND_END;
    } else if (i2c_ret == I2C_ERR_ADDR) {
        ret = IPMI_ERR_NO_DEVICE;
        I2C_reset();
        goto SEND_END;
    }

    send_ret = ulTaskNotifyTakeIndexed(1, pdTRUE, pdMS_TO_TICKS(timeout_ms));
    if (send_ret != pdTRUE) {
        ret = IPMI_ERR_TIMEOUT;
    }

SEND_END:
    return ret;
}

int ipmi_msg_recv(uint8_t * const msg, uint32_t timeout_ms)
{
    int ret = IPMI_ERR_OK;
    uint32_t recv_ret = pdFALSE;
    uint16_t try_count = 0;

    /* 先判断I2C是否busy */
    while (1 == I2C_busy_status()) {
        if (try_count > IPMB_BUSY_TRY_COUNT) {
            ret = IPMI_ERR_BUSY;
            goto RECV_END;
        }
        vTaskDelay(pdMS_TO_TICKS(IPMB_BUSY_TRY_INTERVAL_MS));
        try_count++;
    }
    
    /* 打开I2C中断，准备接收 */
    // I2C_it_switch(1);
    // I2C_dma_switch(0);

    recv_ret = ulTaskNotifyTakeIndexed(0, pdTRUE, pdMS_TO_TICKS(timeout_ms));

    if (recv_ret == pdTRUE) {
        memcpy(msg, ipmi_recv_buf, IPMI_PROTOCOL_MAX_LEN);
        /* 重新打开I2C中断，准备接收 */
        // I2C_it_switch(1);
        // I2C_dma_switch(0);
    } else {
        ret = IPMI_ERR_TIMEOUT;
    }

RECV_END:
    return ret;
}

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
