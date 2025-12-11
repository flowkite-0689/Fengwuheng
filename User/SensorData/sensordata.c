#include "sensordata.h"
#include "debug.h"
#include "Delay.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

static TaskHandle_t sensordate_handle = NULL;

SensorData_TypeDef SensorData;

void SensorData_Init(void)
{
    DHT11_Init();
    Light_ADC_Init(); // ADC1 初始化用于光照传感器
    PM25_Init();      // ADC2 初始化用于PM2.5传感器
}

static void SensorData_Task(void *pvParameters)
{
    printf("SensorData_Task start ->\n");
    
    // 初始延时，确保系统稳定
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    while (1)
    {
        // 读取DHT11温湿度数据
        if (Read_DHT11(&SensorData.dht11_data) != 0)
        {}
        // {
        //     printf("DHT11 read failed\r\n");
        // }
        // else
        // {
        //     printf("Temperature: %d.%dC, Humidity: %d.%d%%\r\n", 
        //            SensorData.dht11_data.temp_int, SensorData.dht11_data.temp_deci,
        //            SensorData.dht11_data.humi_int, SensorData.dht11_data.humi_deci);
        // }
        
        // 读取光照强度数据
        // 配置ADC通道1 (PA1) 用于光照传感器
        ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_55Cycles5);
        uint16_t lux_value = Light_GetLux();
        SensorData.light_data.lux = lux_value;
        // printf("Light: %d lux\r\n", SensorData.light_data.lux);
        // 读取PM2.5数据
        // // 开启PM2.5 LED (低电平有效)
        // GPIO_ResetBits(GPIOC, GPIO_Pin_13);
        // Delay_us(280); // 关键延时280us，等待LED稳定
        // vTaskDelay(pdMS_TO_TICKS(1000));
        // // 读取ADC值
        // uint16_t adc_raw = PM25_GetRawValue();
        // float voltage = ((float)adc_raw / 4095.0f) * 5.0f;
        
        // // 关闭LED (高电平)
        // GPIO_SetBits(GPIOC, GPIO_Pin_13);
        
        // // 计算PM2.5值，使用标准公式
        // float pm25_value = 170.0f * voltage - 100.0f;
        // if (pm25_value < 0) pm25_value = 0;
        
        SensorData.pm25_data.pm25_value = PM25_ReadPM25(); // 四舍五入转换
        SensorData.pm25_data.level = PM25_GetLevelFromValue(SensorData.pm25_data.pm25_value);
        // SensorData.pm25_data.voltage = voltage;
        // SensorData.pm25_data.adc_raw = adc_raw;
        // printf("PM2.5: %.1f ug/m3, Level: %d\r\n", 
        //            SensorData.pm25_data.pm25_value, SensorData.pm25_data.level);
        // printf("-------------\r\n");
        
        // 每2秒读取一次传感器数据
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

void SensorData_CreateTask(void)
{
    xTaskCreate((TaskFunction_t)SensorData_Task,          /* 任务函数 */
                (const char *)"SensorData",               /* 任务名称 */
                (uint16_t)512,                            /* 任务堆栈大小 */
                (void *)NULL,                             /* 任务函数参数 */
                (UBaseType_t)3,                           /* 任务优先级 */
                (TaskHandle_t *)&sensordate_handle);       /* 任务控制句柄 */
}
