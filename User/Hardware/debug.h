#ifndef DEBUG_H
#define DEBUG_H

#include "stm32f10x.h"                  // Device header
#include <stdio.h>

void debug_init(void);
void Usart1_Send_Sring(char *string);
void Usart1_send_bytes(uint8_t *buf, uint16_t len);

#endif
