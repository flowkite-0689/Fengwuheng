#ifndef __SIMPLE_PEDOMETER_H
#define __SIMPLE_PEDOMETER_H

#include "hardware_def.h"

// 全局步数变量
extern unsigned long g_step_count;

// 函数声明
void simple_pedometer_init(void);
unsigned long simple_pedometer_update(short ax, short ay, short az);
void simple_pedometer_reset(void);
unsigned long simple_pedometer_get_steps(void);

#endif
