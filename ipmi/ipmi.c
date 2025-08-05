/*** 
 * @Author       : stoneBeast
 * @Date         : 2025-07-29 15:15:04
 * @Encoding     : UTF-8
 * @LastEditors  : stoneBeast
 * @LastEditTime : 2025-08-04 18:31:17
 * @Description  : 
 */

#include "platform.h"
#include "ipmi.h"
#include "ipmi_protocol.h"

void bmc_init(void)
{
    init_ipmi_i2c(BMC_ADDR);
    init_ipmi_protocol();
}
