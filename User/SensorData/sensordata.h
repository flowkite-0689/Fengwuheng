#ifndef _SENSORDATA_H_
#define _SENSORDATA_H_ 

//温湿度
#include "dht11.h"
//光照
#include "light.h"
//PM2.5
#include "PM25.h"
#include <FreeRTOS.h>
#include <task.h>

typedef struct {
    DHT11_Data_TypeDef  dht11_data;
    PhotoRes_TypeDef    light_data;
    PM25_TypeDef        pm25_data;
} SensorData_TypeDef;
extern SensorData_TypeDef SensorData;

extern uint8_t DHT11_ON;
extern uint8_t Light_ON;
extern uint8_t PM25_ON;



void SensorData_Init(void);
void SensorData_CreateTask(void);


#endif
