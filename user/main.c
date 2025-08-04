/*** 
 * @Author       : stoneBeast
 * @Date         : 2025-07-29 14:33:46
 * @Encoding     : UTF-8
 * @LastEditors  : stoneBeast
 * @LastEditTime : 2025-08-04 18:24:57
 * @Description  : 
 */
#include "platform.h"
#include "ipmi.h"

// DEBUG
#include "ipmi_protocol.h"
#include <string.h>
static uint8_t temp_send[32] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D,
                                0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 
                                0x1D, 0x1E, 0x1F
                            };

static void blink_task_func (void* arg);
static void bmc_task_func(void *arg);

TaskHandle_t bmc_task_handle;

int main(void)
{
    BaseType_t ret;

    bmc_init();

    init_gpio();
    init_uart();
    
    ret = xTaskCreate(blink_task_func, "blink", 128, NULL, 1, NULL);
    if (ret != pdPASS) {
        PRINTF("blink create err\r\n");
    }
    ret = xTaskCreate(bmc_task_func, "bmc", 512, NULL, 4, &bmc_task_handle);
    if (ret != pdPASS) {
        PRINTF("bmc create err\r\n");
    }
    
    vTaskStartScheduler();
    
    while(1);
}

static void blink_task_func (void* arg)
{
    while (1) {
        ledOn();
        vTaskDelay(500);
        ledOff();
        vTaskDelay(500);
    }
}

static void bmc_task_func(void *arg)
{
    uint8_t msg[IPMI_PROTOCOL_MAX_LEN] = {0};
    int ret = 0;
    (void)arg;

    while(1) {
#if 0
        PRINTF("recv start\r\n");
        // ret = ulTaskNotifyTakeIndexed(0, pdTRUE, portMAX_DELAY);
        memset(msg, 0, IPMI_PROTOCOL_MAX_LEN);

        ret = ipmi_msg_recv(msg, 5000);
        if (ret == IPMI_ERR_OK) {
            PRINTF("recv ok\r\n");
        } 
        else if (ret == IPMI_ERR_TIMEOUT){
            PRINTF("recv timeout\r\n");
        }
        else if (ret == IPMI_ERR_BUSY) {
            PRINTF("busy, reset\r\n");
        //     I2C_reset();
        }

#else   // !0
        PRINTF("send start\r\n");

        ret = ipmi_msg_send(0x30, temp_send, 500);

        if (ret == IPMI_ERR_OK) {
            PRINTF("recv ok\r\n");
        } 
        else if (ret == IPMI_ERR_TIMEOUT){
            PRINTF("recv timeout\r\n");
        }
        else if (ret == IPMI_ERR_BUSY) {
            // TODO: 需要判断是真busy还是总线被锁死
            PRINTF("busy, reset\r\n");
            I2C_reset();
        }
        else if (ret == IPMI_ERR_NO_DEVICE) {
            PRINTF("no device, already reset\r\n");
        }
#endif  // !0

        vTaskDelay(5000);
    }
}
