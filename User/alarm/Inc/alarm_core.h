/**
 * @file alarm_core.h
 * @brief 闹钟核心数据结构和管理函数头文件
 * @author flowkite-0689
 * @version v1.0
 * @date 2025.12.08
 */

#ifndef __ALARM_CORE_H
#define __ALARM_CORE_H

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <string.h>

// ==================================
// 宏定义
// ==================================

#define MAX_ALARMS 16           // 最大闹钟数量
#define ALARM_ID_LEN 8          // 闹钟ID长度

// ==================================
// 闹钟数据结构
// ==================================

typedef struct {
    char id[ALARM_ID_LEN];      // 闹钟ID
    uint8_t hour;              // 小时 (0-23)
    uint8_t minute;            // 分钟 (0-59)
    uint8_t second;            // 秒 (0-59)
    uint8_t enabled;           // 是否启用 (0/1)
    uint8_t repeat;            // 是否重复 (0/1)
    uint8_t _reserved[2];      // 保留字节，用于对齐
} Alarm_TypeDef;

// ==================================
// 全局变量声明
// ==================================

extern Alarm_TypeDef g_alarms[MAX_ALARMS];
extern uint8_t g_alarm_count;

// ==================================
// 函数声明
// ==================================

// 闹钟管理函数
int Alarm_Add(Alarm_TypeDef *alarm);
int Alarm_Delete(uint8_t index);
int Alarm_Enable(uint8_t index);
int Alarm_Disable(uint8_t index);
int Alarm_Update(uint8_t index, Alarm_TypeDef *alarm);

// 闹钟检查函数
int Alarm_Check(void);
void Alarm_GenerateId(char *id);

// 闹钟列表操作函数
Alarm_TypeDef* Alarm_Get(uint8_t index);
uint8_t Alarm_GetCount(void);

#endif // __ALARM_CORE_H
