#include "platformConfig.h"
#include "platform.h"
#include "ipmi_protocol.h"
#include "task.h"

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

        vTaskNotifyGiveIndexedFromISR(bmc_task_handle, 1, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void DMA1_Channel7_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (DMA_GetITStatus(DMA1_IT_TC7) != RESET) {
        /* 清除中断标志 */
        DMA_ClearITPendingBit(DMA1_IT_TC7);

        I2C_it_switch(1);
        I2C_dma_switch(0);

        vTaskNotifyGiveIndexedFromISR(bmc_task_handle, 0, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}
