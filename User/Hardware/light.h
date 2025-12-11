#ifndef __LIGHT_H
#define __LIGHT_H

#include "stm32f10x.h"

// 光敏电阻-照度表（ohm → lux）
typedef struct {
    uint16_t ohm;
    uint16_t lux;
} PhotoRes_TypeDef;

extern const PhotoRes_TypeDef GL5516[281];

// 初始化 ADC1 (PA1)
void Light_ADC_Init(void);

// 获取原始 ADC 值（0~4095）
uint16_t Light_ADC_GetValue(void);

// 获取光照强度（lux），带线性插值
uint16_t Light_GetLux(void);

#endif
