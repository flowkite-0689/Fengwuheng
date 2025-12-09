#ifndef __DELAY_H
#define __DELAY_H

#include "stm32f10x.h"
#include "stm32f10x_tim.h"

void TIM2_Delay_Init(void);
void TIM2_Delay_us(uint32_t us);
void Delay_us(uint32_t us);
void Delay_ms(uint32_t ms);
void Delay_s(uint32_t s);
void delay_ms(uint32_t ms);
void delay_us_no_irq(uint32_t us);
#endif
