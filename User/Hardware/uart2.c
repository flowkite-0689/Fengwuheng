#include "uart2.h"
#include <stdio.h>

uint8_t uart2_buffer[UART2_BUF_SIZE];// uart2接收缓冲
uint8_t uart2_rx_len;                  // uart2接收长度

/**
 * @brief  GPIO初始化（PA2=TX2，PA3=RX2）
 */
void UART2_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    
    // 使能GPIOA时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    // PA2(TX2) 推挽复用输出
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // PA3(RX2) 浮空输入
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/**
 * @brief  UART2初始化（含空闲中断使能）
 * @param  baudrate: 波特率（如9600、115200）
 */
void UART2_Init(uint32_t baudrate)
{
    USART_InitTypeDef USART_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    
    // 1. 使能UART2时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    
    // 2. UART配置
    USART_InitStruct.USART_BaudRate = baudrate;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b; // 8位数据位
    USART_InitStruct.USART_StopBits = USART_StopBits_1;      // 1位停止位
    USART_InitStruct.USART_Parity = USART_Parity_No;         // 无校验
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件流控
    USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; // 收发模式
    USART_Init(USART2, &USART_InitStruct);
    
    // 3. 使能UART2空闲中断
    USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);
    
    // 4. 配置UART2中断优先级（NVIC）
    NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 6; // 抢占优先级6
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;        // 子优先级0
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
    
    // 5. 使能UART2
    USART_Cmd(USART2, ENABLE);
}

/**
 * @brief  DMA初始化（UART2_RX -> 缓冲区，循环模式）
 */
void UART2_DMA_Init(void)
{
    DMA_InitTypeDef DMA_InitStruct;
    
    // 1. 使能DMA1时钟（UART2_RX对应DMA1_Channel6）
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    
    // 2. 复位DMA1_Channel6（避免残留配置）
    DMA_DeInit(DMA1_Channel6);
    
    // 3. DMA配置
    DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&USART2->DR; // 外设地址：UART2数据寄存器
    DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)uart2_buffer;    // 内存地址：接收缓冲区
    DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;                // 数据方向：外设->内存
    DMA_InitStruct.DMA_BufferSize = UART2_BUF_SIZE;                // 缓冲区大小：128字节
    DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  // 外设地址不递增
    DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;           // 内存地址递增
    DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // 外设数据宽度：字节
    DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;   // 内存数据宽度：字节
    DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;                   // 循环模式（重复搬运）
    DMA_InitStruct.DMA_Priority = DMA_Priority_Medium;             // 中等优先级
    DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;                      // 禁用内存到内存
    
    DMA_Init(DMA1_Channel6, &DMA_InitStruct);
    
    // 4. 使能DMA1_Channel6
    DMA_Cmd(DMA1_Channel6, ENABLE);
    
    // 5. 使能UART2的DMA接收请求
    USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
}

/**
 * @brief  UART2中断服务函数（处理空闲中断）
 */
void USART2_IRQHandler(void)
{
    uint32_t temp;
    
    // 检查是否是空闲中断
    if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)
    {
        // 必须读取SR和DR寄存器清除空闲中断标志（标准库bug，需手动清）
        temp = USART2->SR;
        temp = USART2->DR;
        (void)temp; // 避免未使用变量警告
        
        // 停止DMA传输（临时）
        DMA_Cmd(DMA1_Channel6, DISABLE);
        
        // 计算接收到的数据长度
        uart2_rx_len = UART2_BUF_SIZE - DMA_GetCurrDataCounter(DMA1_Channel6);
        // printf("len=%d, uart2_buffer=", rx_len);
        // for(uint16_t i=0; i<rx_len; i++)
        // {
        //     printf("%c", uart2_buffer[i]);
        // }
        
        
        // 重新设置DMA缓冲区大小，恢复DMA传输（循环接收）
        DMA_SetCurrDataCounter(DMA1_Channel6, UART2_BUF_SIZE);
        DMA_Cmd(DMA1_Channel6, ENABLE);
    }
}

/**
 * @brief  初始化总入口
 */
void UART2_DMA_RX_Init(uint32_t baudrate)
{
    UART2_GPIO_Init();    // GPIO初始化
    UART2_Init(baudrate); // UART初始化（含空闲中断）
    UART2_DMA_Init();     // DMA初始化
}


/**
 * @brief  【轮询方式】UART2发送数据给WiFi模块（简单可靠，适合短数据）
 * @param  data: 待发送的数据缓冲区指针
 * @param  len:  发送数据的长度（字节）
 * @retval 0: 成功；1: 参数错误
 */
uint8_t UART2_SendDataToWiFi_Poll(uint8_t *data, uint16_t len)
{
    // 参数校验
    if(data == NULL || len == 0 || len > UART2_BUF_SIZE)
    {
        return 1;
    }
    
    // 轮询发送每个字节
    for(uint16_t i = 0; i < len; i++)
    {
        // 等待发送缓冲区为空（TXE标志置位）
        while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
        // 发送单个字节
        USART_SendData(USART2, data[i]);
    }
    
    // 等待最后一个字节发送完成（TC标志置位，确保数据全部发出）
    while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
    
    return 0;
}
 
