#ifndef __PM25_H
#define __PM25_H

#include "stm32f10x.h"

// PM2.5等级定义
#define PM25_LEVEL_GOOD      0    // 优(0-35 μg/m³)
#define PM25_LEVEL_MODERATE  1    // 良(36-75 μg/m³)
#define PM25_LEVEL_UNHEALTHY_SENSITIVE  2    // 轻度污染(76-115 μg/m³)
#define PM25_LEVEL_UNHEALTHY  3    // 中度污染(116-150 μg/m³)
#define PM25_LEVEL_VERY_UNHEALTHY  4    // 重度污染(151-250 μg/m³)
#define PM25_LEVEL_HAZARDOUS  5    // 严重污染(251+ μg/m³)

// PM2.5结构体
typedef struct {
    uint16_t pm25_value;      // PM2.5数值 (μg/m³)
    uint8_t  level;          // 污染等级 (0-5)
    float    voltage;         // ADC电压值 (V)
    uint16_t adc_raw;        // ADC原始值
} PM25_TypeDef;

// 函数声明
void PM25_Init(void);
uint16_t PM25_GetRawValue(void);
float PM25_GetVoltage(void);
float PM25_ReadPM25(void);
uint8_t PM25_GetLevel(void);
uint8_t PM25_GetLevelFromValue(float pm25_value);
const char* PM25_GetDescription(void);
const char* PM25_GetLevelDescription(void);

#endif /* __PM25_H */
