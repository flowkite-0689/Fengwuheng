/**
 * @file index.h
 * @brief 首页界面头文件 - 基于main.c中的Page_Logic
 * @author flowkite-0689
 * @version v1.0
 * @date 2025.12.05
 */

#ifndef __INDEX_H
#define __INDEX_H

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "unified_menu.h"
#include "oled_print.h"
#include "rtc_date.h"
#include "sensordata.h"
// ==================================
// 首页状态数据结构
// ==================================

typedef struct {
    // 时间信息
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
    uint8_t day;
    uint8_t month;
    uint16_t year;
    char weekday[16];
    
    // 系统状态
    uint32_t step_count;        // 步数
    uint8_t step_active;        // 步数激活状态
    
    // 刷新标志
    uint8_t need_refresh;       // 需要刷新
    uint32_t last_update;       // 上次更新时间
} index_state_t;

// ==================================
// 全局变量声明
// ==================================

extern index_state_t g_index_state;

// ==================================
// 函数声明
// ==================================

/**
 * @brief 初始化首页
 * @return 创建的首页菜单项指针
 */
menu_item_t* index_init(void);

/**
 * @brief 首页自定义绘制函数
 * @param context 绘制上下文（首页状态指针）
 */
void index_draw_function(void* context);

/**
 * @brief 首页按键处理函数
 * @param item 菜单项
 * @param key_event 按键事件
 */
void index_key_handler(menu_item_t* item, uint8_t key_event);

/**
 * @brief 更新首页时间信息
 */
void index_update_time(void);

/**
 * @brief 获取首页状态
 * @return 首页状态指针
 */
index_state_t* index_get_state(void);

/**
 * @brief 刷新首页显示
 */
void index_refresh_display(void);

/**
 * @brief 首页进入回调
 * @param item 菜单项
 */
void index_on_enter(menu_item_t* item);

/**
 * @brief 首页退出回调
 * @param item 菜单项
 */
void index_on_exit(menu_item_t* item);

#endif // __INDEX_H
