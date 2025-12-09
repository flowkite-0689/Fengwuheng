#ifndef _SETTIME_H_
#define _SETTIME_H_

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "unified_menu.h"
#include "oled_print.h"
#include "rtc_date.h"

typedef struct{
    // 时间设置状态
    uint8_t set_step;           // 当前设置步骤：0=小时，1=分钟，2=秒
    uint8_t temp_hours;         // 临时小时
    uint8_t temp_minutes;       // 临时分钟
    uint8_t temp_seconds;       // 临时秒
    
    // 刷新标志
    uint8_t need_refresh;       // 需要刷新
    uint32_t last_update;       // 上次更新时间
}SetTime_state_t;

/**
 * @brief 初始化时间设置页面
 * @return 创建的时间设置菜单项指针
 */
menu_item_t* SetTime_init(void);

/**
 * @brief 时间设置自定义绘制函数
 * @param context 绘制上下文
 */
void SetTime_draw_function(void* context);

/**
 * @brief 时间设置按键处理函数
 * @param item 菜单项
 * @param key_event 按键事件
 */
void SetTime_key_handler(menu_item_t* item, uint8_t key_event);

/**
 * @brief 获取时间设置状态
 * @return 时间设置状态指针
 */
SetTime_state_t* SetTime_get_state(void);

/**
 * @brief 刷新时间设置显示
 */
void SetTime_refresh_display(void);

/**
 * @brief 进入时间设置页面时的回调
 * @param item 菜单项
 */
void SetTime_on_enter(menu_item_t* item);

/**
 * @brief 退出时间设置页面时的回调
 * @param item 菜单项
 */
void SetTime_on_exit(menu_item_t* item);

#endif
