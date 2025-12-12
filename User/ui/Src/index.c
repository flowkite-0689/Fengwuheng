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

    // 显示时间信息
    index_display_time_info();

    // 显示状态信息
    index_display_status_info();

    // 刷新显示
    OLED_Refresh_Dirty();
}

void index_key_handler(menu_item_t *item, uint8_t key_event)
{
    index_state_t *state = (index_state_t *)item->content.custom.draw_context;

    switch (key_event)
    {
    case MENU_EVENT_KEY_UP:
        // KEY0 - 可以用来切换某些状态或进入特定功能
        printf("Index: KEY0 pressed\r\n");
        break;

    case MENU_EVENT_KEY_DOWN:
        // KEY1 - 可以用来切换某些状态或进入特定功能
        printf("Index: KEY1 pressed\r\n");
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

static void index_display_time_info(void)
{
    // 显示日期和星期：第0行
    OLED_Printf_Line(0, "%02d/%02d/%02d  T : %2d.%1d",
                     g_index_state.year,
                     g_index_state.month,
                     g_index_state.day,
                     SensorData.dht11_data.temp_int,
                     SensorData.dht11_data.temp_deci);

    // 显示时间：第1行，使用32像素字体
    OLED_Printf_Line(1, " %02d:%02d:%02d   H : %2d ",
                     g_index_state.hours,
                     g_index_state.minutes,
                     g_index_state.seconds,
                      SensorData.dht11_data.humi_int

    );

    OLED_Printf_Line(2, " wifi:%s    L : %2d ",

                     wifi_connected ? "OK" : "NO",
                     SensorData.light_data.lux);
    OLED_Printf_Line(3, "Server:%s   P : %3.1f ",
                     Server_connected ? "OK" : "NO",
                     SensorData.pm25_data.pm25_value);
}

static void index_display_status_info(void)
{

    //温度·
    OLED_DrawProgressBar(64,13,64,3, SensorData.dht11_data.temp_int,0,50,1,1,0);
    //湿度

    OLED_DrawProgressBar(64, 29, 64, 3, SensorData.dht11_data.humi_int, 0, 100, 1, 1, 0);
    //流明

    OLED_DrawProgressBar(64, 45, 64, 3, SensorData.light_data.lux, 0, 987, 1, 1, 0);

    //PM2.5
    OLED_DrawProgressBar(64, 61, 64, 3, SensorData.pm25_data.pm25_value, 0, 250, 1, 1, 0);


    //秒onds进度条
    OLED_DrawProgressBar(60, 0, 2, 64, g_index_state.seconds, 0, 60, 0, 1, 1);
    
}
