/**
 * @file alarm_add.h
 * @brief 新建闹钟页面头文件
 * @author flowkite-0689
 * @version v1.0
 * @date 2025.12.08
 */

#ifndef __ALARM_ADD_H
#define __ALARM_ADD_H

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "unified_menu.h"
#include "oled_print.h"
#include "alarm_core.h"
#include "Key.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ==================================
// 宏定义
// ==================================

#define ALARM_ADD_TEXT_LEN 16

// 设置步骤枚举
typedef enum {
    SET_STEP_HOUR = 0,      // 设置小时
    SET_STEP_MINUTE,        // 设置分钟
    SET_STEP_SECOND,        // 设置秒
    SET_STEP_REPEAT,        // 设置重复
    SET_STEP_COUNT          // 设置项总数
} alarm_set_step_t;

// ==================================
// 闹钟设置状态结构体
// ==================================

typedef struct alarm_add_state {
    // 设置步骤控制
    uint8_t set_step;           // 当前设置步骤
    uint8_t need_refresh;       // 需要刷新标志
    
    // 临时闹钟数据
    Alarm_TypeDef temp_alarm;   // 临时闹钟数据
    
    // 显示控制
    char time_text[ALARM_ADD_TEXT_LEN]; // 时间显示文本
    char repeat_text[ALARM_ADD_TEXT_LEN]; // 重复显示文本
    
    TickType_t last_update;     // 上次更新时间
} alarm_add_state_t;

// ==================================
// 函数声明
// ==================================

// 初始化与销毁函数
menu_item_t* alarm_add_init(void);
void alarm_add_deinit(menu_item_t* item);

// 回调函数
void alarm_add_on_enter(menu_item_t* item);
void alarm_add_on_exit(menu_item_t* item);
void alarm_add_key_handler(menu_item_t* item, uint8_t key_event);
void alarm_add_draw_function(void* context);

// 设置函数
void alarm_add_display_setting(alarm_add_state_t* state);
void alarm_add_process_setting(alarm_add_state_t* state, uint8_t key_event);
void alarm_add_save_alarm(alarm_add_state_t* state);

// 工具函数
static inline uint8_t alarm_add_need_refresh(alarm_add_state_t* state)
{
    return (state != NULL) ? state->need_refresh : 0;
}

static inline void alarm_add_set_refresh(alarm_add_state_t* state, uint8_t refresh)
{
    if (state) {
        state->need_refresh = refresh;
    }
}

#endif // __ALARM_ADD_H
