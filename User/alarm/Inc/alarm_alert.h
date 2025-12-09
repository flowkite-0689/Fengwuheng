/**
 * @file alarm_alert.h
 * @brief 闹钟提醒页面头文件
 * @author flowkite-0689
 * @version v1.0
 * @date 2025.12.08
 */

#ifndef __ALARM_ALERT_H
#define __ALARM_ALERT_H

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "unified_menu.h"
#include "alarm_core.h"

// ==================================
// 闹钟提醒状态结构体
// ==================================

typedef struct {
    uint8_t active;                 // 是否激活
    uint8_t triggered_alarm_index;  // 触发的闹钟索引
    uint8_t need_refresh;           // 需要刷新标志
    TickType_t start_time;          // 开始时间
    TickType_t last_beep_time;      // 上次蜂鸣器时间
    uint8_t blink_state;            // 闪烁状态

    uint8_t isRaing; //是不是在响
} alarm_alert_state_t;

// ==================================
// 函数声明
// ==================================

/**
 * @brief 初始化闹钟提醒页面
 * @return 创建的菜单项指针
 */
menu_item_t *alarm_alert_init(void);

/**
 * @brief 闹钟提醒页面进入回调
 * @param item 菜单项
 */
void alarm_alert_on_enter(menu_item_t *item);

/**
 * @brief 闹钟提醒页面退出回调
 * @param item 菜单项
 */
void alarm_alert_on_exit(menu_item_t *item);

/**
 * @brief 闹钟提醒页面按键处理
 * @param item 菜单项
 * @param key_event 按键事件
 */
void alarm_alert_key_handler(menu_item_t *item, uint8_t key_event);

/**
 * @brief 闹钟提醒页面绘制函数
 * @param context 绘制上下文
 */
void alarm_alert_draw_function(void *context);

/**
 * @brief 触发闹钟提醒
 * @param alarm_index 闹钟索引
 * @return 0-成功，其他-失败
 */
int8_t alarm_alert_trigger(uint8_t alarm_index);

#endif // __ALARM_ALERT_H
