#include "PM25.h"
#include "Delay.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_gpio.h"
#include <stdio.h>

// PM2.5 LED控制引脚 
#define PM25_LED_PIN     GPIO_Pin_13
#define PM25_LED_PORT    GPIOC

/**
  * @brief  PM2.5传感器初始化
  * @param  无
  * @retval 无
  * @note   使用PA0(ADC2通道0)连接PM2.5传感器模拟输出
  *         使用PC13控制PM2.5 LED
  *         原理：LED发光，粉尘散射，光电二极管检测散射光强度
  */
void PM25_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;
    
    // 1. 使能时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_ADC2, ENABLE);
    
    // 延时确保时钟稳定
    Delay_ms(100);
    
    // 配置ADC时钟为12MHz (72MHz/6)，确保不超过14MHz限制
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);
    
    // 2. 配置PA0引脚为模拟输入模式 (ADC1通道0)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 3. 配置PC13为推挽输出模式 (控制PM2.5 LED)
    GPIO_InitStructure.GPIO_Pin = PM25_LED_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(PM25_LED_PORT, &GPIO_InitStructure);
    
    // 初始状态：LED关闭
    GPIO_SetBits(PM25_LED_PORT, PM25_LED_PIN);
    
    // 4. 配置ADC2参数
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;        // 独立模式
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;              // 禁用扫描模式（单通道）
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;       // 禁用连续转换
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // 软件触发
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;     // 数据右对齐
    ADC_InitStructure.ADC_NbrOfChannel = 1;                    // 1个转换通道
    ADC_Init(ADC2, &ADC_InitStructure);
    
    // 5. 配置ADC通道
    // 配置通道0（对应PA0），第1个转换，采样时间为55.5个周期
    ADC_RegularChannelConfig(ADC2, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5);
    
    // 6. 使能ADC
    ADC_Cmd(ADC2, ENABLE);
    
    // 7. ADC校准
    ADC_ResetCalibration(ADC2);
    while(ADC_GetResetCalibrationStatus(ADC2));
    ADC_StartCalibration(ADC2);
    while(ADC_GetCalibrationStatus(ADC2));
}

/**
  * @brief  获取PM2.5传感器ADC原始值
  * @param  无
  * @retval ADC原始值(0-4095)
  */
uint16_t PM25_GetRawValue(void)
{
    // 软件启动ADC转换
    ADC_SoftwareStartConvCmd(ADC2, ENABLE);
    
    // 等待转换完成
    while(ADC_GetFlagStatus(ADC2, ADC_FLAG_EOC) == RESET);
    
    // 读取ADC值(12位值，0-4095)
    return ADC_GetConversionValue(ADC2);
}

/**
  * @brief  获取PM2.5传感器电压值
  * @param  无
  * @retval 电压值(V)
  */
float PM25_GetVoltage(void)
{
    uint16_t adcValue = PM25_GetRawValue();
    // 公式：电压 = (ADC值 / 4095) * 参考电压
    // 与PM25_ReadPM25()保持一致，使用5.0V参考电压
    return ((float)adcValue / 4095.0f) * 5.0f;
}

/**
  * @brief  读取PM2.5浓度值
  * @param  无
  * @retval PM2.5浓度 (μg/m³)
  * @note   参考公式：PM2.5(μg/m³) = (0.17 * Vout - 0.1) * 1000
  *         简化后：PM2.5 = 170 * Vout - 100
  */
float PM25_ReadPM25(void)
{
    float voltage = 0.0f;
    float pm25 = 0.0f;
    uint16_t adc_raw = 0;
    
    // 根据参考代码修改的驱动时序：
    // 1. 开启LED (低电平有效)
    GPIO_ResetBits(PM25_LED_PORT, PM25_LED_PIN);
    
    // 2. 关键延时280us，等待LED稳定
    Delay_us(280);
    
    // 3. 读取ADC值
    adc_raw = PM25_GetRawValue();
    voltage = ((float)adc_raw / 4095.0f) * 5.0f;
    
    // 4. 关闭LED (高电平)
    GPIO_SetBits(PM25_LED_PORT, PM25_LED_PIN);
    
    // 调试信息
    printf("PM25 DEBUG: ADC Raw=%d, Voltage=%.3fV\n", adc_raw, voltage);
    
    // 5. 使用标准公式计算PM2.5浓度
    // 公式：PM2.5(μg/m³) = (0.17 * Vout - 0.1) * 1000
    // 简化后：PM2.5 = 170 * Vout - 100
    pm25 = 170.0f * voltage - 100.0f;
    
    // 6. 确保返回非负值
    return (pm25 > 0) ? pm25 : 0;
}

/**
  * @brief  获取PM2.5污染等级
  * @param  pm25_value: PM2.5浓度值 (μg/m³)
  * @retval 污染等级(0-5)
  */
uint8_t PM25_GetLevel(void)
{
    float pm25;
    
    // 直接调用PM25_ReadPM25()获取PM2.5值，保持接口兼容性
    pm25 = PM25_ReadPM25();
    
    // 基于PM2.5值的污染等级划分
    if (pm25 <= 35) {
        return PM25_LEVEL_GOOD;                    // 优(0-35)
    } else if (pm25 <= 75) {
        return PM25_LEVEL_MODERATE;                // 良(36-75)
    } else if (pm25 <= 115) {
        return PM25_LEVEL_UNHEALTHY_SENSITIVE;     // 轻度污染(76-115)
    } else if (pm25 <= 150) {
        return PM25_LEVEL_UNHEALTHY;                // 中度污染(116-150)
    } else if (pm25 <= 250) {
        return PM25_LEVEL_VERY_UNHEALTHY;          // 重度污染(151-250)
    } else {
        return PM25_LEVEL_HAZARDOUS;                // 严重污染(251+)
    }
}

/**
  * @brief  获取PM2.5污染等级（使用已知PM2.5值）
  * @param  pm25_value: PM2.5浓度值 (μg/m³)
  * @retval 污染等级(0-5)
  */
uint8_t PM25_GetLevelFromValue(float pm25_value)
{
    // 基于PM2.5值的污染等级划分
    if (pm25_value <= 35) {
        return PM25_LEVEL_GOOD;                    // 优(0-35)
    } else if (pm25_value <= 75) {
        return PM25_LEVEL_MODERATE;                // 良(36-75)
    } else if (pm25_value <= 115) {
        return PM25_LEVEL_UNHEALTHY_SENSITIVE;     // 轻度污染(76-115)
    } else if (pm25_value <= 150) {
        return PM25_LEVEL_UNHEALTHY;                // 中度污染(116-150)
    } else if (pm25_value <= 250) {
        return PM25_LEVEL_VERY_UNHEALTHY;          // 重度污染(151-250)
    } else {
        return PM25_LEVEL_HAZARDOUS;                // 严重污染(251+)
    }
}

/**
  * @brief  获取PM2.5等级描述
  * @param  无
  * @retval PM2.5等级描述字符串
  */
const char* PM25_GetDescription(void)
{
    uint8_t level = PM25_GetLevel();
    
    switch (level) {
        case PM25_LEVEL_GOOD:
            return "Good";
        case PM25_LEVEL_MODERATE:
            return "Moderate";
        case PM25_LEVEL_UNHEALTHY_SENSITIVE:
            return "Unhealthy";
        case PM25_LEVEL_UNHEALTHY:
            return "VeryUnhealthy";
        case PM25_LEVEL_VERY_UNHEALTHY:
            return "Hazardous";
        case PM25_LEVEL_HAZARDOUS:
            return "Severe";
        default:
            return "Unknown";
    }
}

/**
  * @brief  获取PM2.5数值的完整描述（包含等级）
  * @param  无
  * @retval 格式化的PM2.5描述字符串
  */
const char* PM25_GetLevelDescription(void)
{
    static char desc[30];
    float pm25 = PM25_ReadPM25();
    const char* levelDesc = PM25_GetDescription();
    
    // 格式："PM2.5: <value> μg/m³ (<level>)"
    sprintf(desc, "PM2.5:%.0f(%s)", pm25, levelDesc);
    return desc;
}
