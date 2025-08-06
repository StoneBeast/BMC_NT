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
}
