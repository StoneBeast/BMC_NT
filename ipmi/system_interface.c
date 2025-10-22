/*** 
 * @Author       : stoneBeast
 * @Date         : 2025-08-14 13:48:04
 * @Encoding     : UTF-8
 * @LastEditors  : stoneBeast
 * @LastEditTime : 2025-10-22 17:06:45
 * @Description  : 
 */
#include "system_interface.h"
#include "ipmi.h"
#include "ipmi_event.h"
#include "ipmi_protocol.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <string.h>

/*
    System Interface Request Packeg
    | Packeg Length 32 Bytes |                   |                        |                 |                 |                   |               |                 |
    | Pkg Header(4Byte)      | Pkg Length(2Byte) | Pkg Sequence ID(2Byte) | Msg Type(1Byte) | Msg Code(1Byte) | Msg Length(2Byte) | Response Body | Checksum(1Byte) |

    System Interface Responst Packeg
    | Packeg Length 256 Bytes |                   |                        |                  |                  |                    |               |                  |
    | Pkg Header(4Byte)       | Pkg Length(2Byte) | Pkg Sequence ID(2Byte) | Msg Type(1 Byte) | Msg Code(1 Byte) | Msg Length(2 Byte) | Response Body | Checksum(1 Byte) |
*/

static uint8_t check_sys_req(const uint8_t* msg);
static void get_chksum(uint8_t * const msg);
static void sys_request_handler_task_func(void* arg);
static uint16_t get_device_list_handler(uint8_t *const res_body);
static uint16_t get_sensor_list_handler(uint8_t target_addr, uint8_t *const res_body);
static uint16_t get_event_handler(uint8_t *const res_body);
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
    uint16_t res_body_len = 0;
    uint16_t i = 0;

    while (1) {
        recv_ret = xQueueReceive(sys_req_queue, &recv_req, portMAX_DELAY);
        if (recv_ret != pdPASS ) {
            OS_PRINTF("recv error ?!\r\n");
        } else {
            OS_PRINTF("recv sys_req\r\n");
        }
        
        for (i = 0; i < recv_req.msg_len; i++)
        {
            OS_PRINTF(" %#02x ", recv_req.msg[i]);
        }
        OS_PRINTF("\r\n");
        

        if (0 != check_sys_req(&(recv_req.msg[SYS_MSG_TYPE_OFFSET]))) {
            OS_PRINTF("check error\r\n");
            continue;
        }

        memset(res_msg, 0, SYSTEM_RESPONSE_LEN);
        *((uint32_t*)res_msg) = SYS_PKG_HEADER_MAGIC;
        ((uint16_t*)(&(res_msg[SYS_PKG_LEN_OFFSET])))[0] = (SYSTEM_RESPONSE_LEN-SYS_PKG_HEADER_LENGTH-SYS_PKG_LEN_LENGTH-SYS_MSG_CHK_LENGTH);
        memcpy(&(res_msg[SYS_PKG_SEQUENCE_ID_OFFSET]), &(recv_req.msg[SYS_PKG_SEQUENCE_ID_OFFSET]), SYS_PKG_SEQUENCE_ID_LENGTH);
        res_msg[SYS_MSG_TYPE_OFFSET] = SYS_MSG_TYPE_RES;

        res_msg[SYS_MSG_CODE_OFFSET] = recv_req.msg[SYS_MSG_CODE_OFFSET];

        switch (recv_req.msg[SYS_MSG_CODE_OFFSET])
        {
        case SYS_CMD_DEVICE_LIST:
            OS_PRINTF("CMD: device list\r\n");
            res_body_len = get_device_list_handler(&(res_msg[SYS_MSG_DATA_OFFSET]));
            break;
        case SYS_CMD_DEVICE_SENSOR:
            OS_PRINTF("CMD: device sensor\r\n");
            res_body_len = get_sensor_list_handler(recv_req.msg[SYS_MSG_DATA_OFFSET], &(res_msg[SYS_MSG_DATA_OFFSET]));
            break;
        case SYS_CMD_GET_EVENT:
            OS_PRINTF("CMD: get event\r\n");
            res_body_len = get_event_handler(&(res_msg[SYS_MSG_DATA_OFFSET]));
            break;
        default:
            OS_PRINTF("CMD: unkonw\r\n");
            res_msg[SYS_MSG_CODE_OFFSET] = SYS_CMD_UNKNOW;
            res_msg[SYS_MSG_DATA_OFFSET] = recv_req.msg[SYS_MSG_CODE_OFFSET];
            res_body_len = SYS_MSG_CODE_LENGTH;
            break;
        }

        if (res_body_len != 0)
            memcpy(&(res_msg[SYS_MSG_LEN_OFFSET]), &res_body_len, SYS_MSG_LEN_LENGTH);

        get_chksum(res_msg);
        sys_response(res_msg);

    }
}

static uint16_t get_device_list_handler(uint8_t *const res_body)
{
    uint8_t device_count = 0;
    uint16_t res_len = 0;

    device_count = scan_card(&(res_body[1]));
    res_len = device_count;

    res_body[0] = device_count;

    OS_PRINTF("device_count: %d\r\n", device_count);

    return res_len;
}

static uint16_t get_sensor_list_handler(uint8_t target_addr, uint8_t *const res_body)
{
    uint16_t res_body_len = 0;
    uint16_t p = 1;
    ipmi_sdr sdr;
    uint8_t id = 0;
    uint8_t sdr_num = 0;

    do
    {
        id = get_card_sdr_by_id(target_addr, id, &sdr);

        /* no 1Byte */
        res_body[p++] = sdr.sensor_no;
        /* signed 1Byte */
        res_body[p++] = sdr.is_signed;
        /* raw data 2Byte */
        memcpy(&(res_body[p]), &(sdr.read_data), 2);
        p += 2;
        /* M 2Byte, K 2Byte */
        memcpy(&(res_body[p]), &(sdr.argM), 2);
        p += 2;
        memcpy(&(res_body[p]), &(sdr.argK), 2);
        p += 2;
        /* unit 1Byte */
        res_body[p++] = sdr.unit_code;
        /* name len 1Byte */
        res_body[p++] = sdr.name_len;
        /* sensor name */
        memcpy(&(res_body[p]), sdr.sensor_name, sdr.name_len);
        p += sdr.name_len;

        sdr_num += 1;

        OS_PRINTF("addr: %#02x, no: %d, sensor_type: %#02x, signed: %d\r\n", sdr.ipmc_addr, sdr.sensor_no, sdr.sensor_type, sdr.is_signed);
        OS_PRINTF("unit: %#02x, namelen: %d, name: %s\r\n", sdr.unit_code, sdr.name_len, sdr.sensor_name);
    } while (id != 0);
    
    res_body[0] = sdr_num;
    res_body_len = p;

    return res_body_len;
}

static uint16_t get_event_handler(uint8_t *const res_body)
{
    int ret = 0;
    ipmi_event event;
    uint16_t p = 1;
    uint8_t event_count = 0;
    uint16_t body_len = 1;
    uint16_t max_trans_count = 0;

    max_trans_count = (SYS_MSG_RES_LENGTH) / (sizeof(ipmi_event));
    OS_PRINTF("Max Transmit event count: %d\r\n", max_trans_count);

    while (ret == 0 && (event_count < max_trans_count))
    {
        ret = get_event_item(&event);
        if (ret == 0) {
            OS_PRINTF("get event item\r\n");
            memcpy(&(res_body[p]), &event, sizeof(ipmi_event));
            p += sizeof(ipmi_event);
            event_count += 1;
        } else {
            OS_PRINTF("no item\r\n");
        }
    }

    res_body[0] = event_count;
    body_len += event_count*sizeof(ipmi_event);

    return body_len;
}

static uint8_t check_sys_req(const uint8_t* msg)
{
    uint16_t sum = 0;
    uint16_t i = 0;

    for (i = 0; i < (SYSTEM_REQUEST_MAX_LEN-SYS_PKG_HEADER_LENGTH-SYS_PKG_LEN_LENGTH-SYS_PKG_SEQUENCE_ID_LENGTH); i++)
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

    for (uint16_t i = SYS_PKG_SEQUENCE_ID_OFFSET; i < (SYSTEM_RESPONSE_LEN-SYS_PKG_HEADER_LENGTH-SYS_PKG_LEN_LENGTH); i++)
        sum += msg[i];

    chk = (0x100 - sum % 0x100);
    msg[SYS_MSG_RES_CHK_OFFSET] = chk;
    OS_PRINTF("chk: %#02x\r\n", chk);
}

static void sys_response(const uint8_t* msg)
{
    usart_start_send(msg);

    OS_PRINTF("Response Sent \r\n");

    while (usart_is_send_complate() == 0)
        vTaskDelay(pdMS_TO_TICKS(5));
}
