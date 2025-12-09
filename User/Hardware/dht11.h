#ifndef DHT11_H
#define DHT11_H

#include "hardware_def.h"
#include "debug.h"
#include <FreeRTOS.h>
#include <task.h>
typedef struct
{
    uint8_t  humi_int;          //湿度的整数部分
    uint8_t  humi_deci;         //湿度的小数部分
    uint8_t  temp_int;          //温度的整数部分
    uint8_t  temp_deci;         //温度的小数部分
    uint8_t  check_sum;         //校验和
}DHT11_Data_TypeDef;

void DHT11_Init(void);
int Read_DHT11(DHT11_Data_TypeDef* data);

#endif
