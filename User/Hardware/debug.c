#include "debug.h"
#include "FreeRTOS.h"
#include "task.h"
//7）在USART接收中断服务函数实现数据接收和发送。
void USART1_IRQHandler(void)
{   
    uint16_t  temp = 0;        
    //判断接收标志位是否置1
    if(USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
    {
        USART_ClearITPendingBit(USART1, USART_IT_RXNE); //清除接收标志位
        
        temp = USART_ReceiveData(USART1);    // 读取数据（读一个字节）
        if (temp == '1')
		{
			GPIO_ResetBits(GPIOB, GPIO_Pin_3);
			vTaskDelay(200);
			GPIO_SetBits(GPIOB, GPIO_Pin_3);
		}
	
        USART_SendData(USART1, temp);        // 将读到的数据发送回去
    }
}

// PA9-TX, PA10-RX
void debug_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    USART_InitTypeDef USART_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    
    // 1）使能RX和TX引脚GPIO时钟和USART时钟；
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);    // GPIO时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE); //for USART1 and USART6 

    // 2）初始化GPIO，并将GPIO复用到USART上
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;      // 复用推挽输出模式
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz; // 高速
 
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    
//    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
//    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

    // 3）配置USART参数；
    USART_InitStruct.USART_BaudRate = 115200;                        // 波特率
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;        // 8位有效数据位
    USART_InitStruct.USART_StopBits = USART_StopBits_1;              // 1位停止位
    USART_InitStruct.USART_Parity = USART_Parity_No;                // 无校验
    USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;    // 接收和发送数据模式
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件控制流
    USART_Init(USART1, &USART_InitStruct);

    // 4）配置中断控制器并使能USART接收中断
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    // 5）设置中断优先级（如果需要开启串口中断才需要这个步骤）
    NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;         // 中断通道(中断源)
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
    
    // 6）使能USART；
    USART_Cmd(USART1, ENABLE);
}

// 通过串口1，单片机发送字符串
void Usart1_Send_Sring(char *string)
{
    
    while(*string != '\0')
    {
        USART_SendData(USART1, *string++);	// 库函数
        // 等待发送数据寄存器空
        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    }
    // 等待发送完成
    while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
}

//重定向c库函数printf到串口，重定向后可使用printf函数
int fputc(int ch, FILE *f)
{
    /* 发送一个字节数据到串口 */
    USART_SendData(USART1, (uint8_t) ch);

    /* 等待发送完毕 */
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);

    return (ch);
}

// USART1发送字节数组函数
void Usart1_send_bytes(uint8_t *buf, uint16_t len)
{
    uint16_t i;
    for(i = 0; i < len; i++) {
        /* 发送一个字节数据到串口 */
        USART_SendData(USART1, buf[i]);
        
        /* 等待发送寄存器空 */
        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    }
    /* 等待发送完成 */
    while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
}
