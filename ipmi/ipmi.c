/*** 
 * @Author       : stoneBeast
 * @Date         : 2025-07-29 15:15:04
 * @Encoding     : UTF-8
 * @LastEditors  : stoneBeast
 * @LastEditTime : 2025-08-06 10:51:32
 * @Description  : 
 */

#include "platform.h"
#include "ipmi.h"
#include "ipmi_protocol.h"
#include "ipmi_event.h"

void bmc_init(void)
{
    int init_ret = 0;

    init_ipmi_i2c(BMC_ADDR);
    init_ipmi_protocol();
    init_ret = init_ipmi_event();
    if (init_ret != 0)
        while(1);

    init_ipmi_sdr();
}

// DEBUG: 测试函数，测试获取目标ipmc的所有sdr功能
void get_all_sdr(uint8_t addr)
{
    int ret = 0;
    uint8_t target_id = 0;
    uint8_t res_body[IPMI_PROTOCOL_DATA_MAX_LEN] = {0};
    ipmi_sdr *sdr = NULL;
    
    do {
        ret = ipmi_request(addr, IPMI_MSG_CODE_GET_SDR, &target_id, 1, 2000, res_body);

        if (ret == IPMI_ERR_OK) {
            target_id = res_body[1];
            OS_PRINTF("recv ok, next id: %#02x\r\n", res_body[1]);
            sdr = (ipmi_sdr*)(&(res_body[2]));
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
