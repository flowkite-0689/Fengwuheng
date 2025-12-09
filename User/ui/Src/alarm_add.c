/**
 * @file alarm_add.c
 * @brief 新建闹钟页面实现
 * @author flowkite-0689
 * @version v1.0
 * @date 2025.12.08
 */

#include "alarm_add.h"
#include <stdlib.h>

// ==================================
// 静态函数声明
// ==================================
static void alarm_add_init_state(alarm_add_state_t *state);
static void alarm_add_cleanup_state(alarm_add_state_t *state);

// ==================================
// 页面初始化
// ==================================

/**
 * @brief 初始化新建闹钟页面
 * @return 创建的页面指针
 */
menu_item_t *alarm_add_init(void)
{
    // 创建自定义菜单项，不分配具体状态数据
    menu_item_t *alarm_add_page = MENU_ITEM_CUSTOM("Alarm Add", alarm_add_draw_function, NULL);
    if (alarm_add_page == NULL) {
        return NULL;
    }
    
    // 设置回调函数
    menu_item_set_callbacks(alarm_add_page, 
                           alarm_add_on_enter, 
                           alarm_add_on_exit, 
                           NULL, 
                           alarm_add_key_handler);
    
    printf("Alarm Add page created successfully\r\n");
    return alarm_add_page;
}

// ==================================
// 自定义绘制函数
// ==================================

/**
 * @brief 新建闹钟自定义绘制函数
 * @param context 绘制上下文
 */
void alarm_add_draw_function(void *context)
{
    alarm_add_state_t *state = (alarm_add_state_t *)context;
    if (state == NULL) {
        return;
    }
    
    // 显示设置界面
    alarm_add_display_setting(state);
    
    OLED_Refresh_Dirty();
}

// ==================================
// 按键处理
// ==================================

/**
 * @brief 新建闹钟按键处理函数
 * @param item 菜单项
 * @param key_event 按键事件
 */
void alarm_add_key_handler(menu_item_t *item, uint8_t key_event)
{
    alarm_add_state_t *state = (alarm_add_state_t *)item->content.custom.draw_context;
    
    switch (key_event) {
        case MENU_EVENT_KEY_UP:
            // KEY0 - 增加当前设置项的值
            printf("Alarm Add: KEY0 pressed - Increase value\r\n");
            alarm_add_process_setting(state, MENU_EVENT_KEY_UP);
            break;
            
        case MENU_EVENT_KEY_DOWN:
            // KEY1 - 减少当前设置项的值
            printf("Alarm Add: KEY1 pressed - Decrease value\r\n");
            alarm_add_process_setting(state, MENU_EVENT_KEY_DOWN);
            break;
            
        case MENU_EVENT_KEY_SELECT:
            // KEY2 - 确认/保存闹钟
            printf("Alarm Add: KEY2 pressed - Save alarm\r\n");
            if (state) {
                alarm_add_save_alarm(state);
            }
            break;
            
        case MENU_EVENT_KEY_ENTER:
            // KEY3 - 下一个设置项
            printf("Alarm Add: KEY3 pressed - Next step\r\n");
            if (state) {
                state->set_step++;
                if (state->set_step >= SET_STEP_COUNT) {
                    state->set_step = 0; // 循环回到第一项
                }
                state->need_refresh = 1;
            }
            break;
            
        case MENU_EVENT_REFRESH:
            // 刷新显示
            if (state) {
                state->need_refresh = 1;
            }
            break;
            
        default:
            break;
    }
}

// ==================================
// 回调函数
// ==================================

/**
 * @brief 新建闹钟页面进入回调
 * @param item 菜单项
 */
void alarm_add_on_enter(menu_item_t *item)
{
    printf("Enter Alarm Add page\r\n");
    
    // 分配状态数据结构
    alarm_add_state_t *state = (alarm_add_state_t *)pvPortMalloc(sizeof(alarm_add_state_t));
    if (state == NULL) {
        printf("Error: Failed to allocate alarm add state memory!\r\n");
        return;
    }
    
    printf("MALLOC: alarm_add_on_enter, state addr=%p, size=%d bytes\r\n", 
           state, sizeof(alarm_add_state_t));
    
    // 初始化状态数据
    alarm_add_init_state(state);
    
    // 设置到菜单项上下文
    item->content.custom.draw_context = state;
    
    // 清屏并标记需要刷新
    OLED_Clear();
    state->need_refresh = 1;
}

/**
 * @brief 新建闹钟页面退出回调
 * @param item 菜单项
 */
void alarm_add_on_exit(menu_item_t *item)
{
    printf("Exit Alarm Add page\r\n");
    
    alarm_add_state_t *state = (alarm_add_state_t *)item->content.custom.draw_context;
    if (state == NULL) {
        return;
    }
    
    // 清理状态数据
    alarm_add_cleanup_state(state);
    
    // 释放状态结构体本身
    printf("FREE: alarm_add_on_exit, state addr=%p, size=%d bytes\r\n", 
           state, sizeof(alarm_add_state_t));
    vPortFree(state);
    
    // 清空指针，防止野指针
    item->content.custom.draw_context = NULL;
    
    // 清屏
    OLED_Clear();
}

// ==================================
// 状态初始化与清理
// ==================================

/**
 * @brief 初始化闹钟设置状态
 * @param state 状态指针
 */
static void alarm_add_init_state(alarm_add_state_t *state)
{
    if (state == NULL) {
        return;
    }
    
    // 清零状态结构体
    memset(state, 0, sizeof(alarm_add_state_t));
    
    // 初始化状态
    state->set_step = SET_STEP_HOUR;
    state->need_refresh = 1;
    state->last_update = xTaskGetTickCount();
    
    // 初始化临时闹钟数据
    state->temp_alarm.hour = 0;
    state->temp_alarm.minute = 0;
    state->temp_alarm.second = 0;
    state->temp_alarm.enabled = 1;
    state->temp_alarm.repeat = 0;
    
    // 初始化文本缓冲区
    strcpy(state->time_text, "00:00:00");
    strcpy(state->repeat_text, "NO");
    
    printf("Alarm Add state initialized\r\n");
}

/**
 * @brief 清理闹钟设置状态
 * @param state 状态指针
 */
static void alarm_add_cleanup_state(alarm_add_state_t *state)
{
    if (state == NULL) {
        return;
    }
    
    printf("Alarm Add state cleaned up\r\n");
}

// ==================================
// 设置功能实现
// ==================================

/**
 * @brief 显示闹钟设置界面
 * @param state 闹钟设置状态
 */
void alarm_add_display_setting(alarm_add_state_t *state)
{
    if (!state) {
        return;
    }
    
    // 根据设置步骤高亮显示当前设置项（参考你的代码格式）
    switch (state->set_step) {
        case SET_STEP_HOUR: // 设置小时
            OLED_Clear_Line(1);
            OLED_Printf_Line(1, " [%02d]:%02d:%02d-loop:%s", 
                             state->temp_alarm.hour, state->temp_alarm.minute, 
                             state->temp_alarm.second, state->temp_alarm.repeat ? "YES" : "NO ");
            OLED_Clear_Line(2);
            OLED_Printf_Line(2, "   Set Hours");
            break;
            
        case SET_STEP_MINUTE: // 设置分钟
            OLED_Clear_Line(1);
            OLED_Printf_Line(1, " %02d:[%02d]:%02d-loop:%s", 
                             state->temp_alarm.hour, state->temp_alarm.minute, 
                             state->temp_alarm.second, state->temp_alarm.repeat ? "YES" : "NO ");
            OLED_Clear_Line(2);
            OLED_Printf_Line(2, "  Set Minutes");
            break;
            
        case SET_STEP_SECOND: // 设置秒
            OLED_Clear_Line(1);
            OLED_Printf_Line(1, " %02d:%02d:[%02d]-loop:%s", 
                             state->temp_alarm.hour, state->temp_alarm.minute, 
                             state->temp_alarm.second, state->temp_alarm.repeat ? "YES" : "NO ");
            OLED_Clear_Line(2);
            OLED_Printf_Line(2, "   Set Seconds");
            break;
            
        case SET_STEP_REPEAT: // 设置重复
            OLED_Clear_Line(1);
            OLED_Printf_Line(1, " %02d:%02d:%02d-loop:[%s]", 
                             state->temp_alarm.hour, state->temp_alarm.minute, 
                             state->temp_alarm.second, state->temp_alarm.repeat ? "YES" : "NO ");
            OLED_Clear_Line(2);
            OLED_Printf_Line(2, "   Set Repeat");
            break;
    }
    
    // 显示标题和操作提示（参考你的代码格式）
    OLED_Printf_Line(0, "  SET ALARM");
    OLED_Printf_Line(3, "KEY0:+ KEY1:- KEY2:OK");
}

/**
 * @brief 处理闹钟设置
 * @param state 闹钟设置状态
 * @param key_event 按键事件
 */
void alarm_add_process_setting(alarm_add_state_t *state, uint8_t key_event)
{
    if (!state) {
        return;
    }
    
    switch (key_event) {
        case MENU_EVENT_KEY_UP: // 增加
            switch (state->set_step) {
                case SET_STEP_HOUR: // 小时
                    state->temp_alarm.hour = (state->temp_alarm.hour + 1) % 24;
                    break;
                case SET_STEP_MINUTE: // 分钟
                    state->temp_alarm.minute = (state->temp_alarm.minute + 1) % 60;
                    break;
                case SET_STEP_SECOND: // 秒
                    state->temp_alarm.second = (state->temp_alarm.second + 1) % 60;
                    break;
                case SET_STEP_REPEAT: // 重复
                    state->temp_alarm.repeat = !state->temp_alarm.repeat;
                    break;
            }
            state->need_refresh = 1;
            break;
            
        case MENU_EVENT_KEY_DOWN: // 减少
            switch (state->set_step) {
                case SET_STEP_HOUR: // 小时
                    state->temp_alarm.hour = (state->temp_alarm.hour == 0) ? 23 : state->temp_alarm.hour - 1;
                    break;
                case SET_STEP_MINUTE: // 分钟
                    state->temp_alarm.minute = (state->temp_alarm.minute == 0) ? 59 : state->temp_alarm.minute - 1;
                    break;
                case SET_STEP_SECOND: // 秒
                    state->temp_alarm.second = (state->temp_alarm.second == 0) ? 59 : state->temp_alarm.second - 1;
                    break;
                case SET_STEP_REPEAT: // 重复
                    state->temp_alarm.repeat = !state->temp_alarm.repeat;
                    break;
            }
            state->need_refresh = 1;
            break;
            
        case MENU_EVENT_KEY_SELECT: // 下一个设置项
            state->set_step = (state->set_step + 1) % SET_STEP_COUNT;
            state->need_refresh = 1;
            break;
    }
}

/**
 * @brief 保存闹钟
 * @param state 闹钟设置状态
 */
void alarm_add_save_alarm(alarm_add_state_t *state)
{
    if (!state) {
        return;
    }
    
    // 保存新闹钟
    if (Alarm_Add(&state->temp_alarm) == 0) {
        OLED_Clear();
        OLED_Printf_Line(1, " ALARM SET ");
        OLED_Printf_Line(2, " SUCCESS! ");
        OLED_Refresh();
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        // 返回上一级菜单
        menu_back_to_parent();
    } else {
        OLED_Clear();
        OLED_Printf_Line(1, " ALARM SET ");
        OLED_Printf_Line(2, " FAILED! ");
        OLED_Refresh();
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        // 标记需要刷新
        state->need_refresh = 1;
    }
}
