/*** 
 * @Author       : stoneBeast
 * @Date         : 2025-08-14 13:48:04
 * @Encoding     : UTF-8
 * @LastEditors  : stoneBeast
 * @LastEditTime : 2025-08-15 14:30:55
 * @Description  : 
 */
#include "system_interface.h"
#include "ipmi.h"
#include "ipmi_protocol.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <string.h>

/*
    System Interface Request Packeg
    |Packeg Length 32 Bytes
    | Msg Type(1 Byte)        | Msg Code(1 Byte) | Msg Length(2 Byte) | Response Body(27 Byte Max) | Checksum(1 Byte) |

    System Interface Responst Packeg
    | Packeg Length 256 Bytes |                  |                    |                             |                  |
    | Msg Type(1 Byte)        | Msg Code(1 Byte) | Msg Length(2 Byte) | Response Body(251 Byte Max) | Checksum(1 Byte) |
*/

// TODO: 当前为了兼容当前版本上位机程序，sys请求长度为变长。后续升级则最好修改为定长

static uint8_t check_sys_req(const uint8_t* msg);
static void get_chksum(uint8_t * const msg);
static void sys_request_handler_task_func(void* arg);
static int get_device_list_handler(uint8_t *const res_body);
static void sys_response(const uint8_t* msg);
QueueHandle_t sys_req_queue;

uint8_t init_sysInterface(void)
{
    BaseType_t ret;
    init_sys_usart();

    ret = xTaskCreate(sys_request_handler_task_func, "sys", 512, NULL, 4, NULL);
    if (ret != pdPASS) {
        PRINTF("sys handler task create err\r\n");
        return 0; 
    }
    
    sys_req_queue = xQueueCreate(SYSTEM_REQUEST_MAX_COUNT, sizeof(sys_req_msg_t));
    if (sys_req_queue == NULL) {
        OS_PRINTF("sys req queue create err\r\n");
        return 0; 
    }

    return 1;
}

static void sys_request_handler_task_func(void *arg)
{
    (void)arg;
    sys_req_msg_t recv_req;
    BaseType_t recv_ret = pdFALSE;
    uint8_t res_msg[SYSTEM_RESPONSE_LEN] = {0};
    // DEBUG:
    // uint8_t res_msg[SYSTEM_RESPONSE_LEN] = "Recv Request, This is Responst.\r\n";
    int res_ready = IPMI_ERR_OK;

    while (1) {
        recv_ret = xQueueReceive(sys_req_queue, &recv_req, portMAX_DELAY);
        if (recv_ret != pdPASS ) {
            OS_PRINTF("recv error ?!\r\n");
        } else {
            OS_PRINTF("recv sys_req\r\n");
        }
        
        // TODO: check request
        if (0 != check_sys_req(recv_req.msg)) {
            OS_PRINTF("check error\r\n");
            continue;
        }

        memset(res_msg, 0, SYSTEM_RESPONSE_LEN);
        res_msg[SYS_MSG_TYPE_OFFSET] = SYS_MSG_TYPE_RES;

        // TODO: switch
        switch (recv_req.msg[SYS_MSG_CODE_OFFSET])
        {
        case SYS_CMD_DEVICE_LIST:
            res_ready = get_device_list_handler(&(res_msg[SYS_MSG_LEN_OFFSET]));
            break;
        case SYS_CMD_DEVICE_SENSOR:
            break;
        case SYS_CMD_GET_EVENT:
            break;
        default:
            break;
        }

        if (res_ready == IPMI_ERR_OK) {
            res_msg[SYS_MSG_CODE_OFFSET] = recv_req.msg[SYS_MSG_CODE_OFFSET];
            get_chksum(res_msg);
            sys_response(res_msg);
        }

    }
}

static int get_device_list_handler(uint8_t *const res_body)
{
    uint8_t device_count = 0;
    uint16_t res_len = 0;

    device_count = scan_card(&(res_body[3]));
    res_len = device_count+1;

    memcpy(res_body, &res_len, SYS_MSG_LEN_LENGTH);
    res_body[2] = device_count;

    // DEBUG:
    OS_PRINTF("device_count: %d\r\n", device_count);

    return IPMI_ERR_OK;
}

static uint8_t check_sys_req(const uint8_t* msg)
{
    uint16_t sum = 0;
    uint16_t i = 0;

    for (i = 0; i < SYSTEM_REQUEST_MAX_LEN; i++)
    {
        sum += msg[i];
    }

    if (sum % 0x100 != 0) /* 校验失败 */
        return 1;

    /* 校验成功 */
    return 0;
}

static void get_chksum(uint8_t * const msg)
{
    uint16_t sum = 0;
    uint8_t chk = 0;

    for (uint16_t i = 0; i < SYSTEM_RESPONSE_LEN; i++)
        sum += msg[i];

    chk = (0x100 - sum % 0x100);
    msg[SYS_MSG_RES_CHK_OFFSET] = chk;
}

static void sys_response(const uint8_t* msg)
{
    usart_start_send(msg);

    while (usart_is_send_complate() == 0)
        vTaskDelay(pdMS_TO_TICKS(5));
}
