/**
 * @file StepCounter.c
 * @brief 步数页面实现文件
 * @author flowkite-0689
 * @version v1.0
 * @date 2025.12.08
 */

#include "StepCounter.h"
#include <string.h>
#include <stdlib.h>

// ==================================
// 本页面变量定义
// ==================================
static StepCounter_state_t s_StepCounter_state = {0};

// ==================================
// 静态函数声明
// ==================================
static void StepCounter_display_info(void);
static void StepCounter_reset_confirm(void);

/**
 * @brief 初始化步数页面
 * @return 创建的步数菜单项指针
 */
menu_item_t *StepCounter_init(void)
{
    memset(&s_StepCounter_state, 0, sizeof(s_StepCounter_state));
    s_StepCounter_state.need_refresh = 1;
    s_StepCounter_state.last_update = xTaskGetTickCount();
    s_StepCounter_state.show_reset_confirm = 0;

    menu_item_t *StepCounter_page = MENU_ITEM_CUSTOM("Step Counter", StepCounter_draw_function, &s_StepCounter_state);
    if (StepCounter_page == NULL)
    {
        return NULL;
    }

    menu_item_set_callbacks(StepCounter_page, StepCounter_on_enter, StepCounter_on_exit, NULL, StepCounter_key_handler);

    printf("StepCounter_page initialized successfully\r\n");
    return StepCounter_page;
}

/**
 * @brief 步数自定义绘制函数
 * @param context 绘制上下文
 */
void StepCounter_draw_function(void *context)
{
    StepCounter_state_t *state = (StepCounter_state_t *)context;
    if (state == NULL)
    {
        return;
    }

    // 如果显示重置确认界面
    if (state->show_reset_confirm) {
        StepCounter_reset_confirm();
    } else {
        StepCounter_display_info();
    }

    OLED_Refresh_Dirty();
}

/**
 * @brief 步数按键处理函数
 * @param item 菜单项
 * @param key_event 按键事件
 */
void StepCounter_key_handler(menu_item_t *item, uint8_t key_event)
{
    StepCounter_state_t *state = (StepCounter_state_t *)item->content.custom.draw_context;
    if (state == NULL) {
        return;
    }
    
    switch (key_event)
    {
    case MENU_EVENT_KEY_UP:
        // KEY0 - 重置步数（需要确认）
        if (!state->show_reset_confirm) {
            // 第一次按下KEY0，显示确认界面
            state->show_reset_confirm = 1;
            printf("StepCounter: Reset confirmation requested\r\n");
        } else {
            // 在确认界面按下KEY0，确认重置
            simple_pedometer_reset();
            state->show_reset_confirm = 0;
            printf("StepCounter: Steps reset to 0\r\n");
        }
        break;

    case MENU_EVENT_KEY_DOWN:
        // KEY1 - 在确认界面按下KEY1，取消重置
        if (state->show_reset_confirm) {
            state->show_reset_confirm = 0;
            printf("StepCounter: Reset cancelled\r\n");
        }
        break;

    case MENU_EVENT_KEY_SELECT:
        // KEY2 - 退出
        printf("Exiting StepCounter\r\n");
        menu_back_to_parent();
        break;

    case MENU_EVENT_KEY_ENTER:
        // KEY3 - 刷新显示（可选功能）
        printf("StepCounter: Manual refresh\r\n");
        break;

    case MENU_EVENT_REFRESH:
        // 刷新显示
        state->need_refresh = 1;
        break;

    default:
        break;
    }

    // 标记需要刷新
    state->need_refresh = 1;
}

/**
 * @brief 获取步数状态
 * @return 步数状态指针
 */
StepCounter_state_t *StepCounter_get_state(void)
{
    return &s_StepCounter_state;
}

/**
 * @brief 刷新步数显示
 */
void StepCounter_refresh_display(void)
{
    s_StepCounter_state.need_refresh = 1;
    s_StepCounter_state.last_update = xTaskGetTickCount();
}

/**
 * @brief 进入步数页面时的回调
 * @param item 菜单项
 */
void StepCounter_on_enter(menu_item_t *item)
{
    printf("Enter StepCounter page\r\n");
    OLED_Clear();
    
    // 初始化步数状态
    s_StepCounter_state.need_refresh = 1;
    s_StepCounter_state.show_reset_confirm = 0;
    
    printf("Current steps: %lu\r\n", simple_pedometer_get_steps());
}

/**
 * @brief 退出步数页面时的回调
 * @param item 菜单项
 */
void StepCounter_on_exit(menu_item_t *item)
{
    printf("Exit StepCounter page\r\n");
    OLED_Clear();
    
    // 退出时确保重置确认界面关闭
    s_StepCounter_state.show_reset_confirm = 0;
}

/**
 * @brief 显示步数界面信息
 */
static void StepCounter_display_info(void)
{
    unsigned long current_steps = simple_pedometer_get_steps();
    
    // 显示标题
    OLED_Printf_Line_32(0, "   STEPS");
    
    // 显示步数（大字体）
    OLED_Printf_Line_32(2, "  %06lu", current_steps);
    
    // 显示操作提示
    OLED_Printf_Line(4, "KEY0:Reset KEY2:Exit");
    OLED_Printf_Line(5, "Keep walking to count!");
}

/**
 * @brief 显示重置确认界面
 */
static void StepCounter_reset_confirm(void)
{
    OLED_Printf_Line_32(1, " Reset Steps?");
    OLED_Printf_Line(3, "   KEY0: YES");
    OLED_Printf_Line(4, "   KEY1: NO");
    OLED_Printf_Line(6, "   Confirm?");
}
