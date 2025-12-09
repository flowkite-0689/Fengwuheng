/**
 * @file alarm_list.c
 * @brief 闹钟列表页面实现
 * @author flowkite-0689
 * @version v1.0
 * @date 2025.12.08
 */

#include "alarm_list.h"
#include "alarm_add.h"
#include <stdlib.h>

// ==================================
// 静态函数声明
// ==================================
static void alarm_list_init_state(alarm_list_state_t *state);
static void alarm_list_cleanup_state(alarm_list_state_t *state);
static void alarm_list_process_list(alarm_list_state_t *state, uint8_t key_event);
static void alarm_list_process_edit(alarm_list_state_t *state, uint8_t key_event);
static void alarm_list_edit_time(alarm_list_state_t *state);
static void alarm_list_exit_edit(alarm_list_state_t *state);
static void alarm_list_display_edit(alarm_list_state_t *state);

// ==================================
// 页面初始化
// ==================================

/**
 * @brief 初始化闹钟列表页面
 * @return 创建的页面指针
 */
menu_item_t *alarm_list_init(void)
{
    // 创建自定义菜单项，不分配具体状态数据
    menu_item_t *alarm_list_page = MENU_ITEM_CUSTOM("Alarm List", alarm_list_draw_function, NULL);
    if (alarm_list_page == NULL) {
        return NULL;
    }
    
    // 设置回调函数
    menu_item_set_callbacks(alarm_list_page, 
                           alarm_list_on_enter, 
                           alarm_list_on_exit, 
                           NULL, 
                           alarm_list_key_handler);
    
    printf("Alarm List page created successfully\r\n");
    return alarm_list_page;
}

// ==================================
// 自定义绘制函数
// ==================================

/**
 * @brief 闹钟列表自定义绘制函数
 * @param context 绘制上下文
 */
void alarm_list_draw_function(void *context)
{
    alarm_list_state_t *state = (alarm_list_state_t *)context;
    if (state == NULL) {
        return;
    }
    
    // 根据当前视图模式显示不同内容
    if (state->editing_mode) {
        // 显示编辑视图
        alarm_list_display_edit(state);
    } else if (state->in_detail_view) {
        // 显示详情视图
        alarm_list_display_detail(state, state->selected);
    } else {
        // 显示列表视图
        if (Alarm_GetCount() == 0) {
            alarm_list_display_empty();
        } else {
            alarm_list_display_list(state);
        }
    }
    
    OLED_Refresh_Dirty();
}

// ==================================
// 按键处理
// ==================================

/**
 * @brief 闹钟列表按键处理函数
 * @param item 菜单项
 * @param key_event 按键事件
 */
void alarm_list_key_handler(menu_item_t *item, uint8_t key_event)
{
    alarm_list_state_t *state = (alarm_list_state_t *)item->content.custom.draw_context;
    if (state == NULL) {
        return;
    }
    
    if (state->editing_mode) {
        // 编辑模式下的按键处理
        alarm_list_process_edit(state, key_event);
    } else if (state->in_detail_view) {
        // 详情视图下的按键处理
        alarm_list_process_detail(state, key_event);
    } else {
        // 列表视图下的按键处理
        alarm_list_process_list(state, key_event);
    }
}

// ==================================
// 回调函数
// ==================================

/**
 * @brief 闹钟列表页面进入回调
 * @param item 菜单项
 */
void alarm_list_on_enter(menu_item_t *item)
{
    printf("Enter Alarm List page\r\n");
    
    // 分配状态数据结构
    alarm_list_state_t *state = (alarm_list_state_t *)pvPortMalloc(sizeof(alarm_list_state_t));
    if (state == NULL) {
        printf("Error: Failed to allocate alarm list state memory!\r\n");
        return;
    }
    
    printf("MALLOC: alarm_list_on_enter, state addr=%p, size=%d bytes\r\n", 
           state, sizeof(alarm_list_state_t));
    
    // 初始化状态数据
    alarm_list_init_state(state);
    
    // 设置到菜单项上下文
    item->content.custom.draw_context = state;
    
    // 清屏并标记需要刷新
    OLED_Clear();
    state->need_refresh = 1;
}

/**
 * @brief 闹钟列表页面退出回调
 * @param item 菜单项
 */
void alarm_list_on_exit(menu_item_t *item)
{
    printf("Exit Alarm List page\r\n");
    
    alarm_list_state_t *state = (alarm_list_state_t *)item->content.custom.draw_context;
    if (state == NULL) {
        return;
    }
    
    // 清理状态数据
    alarm_list_cleanup_state(state);
    
    // 释放状态结构体本身
    printf("FREE: alarm_list_on_exit, state addr=%p, size=%d bytes\r\n", 
           state, sizeof(alarm_list_state_t));
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
 * @brief 初始化闹钟列表状态
 * @param state 状态指针
 */
static void alarm_list_init_state(alarm_list_state_t *state)
{
    if (state == NULL) {
        return;
    }
    
    // 清零状态结构体
    memset(state, 0, sizeof(alarm_list_state_t));
    
    // 初始化状态
    state->selected = 0;
    state->current_page = 0;
    state->need_refresh = 1;
    state->in_detail_view = 0;
    state->detail_selected = 0;
    state->operating = 0;
    state->last_update = xTaskGetTickCount();
    
    printf("Alarm List state initialized\r\n");
}

/**
 * @brief 清理闹钟列表状态
 * @param state 状态指针
 */
static void alarm_list_cleanup_state(alarm_list_state_t *state)
{
    if (state == NULL) {
        return;
    }
    
    printf("Alarm List state cleaned up\r\n");
}

// ==================================
// 列表视图处理
// ==================================

/**
 * @brief 处理列表视图按键
 * @param state 列表状态
 * @param key_event 按键事件
 */
static void alarm_list_process_list(alarm_list_state_t *state, uint8_t key_event)
{
    switch (key_event) {
        case MENU_EVENT_KEY_UP:
            // KEY0 - 上一个闹钟
            printf("Alarm List: KEY0 pressed - Previous alarm\r\n");
            if (state->selected == 0) {
                state->selected = Alarm_GetCount() - 1; // 循环到最后一项
            } else {
                state->selected--;
            }
            state->need_refresh = 1;
            break;
            
        case MENU_EVENT_KEY_DOWN:
            // KEY1 - 下一个闹钟
            printf("Alarm List: KEY1 pressed - Next alarm\r\n");
            state->selected++;
            if (state->selected >= Alarm_GetCount()) {
                state->selected = 0; // 循环到第一项
            }
            state->need_refresh = 1;
            break;
            
        case MENU_EVENT_KEY_SELECT:
            // KEY2 - 返回上一级
            printf("Alarm List: KEY2 pressed - Return\r\n");
            menu_back_to_parent();
            break;
            
        case MENU_EVENT_KEY_ENTER:
            // KEY3 - 进入当前选中闹钟的详情界面
            printf("Alarm List: KEY3 pressed - Enter detail\r\n");
            if (Alarm_GetCount() > 0) {
                alarm_list_enter_detail(state, state->selected);
            }
            break;
            
        case MENU_EVENT_REFRESH:
            // 刷新显示
            state->need_refresh = 1;
            break;
            
        default:
            break;
    }
}

/**
 * @brief 显示闹钟列表
 * @param state 列表状态
 */
void alarm_list_display_list(alarm_list_state_t *state)
{
    if (!state || Alarm_GetCount() == 0) {
        return;
    }
    
    uint8_t page = state->selected / ALARM_LIST_SHOWING_NUM;
    if (state->current_page != page) {
        
        state->current_page = page;
    }
    
    // 当前在第几页（从0开始）
    uint8_t start_idx = page * ALARM_LIST_SHOWING_NUM;        // 本页第一个选项的索引
    uint8_t items_this_page = Alarm_GetCount() - start_idx;   // 本页实际有多少项
    if (items_this_page > ALARM_LIST_SHOWING_NUM) {
        items_this_page = ALARM_LIST_SHOWING_NUM;
    }
    
    // 显示当前页的闹钟项目
    for (uint8_t i = 0; i < items_this_page; i++) {
        uint8_t current_idx = start_idx + i; // 实际在总列表中的索引
        Alarm_TypeDef *alarm = Alarm_Get(current_idx);
        
        if (alarm == NULL) {
            continue;
        }
        
        // 判断是否是当前选中项
        char arrow = (current_idx == state->selected) ? '>' : ' ';
        
        // 显示这一行（从第0行开始显示闹钟）
        OLED_Printf_Line(i, "%c%02d:%02d:%02d %s %s",
                         arrow,
                         alarm->hour, alarm->minute, alarm->second,
                         alarm->enabled ? "ON " : "OFF",
                         alarm->repeat ? "R" : " ");
    }
    
    // 如果本页不足ALARM_LIST_SHOWING_NUM行，下面几行清空
    for (uint8_t i = items_this_page; i < ALARM_LIST_SHOWING_NUM; i++) {
        OLED_Clear_Line(i);
    }
    
    // 显示操作提示
    OLED_Printf_Line(6, "KEY0/1:Select KEY3:Detail");
    OLED_Printf_Line(7, "KEY2:Return");
}

/**
 * @brief 显示空列表提示
 */
void alarm_list_display_empty(void)
{
    OLED_Clear();
    OLED_Printf_Line(1, " NO ALARMS ");
    OLED_Printf_Line(2, "   FOUND   ");
    OLED_Printf_Line(3, " Press KEY2 ");
    OLED_Printf_Line(4, " to Return  ");
}

// ==================================
// 详情视图处理
// ==================================

/**
 * @brief 进入详情视图
 * @param state 列表状态
 * @param alarm_index 闹钟索引
 */
void alarm_list_enter_detail(alarm_list_state_t *state, uint8_t alarm_index)
{
    if (alarm_index >= Alarm_GetCount()) {
        return;
    }
    
    state->in_detail_view = 1;
    state->detail_selected = 0;
    state->operating = 0;
    state->need_refresh = 1;
    
    printf("Enter detail view for alarm index %d\r\n", alarm_index);
}

/**
 * @brief 退出详情视图
 * @param state 列表状态
 */
void alarm_list_exit_detail(alarm_list_state_t *state)
{
    state->in_detail_view = 0;
    state->detail_selected = 0;
    state->operating = 0;
    state->need_refresh = 1;
    
    printf("Exit detail view\r\n");
}

/**
 * @brief 处理详情视图按键
 * @param state 列表状态
 * @param key_event 按键事件
 */
void alarm_list_process_detail(alarm_list_state_t *state, uint8_t key_event)
{
    switch (key_event) {
        case MENU_EVENT_KEY_UP:
            // KEY0 - 上一个选项
            printf("Alarm Detail: KEY0 pressed - Previous option\r\n");
            if (state->detail_selected == 0) {
                state->detail_selected = ALARM_DETAIL_OPTIONS_NUM - 1; // 循环到最后一项
            } else {
                state->detail_selected--;
            }
            state->need_refresh = 1;
            break;
            
        case MENU_EVENT_KEY_DOWN:
            // KEY1 - 下一个选项
            printf("Alarm Detail: KEY1 pressed - Next option\r\n");
            state->detail_selected++;
            if (state->detail_selected >= ALARM_DETAIL_OPTIONS_NUM) {
                state->detail_selected = 0; // 循环到第一项
            }
            state->need_refresh = 1;
            break;
            
        case MENU_EVENT_KEY_SELECT:
            // KEY2 - 返回列表
            printf("Alarm Detail: KEY2 pressed - Back to list\r\n");
            alarm_list_exit_detail(state);
            break;
            
        case MENU_EVENT_KEY_ENTER:
            // KEY3 - 确认操作
            printf("Alarm Detail: KEY3 pressed - Confirm action\r\n");
            alarm_list_handle_detail_action(state);
            break;
            
        default:
            break;
    }
}

/**
 * @brief 处理详情视图操作
 * @param state 列表状态
 */
void alarm_list_handle_detail_action(alarm_list_state_t *state)
{
    Alarm_TypeDef *alarm = Alarm_Get(state->selected);
    if (alarm == NULL) {
        return;
    }
    
    switch (state->detail_selected) {
        case DETAIL_OPTION_TIME: // 时间选项 - 进入修改页面
            printf("Alarm Detail: Edit time\r\n");
            // 调用闹钟编辑功能：先读取当前闹钟设置的时间，新建闹钟，再删除原闹钟
            alarm_list_edit_time(state);
            break;
            
        case DETAIL_OPTION_STATUS: // 切换状态
            printf("Alarm Detail: Toggle status\r\n");
            if (alarm->enabled) {
                Alarm_Disable(state->selected);
            } else {
                Alarm_Enable(state->selected);
            }
            state->need_refresh = 1;
            break;
            
        case DETAIL_OPTION_REPEAT: // 切换重复状态
            printf("Alarm Detail: Toggle repeat\r\n");
            alarm->repeat = !alarm->repeat;
            state->need_refresh = 1;
            break;
            
        case DETAIL_OPTION_DELETE: // 删除闹钟
            printf("Alarm Detail: Delete alarm\r\n");
            Alarm_Delete(state->selected);
            OLED_Clear();
            OLED_Printf_Line(1, " ALARM DELETED ");
            OLED_Refresh();
            vTaskDelay(pdMS_TO_TICKS(1000));
            
            // 如果删除后没有闹钟了，直接退出详情视图
            if (Alarm_GetCount() == 0) {
                alarm_list_exit_detail(state);
            } else {
                // 调整选中项
                if (state->selected >= Alarm_GetCount()) {
                    state->selected = Alarm_GetCount() - 1;
                }
                alarm_list_exit_detail(state);
            }
            break;
    }
}

/**
 * @brief 显示闹钟详情
 * @param state 列表状态
 * @param alarm_index 闹钟索引
 */
void alarm_list_display_detail(alarm_list_state_t *state, uint8_t alarm_index)
{
    if (alarm_index >= Alarm_GetCount()) {
        return;
    }
    
    Alarm_TypeDef *alarm = Alarm_Get(alarm_index);
    if (alarm == NULL) {
        return;
    }
    
    // 显示闹钟详情（参考你的代码格式）
    OLED_Printf_Line(0, "%c Time: %02d:%02d:%02d", 
                     state->detail_selected == DETAIL_OPTION_TIME ? '>' : ' ', 
                     alarm->hour, alarm->minute, alarm->second);
    OLED_Printf_Line(1, "%c Status: %s", 
                     state->detail_selected == DETAIL_OPTION_STATUS ? '>' : ' ', 
                     alarm->enabled ? "ENABLED " : "DISABLED");
    OLED_Printf_Line(2, "%c Repeat: %s", 
                     state->detail_selected == DETAIL_OPTION_REPEAT ? '>' : ' ', 
                     alarm->repeat ? "YES" : "NO");
    OLED_Printf_Line(3, "%c Delete", 
                     state->detail_selected == DETAIL_OPTION_DELETE ? '>' : ' ');
    
    // 显示操作提示
    OLED_Printf_Line(6, "KEY0/1:Select KEY3:Action");
    OLED_Printf_Line(7, "KEY2:Back");
    }

// ==================================
// 闹钟编辑功能实现
// ==================================

/**
 * @brief 编辑闹钟时间（使用alarm_add功能）
 * @param state 列表状态
 */
static void alarm_list_edit_time(alarm_list_state_t *state)
{
    if (state->selected >= Alarm_GetCount()) {
        return;
    }
    
    // 获取当前选中的闹钟
    Alarm_TypeDef *original_alarm = Alarm_Get(state->selected);
    if (original_alarm == NULL) {
        return;
    }
    
    // 显示编辑提示
    OLED_Clear();
    OLED_Printf_Line(1, " EDITING ALARM ");
    OLED_Printf_Line(2, " %02d:%02d:%02d ", 
                     original_alarm->hour, original_alarm->minute, original_alarm->second);
    OLED_Printf_Line(3, " Loading... ");
    OLED_Refresh();
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // 创建临时闹钟结构体，复制原闹钟数据
    Alarm_TypeDef new_alarm;
    memcpy(&new_alarm, original_alarm, sizeof(Alarm_TypeDef));
    
    // 创建新的闹钟设置状态结构体
    alarm_add_state_t temp_state;
    memset(&temp_state, 0, sizeof(alarm_add_state_t));
    
    // 使用原闹钟的时间初始化临时状态
    temp_state.temp_alarm.hour = original_alarm->hour;
    temp_state.temp_alarm.minute = original_alarm->minute;
    temp_state.temp_alarm.second = original_alarm->second;
    temp_state.temp_alarm.repeat = original_alarm->repeat;
    temp_state.temp_alarm.enabled = original_alarm->enabled;
    
    temp_state.set_step = SET_STEP_HOUR;
    temp_state.need_refresh = 1;
    
    // 显示编辑界面标题
    OLED_Clear();
    OLED_Printf_Line(0, "  EDIT ALARM ");
    
    // 进入编辑模式
    state->editing_mode = 1;
    state->edit_step = SET_STEP_HOUR;
    state->need_refresh = 1;
}

// ==================================
// 编辑模式处理函数
// ==================================

/**
 * @brief 退出编辑模式
 * @param state 列表状态
 */
static void alarm_list_exit_edit(alarm_list_state_t *state)
{
    state->editing_mode = 0;
    state->edit_step = 0;
    state->need_refresh = 1;
    
    printf("Exit edit mode\r\n");
}

/**
 * @brief 处理编辑模式按键
 * @param state 列表状态
 * @param key_event 按键事件
 */
static void alarm_list_process_edit(alarm_list_state_t *state, uint8_t key_event)
{
    if (state == NULL) {
        return;
    }
    
    printf("Alarm Edit: KEY%d pressed\r\n", key_event - 1);
    
    switch (key_event) {
        case MENU_EVENT_KEY_UP:
            // KEY0 - 增加当前设置项的值
            switch (state->edit_step) {
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
            
        case MENU_EVENT_KEY_DOWN:
            // KEY1 - 减少当前设置项的值
            switch (state->edit_step) {
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
            
        case MENU_EVENT_KEY_SELECT:
            // KEY2 - 确认保存
            printf("Alarm Edit: Save alarm\r\n");
            
            // 保存新闹钟
            if (Alarm_Add(&state->temp_alarm) == 0) {
                // 删除原闹钟
                Alarm_Delete(state->selected);
                
                OLED_Clear();
                OLED_Printf_Line(1, " ALARM UPDATED ");
                OLED_Printf_Line(2, " SUCCESSFULLY! ");
                OLED_Refresh();
                vTaskDelay(pdMS_TO_TICKS(1000));
                
                // 如果删除后没有闹钟了，直接退出详情视图
                if (Alarm_GetCount() == 0) {
                    alarm_list_exit_detail(state);
                    alarm_list_exit_edit(state);
                } else {
                    // 调整选中项
                    if (state->selected >= Alarm_GetCount()) {
                        state->selected = Alarm_GetCount() - 1;
                    }
                    alarm_list_exit_detail(state);
                    alarm_list_exit_edit(state);
                }
            } else {
                OLED_Clear();
                OLED_Printf_Line(1, " UPDATE FAILED ");
                OLED_Refresh();
                vTaskDelay(pdMS_TO_TICKS(1000));
                state->need_refresh = 1;
            }
            break;
            
        case MENU_EVENT_KEY_ENTER:
            // KEY3 - 下一个设置项
            state->edit_step++;
            if (state->edit_step >= SET_STEP_COUNT) {
                state->edit_step = 0; // 循环回到第一项
            }
            state->need_refresh = 1;
            break;
            
        default:
            break;
    }
}

/**
 * @brief 显示编辑界面
 * @param state 列表状态
 */
static void alarm_list_display_edit(alarm_list_state_t *state)
{
    if (!state) {
        return;
    }
    
    // 根据设置步骤高亮显示当前设置项
    switch (state->edit_step) {
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
    
    // 显示标题和操作提示
    OLED_Printf_Line(0, "  EDIT ALARM");
    OLED_Printf_Line(3, "KEY0:+ KEY1:- KEY2:OK");
}
