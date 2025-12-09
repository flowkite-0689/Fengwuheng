#ifndef __KEY_H
#define __KEY_H
#include "hardware_def.h"//我的位带映射
#include <FreeRTOS.h>
#include <task.h>

// ==================================
// 按键按键常量定义
// ==================================



void Key_Init(void);
uint8_t Key_GetNum(void);
uint8_t KEY_Get(void);
#endif
