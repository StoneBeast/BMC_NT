#include "platform.h"

static void blink_task_func (void* arg);
static void print_task_func (void* arg);

int main(void)
{   
    init_gpio();
    init_uart();
    
    xTaskCreate(blink_task_func, "blink", 128, NULL, 1, NULL);
    xTaskCreate(print_task_func, "print", 128, NULL, 1, NULL);
    
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

static void print_task_func (void* arg)
{
    while (1) {
        PRINTF("hello --new platform\r\n");
        vTaskDelay(500);
    }
}
