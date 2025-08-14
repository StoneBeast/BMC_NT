#include "platformConfig.h"
#include "platform.h"
#include "ipmi_protocol.h"
#include "task.h"
#include "message_buffer.h"
#include "queue.h"
#include "system_interface.h"
#include <string.h>

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

extern void I2C_dma_switch(uint8_t function);

void I2C1_EV_IRQHandler(void)
{
    if (SET == I2C_GetITStatus(IPMI_I2C, I2C_IT_ADDR)) {
        /* 匹配地址 */
        CLEAR_ADDRFLAG(IPMI_I2C);
        /* 检测到addr，方向为接收，关闭中断，开启dma，准备接收数据 */
        if (RESET == I2C_GetFlagStatus(IPMI_I2C, I2C_FLAG_TRA)) {
            I2C_it_switch(0);
            I2C_dma_switch(1);
        } else {    /* 方向为发送 */
            I2C_it_switch(0);
            DMA_Cmd(DMA1_Channel6, ENABLE);
        }
    }
    if (SET == I2C_GetITStatus(IPMI_I2C, I2C_IT_STOPF)) {
        /* 清除stop flag */
        I2C_Cmd(IPMI_I2C, ENABLE);
    }
}

void I2C1_ER_IRQHandler(void)
{
    if (SET == I2C_GetITStatus(IPMI_I2C, I2C_IT_TIMEOUT)) {
        PRINTF("timeout err\r\n");
        I2C_ClearITPendingBit(IPMI_I2C, I2C_IT_TIMEOUT);
    }
    else if (SET == I2C_GetITStatus(IPMI_I2C, I2C_IT_AF)) {
        PRINTF("af err\r\n");
        I2C_ClearITPendingBit(IPMI_I2C, I2C_IT_AF);
        I2C_GenerateSTOP(IPMI_I2C, ENABLE);
    }
    else if (SET == I2C_GetITStatus(IPMI_I2C, I2C_IT_BERR)){
        PRINTF("berr err\r\n");
        I2C_ClearITPendingBit(IPMI_I2C, I2C_IT_BERR);
    }
}

extern uint8_t ipmi_recv_buf[IPMI_PROTOCOL_MAX_LEN];
extern TaskHandle_t bmc_task_handle;

void DMA1_Channel6_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (DMA_GetITStatus(DMA1_IT_TC6) != RESET) {
        /* 清除中断标志 */
        DMA_ClearITPendingBit(DMA1_IT_TC6);

        /*
            BUG: DMA中传输数据设置为32Byte，当数据全部被从内存的buffer
                 中传输到I2C的DR中后，该中断被触发，之后马上发送停止位
                 而此时最后一个数据只是被放入的DR，而未被发送出去。因此
                 会造成丢失最后一个字节的现象。
                 在这里直接在中断中使用了阻塞的方式等待标志位置位，会有锁死的风险(几乎没有)
        */
        while (RESET == I2C_GetFlagStatus(IPMI_I2C, I2C_FLAG_BTF));

        /* 产生停止位 */
        I2C_GenerateSTOP(IPMI_I2C, ENABLE);

        /* 打开中断 */
        I2C_it_switch(1);
        DMA_Cmd(DMA1_Channel6, DISABLE);
        DMA_SetCurrDataCounter(DMA1_Channel6, IPMI_PROTOCOL_MAX_LEN);

        vTaskNotifyGiveIndexedFromISR(bmc_task_handle, IPMI_SEND_CMP_BIT, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

extern MessageBufferHandle_t req_msgBuffer;
extern QueueHandle_t event_queue;

/*** 
 * @brief DMA1_channel7 中断处理函数，负责I2C1的DMA接收
 * @return [void]
 */
void DMA1_Channel7_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    int check_ret = 0;

    if (DMA_GetITStatus(DMA1_IT_TC7) != RESET) {
        /* 清除中断标志 */
        DMA_ClearITPendingBit(DMA1_IT_TC7);

        I2C_it_switch(1);
        I2C_dma_switch(0);

        check_ret = check_msg(ipmi_recv_buf);

        if (check_ret == 0) {
            if (ipmi_recv_buf[IPMI_PROTOCOL_MSG_TYPE_OFFSET] == IPMI_MSG_TYPE_RES) {
                xMessageBufferSendFromISR(req_msgBuffer, ipmi_recv_buf, IPMI_PROTOCOL_MAX_LEN, &xHigherPriorityTaskWoken);
            } else if (ipmi_recv_buf[IPMI_PROTOCOL_MSG_TYPE_OFFSET] == IPMI_MSG_TYPE_EVENT) {
                xQueueSendFromISR(event_queue, &(ipmi_recv_buf[IPMI_PROTOCOL_MSG_DATA_OFFSET]), &xHigherPriorityTaskWoken);
            }
        }
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

extern QueueHandle_t sys_req_queue;
static sys_req_msg_t sys_req_msg = {.msg_len = 0};

void USART1_IRQHandler(void)
{
    uint8_t rc = 0;
    BaseType_t is_yield = pdFALSE;

    if (SET == USART_GetITStatus(USART1, USART_IT_RXNE)) {
        rc = USART_ReceiveData(USART1);
        if (sys_req_msg.msg_len < SYS_INTERFACE_MSG_MAX_LEN) {
            sys_req_msg.msg[sys_req_msg.msg_len] = rc;
            sys_req_msg.msg_len++;
        }
    }
    else if (SET == USART_GetITStatus(USART1, USART_IT_IDLE)) { /* idel 中断，接收完成？ */
        xQueueSendFromISR(sys_req_queue, &sys_req_msg, &is_yield);
        memset(&sys_req_msg, 0, sizeof(sys_req_msg));
        portYIELD_FROM_ISR(is_yield);
    }
}
