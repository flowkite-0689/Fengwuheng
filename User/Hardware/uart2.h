#ifndef UART2_H
#define UART2_H
#include "stm32f10x.h"
// 定义UART2接收缓冲区（128字节）
#define UART2_BUF_SIZE 128
void UART2_DMA_RX_Init(uint32_t baudrate);
uint8_t UART2_SendDataToWiFi_Poll(uint8_t *data, uint16_t len);
#endif
