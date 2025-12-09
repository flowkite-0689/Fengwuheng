#ifndef _SETDATE_H_
#define _SETDATE_H_

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "unified_menu.h"
#include "oled_print.h"
#include "rtc_date.h"

typedef struct{
    // 日期设置状态
    uint8_t set_step;           // 当前设置步骤：0=年，1=月，2=日
    uint16_t temp_year;          // 临时年份（两位数）
    uint8_t temp_month;         // 临时月份
    uint8_t temp_day;           // 临时日
    
    // 刷新标志
    uint8_t need_refresh;       // 需要刷新
    uint32_t last_update;       // 上次更新时间
}SetDate_state_t;

/**
 * @brief 获取月份的最大天数
 * @param year 年份（两位数）
 * @param month 月份（1-12）
 * @return 该月的最大天数
 */
uint8_t get_max_days_in_month(u8 year, u8 month);

/**
 * @brief 初始化日期设置页面
 * @return 创建的日期设置菜单项指针
 */
menu_item_t* SetDate_init(void);

/**
 * @brief 日期设置自定义绘制函数
 * @param context 绘制上下文
 */
void SetDate_draw_function(void* context);

/**
 * @brief 日期设置按键处理函数
 * @param item 菜单项
 * @param key_event 按键事件
 */
void SetDate_key_handler(menu_item_t* item, uint8_t key_event);

/**
 * @brief 获取日期设置状态
 * @return 日期设置状态指针
 */
SetDate_state_t* SetDate_get_state(void);

/**
 * @brief 刷新日期设置显示
 */
void SetDate_refresh_display(void);

/**
 * @brief 进入日期设置页面时的回调
 * @param item 菜单项
 */
void SetDate_on_enter(menu_item_t* item);

/**
 * @brief 退出日期设置页面时的回调
 * @param item 菜单项
 */
void SetDate_on_exit(menu_item_t* item);

#endif
