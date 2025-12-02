/*** 
 * @Author       : stoneBeast
 * @Date         : 2025-07-29 15:15:04
 * @Encoding     : UTF-8
 * @LastEditors  : stoneBeast
 * @LastEditTime : 2025-10-23 10:55:30
 * @Description  : 
 */

#include "platform.h"
#include "ipmi.h"
#include "ipmi_protocol.h"
#include "ipmi_event.h"
#include <string.h>

void bmc_init(void)
{
    int init_ret = 0;

    init_ipmi_i2c(BMC_ADDR);
    init_ipmi_protocol();
    init_ret = init_ipmi_event();
    if (init_ret != 0)
        while(1);

    BasicTimer_Init();
    init_sensor_i2c();
    init_ipmi_sdr();
}

uint8_t scan_card(uint8_t *const card_list)
{
    uint8_t i = 0;
    uint8_t card_count = 0;
    int ret = IPMI_ERR_OK;
    uint8_t res_body[IPMI_PROTOCOL_DATA_MAX_LEN] = {0};
    uint8_t card_addr = 0;

    /* 先添加本机 */
    card_list[card_count] = BMC_ADDR;
    card_count += 1;

    for (i = 0; i < SLOT_COUNT; i++) {
        card_addr = VPX_IPMB_ADDR((IPMC_BASE_ADDR + 1 + i));
        ret = ipmi_request(card_addr, IPMI_MSG_CODE_SCAN, NULL, 0, 2000, res_body);
        if (ret == IPMI_ERR_OK) {
            card_list[card_count] = card_addr;
            card_count += 1;
        }
    }

    return card_count;
}

uint8_t get_card_sdr_by_id(uint8_t addr, uint8_t id, ipmi_sdr *const sdr)
{
    uint8_t next_id = 0;
    int ret = IPMI_ERR_OK;
    uint8_t res_body[IPMI_PROTOCOL_DATA_MAX_LEN + 1] = {0};

    if (addr == BMC_ADDR) {
        next_id = get_sdr_by_id(id, sdr);
    } else {
        ret = ipmi_request(addr, IPMI_MSG_CODE_GET_SDR, &id, 1, 2000, res_body);
        if (ret == IPMI_ERR_OK) {
            next_id = res_body[1];
            memcpy(sdr, (ipmi_sdr*)(&(res_body[2])), sizeof(ipmi_sdr));
        } else if (ret == IPMI_ERR_BUSY) {
            I2C_reset();
        }
    }

    return next_id;
}

// FIX: 无法获取IPMC版本信息
uint16_t get_version_info(uint8_t addr, char* const ver_str)
{
    uint8_t res_body[32] = {0};
    int ret = 0;

    if (addr == BMC_ADDR) {
        sprintf(ver_str, "BMC Version %d.%d.%d. Built on %s %s", MAIN_VERSION, SUB_VERSION, FIX_VERSION, __DATE__, __TIME__);
        return strlen(ver_str);
    } else {
        ret = ipmi_request(addr, IPMI_MSG_CODE_GET_VERSION, NULL, 0, 2000, res_body);
        if (ret == IPMI_ERR_OK) {
            memcpy(ver_str, &(res_body[1]), res_body[0]);
            return res_body[0];
        } else if (ret == IPMI_ERR_BUSY) {
            I2C_reset();
        }
        return 0;
    }
}

// DEBUG: 测试函数，测试获取目标ipmc的所有sdr功能
void get_all_sdr(uint8_t addr)
{
    int ret = 0;
    uint8_t target_id = 0;
    uint8_t res_body[IPMI_PROTOCOL_DATA_MAX_LEN+1] = {0};
    ipmi_sdr *sdr = NULL;
    
    do {
        ret = ipmi_request(addr, IPMI_MSG_CODE_GET_SDR, &target_id, 1, 2000, res_body);

        if (ret == IPMI_ERR_OK) {
            target_id = res_body[1];
            OS_PRINTF("recv ok, next id: %#02x\r\n", res_body[1]);
            sdr = (ipmi_sdr*)(&(res_body[2]));
            is_over_value(sdr);
            OS_PRINTF("addr: %#02x, no: %d, sensor_type: %#02x, signed: %d\r\n", sdr->ipmc_addr, sdr->sensor_no, sdr->sensor_type, sdr->is_signed);
            OS_PRINTF("unit: %#02x, namelen: %d, name: %s\r\n", sdr->unit_code, sdr->name_len, sdr->sensor_name);
        } 
        else if (ret == IPMI_ERR_TIMEOUT){
            OS_PRINTF("recv timeout\r\n");
        }
        else if (ret == IPMI_ERR_BUSY) {
            // TODO: 需要判断是真busy还是总线被锁死
            OS_PRINTF("busy, reset\r\n");
            I2C_reset();
        }
        else if (ret == IPMI_ERR_NO_DEVICE) {
            OS_PRINTF("no device, already reset\r\n");
        }
        else if (ret == IPMI_ERR_MSG_ERROR) {
            OS_PRINTF("message error\r\n");
        }
        else if (ret == IPMI_ERR_RES_TIMEOUT) {
            OS_PRINTF("response timeout\r\n");
        }

        OS_PRINTF("\r\n");
    } while(target_id != 0);
}
