#ifndef _STEPCOUNTER_H_
#define _STEPCOUNTER_H_

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "unified_menu.h"
#include "oled_print.h"
#include "../../Hardware/MPU6050/simple_pedometer.h"

// 步数界面状态结构体
typedef struct {
    // 刷新标志
    uint8_t need_refresh;       // 需要刷新
    uint32_t last_update;       // 上次更新时间
    
    // 步数显示状态
    uint8_t show_reset_confirm; // 是否显示重置确认
} StepCounter_state_t;

/**
 * @brief 初始化步数页面
 * @return 创建的步数菜单项指针
 */
menu_item_t* StepCounter_init(void);

/**
 * @brief 步数自定义绘制函数
 * @param context 绘制上下文
 */
void StepCounter_draw_function(void* context);

/**
 * @brief 步数按键处理函数
 * @param item 菜单项
 * @param key_event 按键事件
 */
void StepCounter_key_handler(menu_item_t* item, uint8_t key_event);

/**
 * @brief 获取步数状态
 * @return 步数状态指针
 */
StepCounter_state_t* StepCounter_get_state(void);

/**
 * @brief 刷新步数显示
 */
void StepCounter_refresh_display(void);

/**
 * @brief 进入步数页面时的回调
 * @param item 菜单项
 */
void StepCounter_on_enter(menu_item_t* item);

/**
 * @brief 退出步数页面时的回调
 * @param item 菜单项
 */
void StepCounter_on_exit(menu_item_t* item);

#endif
