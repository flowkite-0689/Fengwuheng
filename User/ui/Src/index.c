/**
 * @file index.c
 * @brief 首页界面实现文件
 * @author flowkite-0689
 * @version v1.0
 * @date 2025.12.05
 */

#include "index.h"
#include "main_menu.h"
#include <string.h>
#include "simple_pedometer.h"
#include "light.h"
#include "esp8266.h"
// ==================================
// 全局变量定义
// ==================================

index_state_t g_index_state = {0};

// ==================================
// 静态函数声明
// ==================================

static void index_display_time_info(void);
static void index_display_status_info(void);
static void index_update_scroll(void);
static void index_scroll_to_offset(uint8_t target_offset);

// ==================================
// 首页实现
// ==================================

menu_item_t *index_init(void)
{
    // 初始化首页状态
    memset(&g_index_state, 0, sizeof(index_state_t));
    g_index_state.need_refresh = 1;
    g_index_state.last_update = xTaskGetTickCount();
    g_index_state.step_count = 0;
    g_index_state.step_active = 0;
    g_index_state.scroll_offset = 0; // 初始不滚动
    g_index_state.scroll_direction = 0;
    g_index_state.scroll_step = 0;

    // 初始化RTC
    MyRTC_Init();

    // 创建首页菜单项
    menu_item_t *index_menu = MENU_ITEM_CUSTOM("Index", index_draw_function, &g_index_state);
    if (index_menu == NULL)
    {
        return NULL;
    }

    // 设置回调函数
    menu_item_set_callbacks(index_menu, index_on_enter, index_on_exit, NULL, index_key_handler);

    // 创建并添加主菜单作为子菜单
    menu_item_t *main_menu = main_menu_init();
    if (main_menu != NULL)
    {
        menu_add_child(index_menu, main_menu);
    }

    printf("Index page initialized successfully\r\n");

    return index_menu;
}

void index_draw_function(void *context)
{
    index_state_t *state = (index_state_t *)context;
    if (state == NULL)
    {
        return;
    }

    // 更新时间信息
    index_update_time();
    
    // 更新滚动状态
    index_update_scroll();

    // 显示时间信息
    index_display_time_info();

    // 显示状态信息
    index_display_status_info();

    OLED_Refresh(); // 改为全屏刷新，确保内容正确显示
}

void index_key_handler(menu_item_t *item, uint8_t key_event)
{
    index_state_t *state = (index_state_t *)item->content.custom.draw_context;

    switch (key_event)
    {
    case MENU_EVENT_KEY_UP:
        // KEY0 - 回到原始位置
        printf("Index: KEY0 pressed - Scroll to original position\r\n");
        index_scroll_to_offset(0);
        break;

    case MENU_EVENT_KEY_DOWN:
        // KEY1 - 向右滚动64像素
        printf("Index: KEY1 pressed - Scroll right 64 pixels\r\n");
        index_scroll_to_offset(64);
        break;

    case MENU_EVENT_KEY_SELECT:
        // KEY2 - 可以返回上一级或特定功能
        printf("Index: KEY2 pressed\r\n");
        break;

    case MENU_EVENT_KEY_ENTER:
        // KEY3 - 进入主菜单
        printf("Index: KEY3 pressed - Enter main menu\r\n");
        // 调用统一框架的进入选中函数
        menu_enter_selected();
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

void index_update_time(void)
{
    // 读取RTC时间
    MyRTC_ReadTime();

    // 更新状态
    g_index_state.hours = RTC_data.hours;
    g_index_state.minutes = RTC_data.minutes;
    g_index_state.seconds = RTC_data.seconds;
    g_index_state.day = RTC_data.day;
    g_index_state.month = RTC_data.mon;
    g_index_state.year = RTC_data.year;
    strncpy(g_index_state.weekday, RTC_data.weekday, sizeof(g_index_state.weekday) - 1);
    g_index_state.weekday[sizeof(g_index_state.weekday) - 1] = '\0';
}

index_state_t *index_get_state(void)
{
    return &g_index_state;
}

void index_refresh_display(void)
{
    g_index_state.need_refresh = 1;
    g_index_state.last_update = xTaskGetTickCount();
}

void index_on_enter(menu_item_t *item)
{
    printf("Enter index page\r\n");
    // 初始化滚动状态
    g_index_state.scroll_offset = 0;
    g_index_state.scroll_direction = 0;
    g_index_state.scroll_step = 0;
    OLED_Clear();
    g_index_state.need_refresh = 1;
}

void index_on_exit(menu_item_t *item)
{
    printf("Exit index page\r\n");
    OLED_Clear();
}

// ==================================
// 静态函数实现
// ==================================

/**
 * @brief 更新滚动状态
 */
static void index_update_scroll(void)
{
    index_state_t *state = &g_index_state;
    
    // 如果正在滚动，逐步更新
    if (state->scroll_direction != 0 && state->scroll_step < 8)
    {
        state->scroll_step++;
        
        // 根据方向更新偏移量，每次8像素
        if (state->scroll_direction == 1) // 向右
        {
            state->scroll_offset = (state->scroll_step * 8);
        }
        else if (state->scroll_direction == 2) // 向左
        {
            state->scroll_offset = 64 - (state->scroll_step * 8);
        }
        
        // 调试信息
        printf("Scroll step %d, direction %d, offset %d\r\n", 
               state->scroll_step, state->scroll_direction, state->scroll_offset);
        
        state->need_refresh = 1;
        
        // 滚动完成
        if (state->scroll_step >= 8)
        {
            if (state->scroll_direction == 1)
            {
                state->scroll_offset = 64;
            }
            else if (state->scroll_direction == 2)
            {
                state->scroll_offset = 0;
            }
            state->scroll_direction = 0;
            // 任何滚动完成后都强制清屏，确保内容正确显示
            OLED_Clear();
            printf("Scroll completed, final offset: %d\r\n", state->scroll_offset);
        }
    }
}

/**
 * @brief 设置滚动到指定偏移量
 * @param target_offset 目标偏移量(0或64)
 */
static void index_scroll_to_offset(uint8_t target_offset)
{
    index_state_t *state = &g_index_state;
    
    // 检查是否需要滚动
    if (state->scroll_offset == target_offset)
    {
        printf("Already at target offset %d, no scroll needed\r\n", target_offset);
        return;
    }
    
    // 设置滚动方向和步骤
    if (target_offset > state->scroll_offset)
    {
        state->scroll_direction = 1; // 向右
        printf("Starting scroll right from %d to %d\r\n", state->scroll_offset, target_offset);
    }
    else
    {
        state->scroll_direction = 2; // 向左
        printf("Starting scroll left from %d to %d\r\n", state->scroll_offset, target_offset);
    }
    
    state->scroll_step = 0;
    state->need_refresh = 1;
}

static void index_display_time_info(void)
{
    index_state_t *state = &g_index_state;
    uint8_t x_offset = state->scroll_offset; // 获取当前滚动偏移
    
    if (x_offset > 0)
    {
          //在页面左边64像素位置放图标
          if (wifi_connected)
          {
            OLED_ShowPicture(-64+x_offset, 0,32, 32,gImage_wifi, 1);
          }
          
    
    OLED_ShowPicture(-32+x_offset, 0,32, 32,gImage_lightQD, 1);
    OLED_ShowPicture(-64+x_offset, 32,32, 32,gImage_TandH, 1);
    OLED_ShowPicture(-32+x_offset, 32,32, 32,gImage_pm25, 1);
    }
    
  

    // 应用滚动偏移显示内容
    if (DHT11_ON)
    {
    OLED_Printf(x_offset, 0, "%02d/%02d/%02d  T : %2d.%1d",
                     g_index_state.year,
                     g_index_state.month,
                     g_index_state.day,
                     SensorData.dht11_data.temp_int,
                     SensorData.dht11_data.temp_deci);

   
    OLED_Printf(x_offset, 16, " %02d:%02d:%02d   H : %2d ",
                     g_index_state.hours,
                     g_index_state.minutes,
                     g_index_state.seconds,
                      SensorData.dht11_data.humi_int

    );
    }else
    {
        OLED_Printf(x_offset, 0, "%02d/%02d/%02d  T : OFF",
                     g_index_state.year,
                     g_index_state.month,
                     g_index_state.day
                    );

   
    OLED_Printf(x_offset, 16, " %02d:%02d:%02d   H : OFF ",
                     g_index_state.hours,
                     g_index_state.minutes,
                     g_index_state.seconds);
    }
    
   
    if (Light_ON)
    {
         OLED_Printf(x_offset, 32, " wifi:%s    L : %2d ",

                     wifi_connected ? "OK" : "NO",
                     SensorData.light_data.lux);
    }else
    {
        OLED_Printf(x_offset, 32, " wifi:%s    L : OFF ",

                     wifi_connected ? "OK" : "NO");

    }
    
   if (PM25_ON)
   {
     OLED_Printf(x_offset, 48, "Server:%s   P : %3.1f ",
                     Server_connected ? "OK" : "NO",
                     SensorData.pm25_data.pm25_value);
   }else
   {
     OLED_Printf(x_offset, 48, "Server:%s   P : OFF ",
                     Server_connected ? "OK" : "NO");

   }
   
   
}

static void index_display_status_info(void)
{
    index_state_t *state = &g_index_state;
    uint8_t x_offset = state->scroll_offset; // 获取当前滚动偏移

    //秒onds进度条
    OLED_DrawProgressBar(60 + x_offset, 0, 2, 64, g_index_state.seconds, 0, 60, 0, 1, 1);
}
