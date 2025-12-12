#ifndef _WIFISTATUS_H_
#define _WIFISTATUS_H_

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "unified_menu.h"
#include "oled_print.h"
#include "esp8266.h"
#include "rtc_date.h"

// 声明外部WiFi状态变量
extern uint8_t wifi_connected;
extern uint8_t Server_connected;

typedef struct
{
   uint8_t wifi_status;           // WiFi连接状态
   uint8_t server_status;         // 服务器连接状态
   uint8_t time_sync_status;      // 时间同步状态
   uint8_t time_sync_attempted;   // 是否已尝试同步时间
   uint8_t need_refresh;          // 需要刷新
   uint32_t last_update;          // 上次更新时间
   uint32_t last_time_sync;       // 上次时间同步时间
   char time_buffer[64];          // 时间缓冲区
   
} WiFiStatus_state_t;

// 声明静态状态变量，避免动态内存分配
extern WiFiStatus_state_t g_wifistatus_state;

/**
 * @brief 初始化WiFi状态页面
 * @return 创建的WiFi状态菜单项指针
 */
menu_item_t *WiFiStatus_init(void);

/**
 * @brief WiFi状态自定义绘制函数
 * @param context 绘制上下文
 */
void WiFiStatus_draw_function(void *context);

void WiFiStatus_key_handler(menu_item_t *item, uint8_t key_event);

WiFiStatus_state_t *WiFiStatus_get_state(void *context);

void WiFiStatus_refresh_display(void *context);

void WiFiStatus_on_enter(menu_item_t *item);

void WiFiStatus_on_exit(menu_item_t *item);

#endif