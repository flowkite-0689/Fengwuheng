/**
 * @file alarm_alert.c
 * @brief 闹钟提醒页面实现
 * @author flowkite-0689
 * @version v1.0
 * @date 2025.12.08
 */

#include "alarm_alert.h"
#include "beep.h"
#include "oled_print.h"
#include <stdio.h>
#include <string.h>

// ==================================
// 全局变量定义
// ==================================

static alarm_alert_state_t g_alarm_alert_state;

// ==================================
// 静态函数声明
// ==================================

static void alarm_alert_init_state(alarm_alert_state_t *state);

// ==================================
// 页面初始化
// ==================================

/**
 * @brief 初始化闹钟提醒页面
 * @return 创建的菜单项指针
 */
menu_item_t *alarm_alert_init(void)
{
    // 创建自定义菜单项
    menu_item_t *alarm_alert_page = MENU_ITEM_CUSTOM("Alarm Alert", alarm_alert_draw_function, NULL);
    if (alarm_alert_page == NULL) {
        return NULL;
    }
    
    // 设置回调函数
    menu_item_set_callbacks(alarm_alert_page, 
                           alarm_alert_on_enter, 
                           alarm_alert_on_exit, 
                           NULL, 
                           alarm_alert_key_handler);
    
    printf("Alarm Alert page created successfully\r\n");
    return alarm_alert_page;
}

// ==================================
// 回调函数
// ==================================

/**
 * @brief 闹钟提醒页面进入回调
 * @param item 菜单项
 */
void alarm_alert_on_enter(menu_item_t *item)
{
    printf("Enter Alarm Alert page\r\n");
    Beep_Init();
    // 分配状态数据结构
    alarm_alert_state_t *state = &g_alarm_alert_state;
    
    // 设置到菜单项上下文
    item->content.custom.draw_context = state;
    
    // 如果闹钟提醒已经激活，保持状态不变
    if (!state->active) {
        // 只有当闹钟提醒未激活时才重新初始化
        alarm_alert_init_state(state);
    }
    
    // 清屏并标记需要刷新
    OLED_Clear();
    state->need_refresh = 1;
}

/**
 * @brief 闹钟提醒页面退出回调
 * @param item 菜单项
 */
void alarm_alert_on_exit(menu_item_t *item)
{
    printf("Exit Alarm Alert page\r\n");
    
    alarm_alert_state_t *state = (alarm_alert_state_t *)item->content.custom.draw_context;
    if (state == NULL) {
        return;
    }
    
    // 停止蜂鸣器
   state->isRaing = 0;
    
    // 关闭闹钟提醒
    state->active = 0;
    
    // 清屏
    OLED_Clear();
}

// ==================================
// 按键处理
// ==================================

/**
 * @brief 闹钟提醒页面按键处理
 * @param item 菜单项
 * @param key_event 按键事件
 */
void alarm_alert_key_handler(menu_item_t *item, uint8_t key_event)
{
    alarm_alert_state_t *state = (alarm_alert_state_t *)item->content.custom.draw_context;
    if (state == NULL || !state->active) {
        return;
    }
    
    printf("Alarm Alert: KEY%d pressed\r\n", key_event - 1);
    
    switch (key_event) {
        case MENU_EVENT_KEY_SELECT:
        case MENU_EVENT_KEY_ENTER:
            // KEY2 或 KEY3 - 关闭闹钟提醒
            printf("Alarm Alert: Close alarm\r\n");
            
            // 停止蜂鸣器
            state->isRaing = 0;
            
            // 关闭闹钟提醒
            state->active = 0;
            
            // 立即清屏，避免绘制冲突
            OLED_Clear();
            OLED_Refresh();
            
            // 返回父菜单
            menu_back_to_parent();
            break;
            
        default:
            break;
    }
}

// ==================================
// 绘制函数
// ==================================

/**
 * @brief 闹钟提醒页面绘制函数
 * @param context 绘制上下文
 */
void alarm_alert_draw_function(void *context)
{
    alarm_alert_state_t *state = (alarm_alert_state_t *)context;
    if (state == NULL || !state->active) {
        // 如果页面不活跃，确保清屏并返回
        OLED_Clear();
        OLED_Refresh();
        return;
    }
    
    // 获取触发的闹钟信息
    Alarm_TypeDef *alarm = Alarm_Get(state->triggered_alarm_index);
    if (alarm == NULL) {
        return;
    }
    
    // 闪烁效果：每秒切换一次
    TickType_t current_time = xTaskGetTickCount();
    if (current_time - state->last_beep_time >= pdMS_TO_TICKS(500)) {
        // 每500ms播放一次蜂鸣器
        if (state->isRaing) {
            Beep_ON();
            vTaskDelay(30);
            Beep_OFF();
        }
        state->last_beep_time = current_time;
        state->need_refresh = 1;
    }
    
    if (current_time - state->start_time >= pdMS_TO_TICKS(1000)) {
        state->blink_state = !state->blink_state;
        state->start_time = current_time;
        state->need_refresh = 1;
    }
    
    if (state->need_refresh) {
        OLED_Clear();
        
        // 显示闹钟提醒标题
        OLED_Printf_Line(0, "   ALARM!   ");
        
        // 显示闹钟时间（闪烁效果）
        if (state->blink_state) {
            OLED_Printf_Line(2, "  %02d:%02d:%02d  ", 
                           alarm->hour, alarm->minute, alarm->second);
        } else {
            OLED_Printf_Line(2, "            ");
        }
        
        // 显示重复状态
        OLED_Printf_Line(3, " Repeat: %s  ", alarm->repeat ? "YES" : "NO");
        
        // 显示操作提示
        OLED_Printf_Line(6, " Press KEY2/3  ");
        OLED_Printf_Line(7, "  to Close     ");
        
        OLED_Refresh();
        state->need_refresh = 0;
    }
}

// ==================================
// 闹钟触发函数
// ==================================

/**
 * @brief 触发闹钟提醒
 * @param alarm_index 闹钟索引
 * @return 0-成功，其他-失败
 */
int8_t alarm_alert_trigger(uint8_t alarm_index)
{
    if (alarm_index >= Alarm_GetCount()) {
        return -1;
    }
    
    // 初始化状态
    alarm_alert_init_state(&g_alarm_alert_state);
    
    // 设置触发的闹钟索引
    g_alarm_alert_state.active = 1;
    g_alarm_alert_state.triggered_alarm_index = alarm_index;
    g_alarm_alert_state.need_refresh = 1;
    g_alarm_alert_state.start_time = xTaskGetTickCount();
    g_alarm_alert_state.last_beep_time = xTaskGetTickCount();
    g_alarm_alert_state.blink_state = 1;
    g_alarm_alert_state.isRaing = 1;
    
    // 播放一次蜂鸣器提示
    BEEP_Buzz(10);
    
    printf("Alarm Alert triggered for index %d\r\n", alarm_index);
    
    return 0;
}

// ==================================
// 状态管理
// ==================================

/**
 * @brief 初始化闹钟提醒状态
 * @param state 状态指针
 */
static void alarm_alert_init_state(alarm_alert_state_t *state)
{
    if (state == NULL) {
        return;
    }
    
    memset(state, 0, sizeof(alarm_alert_state_t));
    
    // 初始化默认值
    state->active = 0;
    state->triggered_alarm_index = 0;
    state->need_refresh = 1;
    state->start_time = xTaskGetTickCount();
    state->blink_state = 1;
    
    printf("Alarm Alert state initialized\r\n");
}
