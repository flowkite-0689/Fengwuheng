/**
 * @file alarm_list.h
 * @brief 闹钟列表页面头文件
 * @author flowkite-0689
 * @version v1.0
 * @date 2025.12.08
 */

#ifndef __ALARM_LIST_H
#define __ALARM_LIST_H

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

#include "alarm_add.h"
// ==================================
// 宏定义
// ==================================

#define ALARM_LIST_SHOWING_NUM 4    // 每页显示的闹钟数量
#define ALARM_DETAIL_OPTIONS_NUM 4  // 详情页面选项数量

// 详情页面选项枚举
typedef enum {
    DETAIL_OPTION_TIME = 0,     // 时间选项
    DETAIL_OPTION_STATUS,        // 状态选项
    DETAIL_OPTION_REPEAT,        // 重复选项
    DETAIL_OPTION_DELETE         // 删除选项
} alarm_detail_option_t;

// ==================================
// 闹钟列表状态结构体
// ==================================

typedef struct alarm_list_state {
    // 列表控制
    uint8_t selected;           // 当前选中项
    uint8_t current_page;       // 当前页码
    uint8_t need_refresh;       // 需要刷新标志
    
    // 详情控制
    uint8_t in_detail_view;     // 是否在详情视图
    uint8_t detail_selected;    // 详情页面选中项
    uint8_t operating;          // 是否正在操作
    
    // 编辑控制
    uint8_t editing_mode;       // 是否在编辑模式
    uint8_t edit_step;          // 当前编辑步骤
    
    // 临时数据
    Alarm_TypeDef temp_alarm;   // 临时闹钟数据（用于编辑）
    
    TickType_t last_update;     // 上次更新时间
} alarm_list_state_t;

// ==================================
// 函数声明
// ==================================

// 初始化与销毁函数
menu_item_t* alarm_list_init(void);
void alarm_list_deinit(menu_item_t* item);

// 回调函数
void alarm_list_on_enter(menu_item_t* item);
void alarm_list_on_exit(menu_item_t* item);
void alarm_list_key_handler(menu_item_t* item, uint8_t key_event);
void alarm_list_draw_function(void* context);

// 显示函数
void alarm_list_display_list(alarm_list_state_t* state);
void alarm_list_display_detail(alarm_list_state_t* state, uint8_t alarm_index);
void alarm_list_display_empty(void);

// 详情处理函数
void alarm_list_enter_detail(alarm_list_state_t* state, uint8_t alarm_index);
void alarm_list_exit_detail(alarm_list_state_t* state);
void alarm_list_process_detail(alarm_list_state_t* state, uint8_t key_event);
void alarm_list_handle_detail_action(alarm_list_state_t* state);

// 工具函数
static inline uint8_t alarm_list_need_refresh(alarm_list_state_t* state)
{
    return (state != NULL) ? state->need_refresh : 0;
}

static inline void alarm_list_set_refresh(alarm_list_state_t* state, uint8_t refresh)
{
    if (state) {
        state->need_refresh = refresh;
    }
}

#endif // __ALARM_LIST_H
