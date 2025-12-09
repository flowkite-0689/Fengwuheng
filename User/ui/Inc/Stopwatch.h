#ifndef _STOPWATCH_H_
#define _STOPWATCH_H_

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "unified_menu.h"
#include "oled_print.h"

// 秒表更新间隔（毫秒）
#define STOPWATCH_UPDATE_INTERVAL 10

typedef struct{
    // 秒表状态
    uint8_t running;             // 是否正在运行
    uint32_t start_time;         // 开始时间
    uint32_t pause_time;         // 累计暂停时间
    uint32_t elapsed_time;       // 已用时间（毫秒）
    
    // 刷新标志
    uint8_t need_refresh;       // 需要刷新
    uint32_t last_update;       // 上次更新时间
}Stopwatch_state_t;

/**
 * @brief 初始化秒表页面
 * @return 创建的秒表菜单项指针
 */
menu_item_t* Stopwatch_init(void);

/**
 * @brief 秒表自定义绘制函数
 * @param context 绘制上下文
 */
void Stopwatch_draw_function(void* context);

/**
 * @brief 秒表按键处理函数
 * @param item 菜单项
 * @param key_event 按键事件
 */
void Stopwatch_key_handler(menu_item_t* item, uint8_t key_event);

/**
 * @brief 获取秒表状态
 * @return 秒表状态指针
 */
Stopwatch_state_t* Stopwatch_get_state(void);

/**
 * @brief 刷新秒表显示
 */
void Stopwatch_refresh_display(void);

/**
 * @brief 进入秒表页面时的回调
 * @param item 菜单项
 */
void Stopwatch_on_enter(menu_item_t* item);

/**
 * @brief 退出秒表页面时的回调
 * @param item 菜单项
 */
void Stopwatch_on_exit(menu_item_t* item);

#endif
