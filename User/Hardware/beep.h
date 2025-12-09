#ifndef _DEEP_H_
#define _DEEP_H_


#include "hardware_def.h"
#include "Delay.h"

void Beep_Init(void);


/**
 * @brief 蜂鸣器发声
 * @param duration_ms 持续时间（毫秒）
 * @note 控制蜂鸣器发声指定时间后自动停止
 */
void BEEP_Buzz( uint32_t duration_ms);

void Beep_ON(void);
void Beep_OFF(void);
#endif
