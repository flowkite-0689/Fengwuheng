#ifndef RTC_DATE_H
#define RTC_DATE_H

#include "stm32f10x.h"                  // Device header
#include <time.h>
#include "debug.h"
#include "oled_print.h"
#include "Delay.h"
extern uint16_t MyRTC_Time[];

typedef struct
{
  uint32_t year;
  uint8_t mon;
  uint8_t day;
  uint8_t hours;
  uint8_t minutes;
  uint8_t seconds;
  const char* weekday; 

}myRTC_data;

extern myRTC_data RTC_data;
void MyRTC_Init(void);
void MyRTC_SetTime(void);
void MyRTC_ReadTime(void);
void RTC_SetTime_Manual(uint8_t hours, uint8_t minutes, uint8_t seconds);
void RTC_SetDate_Manual(uint16_t year, uint8_t month, uint8_t day);
void RTC_SetDateTime_Manual(uint16_t year, uint8_t month, uint8_t day,
                            uint8_t hours, uint8_t minutes, uint8_t seconds);
uint8_t RTC_SetFromNetworkTime(const char *time_str);
#endif
