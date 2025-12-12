#include "WiFiStatus.h"
#include "esp8266.h"
#include "rtc_date.h"

// 声明外部变量
extern uint8_t wifi_connected;
extern uint8_t Server_connected;

// 定义静态状态变量，避免动态内存分配
WiFiStatus_state_t g_wifistatus_state = {0};

// ==================================
// 静态函数声明
// ==================================
static void WiFiStatus_init_state(WiFiStatus_state_t *state);
static void WiFiStatus_cleanup_state(WiFiStatus_state_t *state);
static void WiFiStatus_display_info(void *context);
static uint8_t WiFiStatus_sync_time(void);

/**
 * @brief 初始化WiFi状态页面
 * @return 创建的WiFi状态菜单项指针
 */
menu_item_t *WiFiStatus_init(void)
{
  // 创建自定义菜单项，不分配具体状态数据
  menu_item_t *WiFiStatus_page = MENU_ITEM_CUSTOM("WiFi Status", WiFiStatus_draw_function, NULL);
  if (WiFiStatus_page == NULL)
  {
    return NULL;
  }

  menu_item_set_callbacks(WiFiStatus_page, WiFiStatus_on_enter, WiFiStatus_on_exit, NULL, WiFiStatus_key_handler);

  printf("WiFiStatus_page created successfully\r\n");
  return WiFiStatus_page;
}

/**
 * @brief WiFi状态自定义绘制函数
 * @param context 绘制上下文
 */
void WiFiStatus_draw_function(void *context)
{
  WiFiStatus_state_t *state = (WiFiStatus_state_t *)context;
  if (state == NULL)
  {
    return;
  }

  // 更新连接状态
  state->wifi_status = wifi_connected;
  state->server_status = Server_connected;

  WiFiStatus_display_info(state);

  OLED_Refresh_Dirty();
}

void WiFiStatus_key_handler(menu_item_t *item, uint8_t key_event)
{
  WiFiStatus_state_t *state = (WiFiStatus_state_t *)item->content.custom.draw_context;
  if (state == NULL) {
    return;
  }
  
  switch (key_event)
  {
  case MENU_EVENT_KEY_UP:
    // KEY0 - 手动尝试同步时间
    printf("WiFiStatus: KEY0 pressed - Attempt time sync\r\n");
    if (state->wifi_status && state->server_status) {
      WiFiStatus_sync_time();
    }
    break;

  case MENU_EVENT_KEY_DOWN:
    // KEY1 - 刷新显示
    printf("WiFiStatus: KEY1 pressed - Refresh display\r\n");
    OLED_Clear();
    state->need_refresh = 1;
    break;

  case MENU_EVENT_KEY_SELECT:
    // KEY2 - 返回上一级
    printf("WiFiStatus: KEY2 pressed - Back to parent\r\n");
    menu_back_to_parent();
    break;

  case MENU_EVENT_KEY_ENTER:
    // KEY3 - 可以用于其他功能
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
 * @brief 从上下文获取WiFi状态结构体指针
 * @param context 指向WiFiStatus状态结构体的上下文指针
 * @return 返回指向WiFiStatus状态结构体的指针
 */
WiFiStatus_state_t *WiFiStatus_get_state(void *context)
{
  return (WiFiStatus_state_t *)context;
}

void WiFiStatus_refresh_display(void *context)
{
  WiFiStatus_state_t *state = (WiFiStatus_state_t *)context;
  if (state == NULL) {
    return;
  }
  
  state->need_refresh = 1;
  state->last_update = xTaskGetTickCount();
}

void WiFiStatus_on_enter(menu_item_t *item)
{
  printf("Enter WiFiStatus page\r\n");
  
  // 使用静态分配的状态数据，直接初始化
  WiFiStatus_init_state(&g_wifistatus_state);
  
  // 设置静态状态到菜单项上下文
  item->content.custom.draw_context = &g_wifistatus_state;
  
  // 清屏并标记需要刷新
  OLED_Clear();
  g_wifistatus_state.need_refresh = 1;
  
  // 如果WiFi和服务器已连接，尝试同步时间
  if (g_wifistatus_state.wifi_status && g_wifistatus_state.server_status && 
      !g_wifistatus_state.time_sync_attempted) {
    printf("WiFi and Server connected, attempting time sync...\r\n");
    WiFiStatus_sync_time();
  }
}

void WiFiStatus_on_exit(menu_item_t *item)
{
  printf("Exit WiFiStatus page\r\n");
  
  WiFiStatus_state_t *state = (WiFiStatus_state_t *)item->content.custom.draw_context;
  if (state == NULL) {
    return;
  }
  
  // 清理状态数据
  WiFiStatus_cleanup_state(state);
  
  printf("WiFiStatus state cleaned up (no memory free needed)\r\n");
  
  // 清空指针，防止野指针
  item->content.custom.draw_context = NULL;
  
  // 清屏
  OLED_Clear();
}

// ==================================
// 状态初始化与清理
// ==================================

/**
 * @brief 初始化WiFi状态数据
 * @param state 状态指针
 */
static void WiFiStatus_init_state(WiFiStatus_state_t *state)
{
    if (state == NULL) {
        return;
    }
    
    // 清零状态结构体
    memset(state, 0, sizeof(WiFiStatus_state_t));
    
    // 初始化状态
    state->wifi_status = wifi_connected;
    state->server_status = Server_connected;
    state->time_sync_status = 0;
    state->time_sync_attempted = 0;
    state->need_refresh = 1;
    state->last_update = xTaskGetTickCount();
    state->last_time_sync = 0;
    
    printf("WiFiStatus state initialized\r\n");
}

/**
 * @brief 清理状态数据
 * @param state 状态指针
 */
static void WiFiStatus_cleanup_state(WiFiStatus_state_t *state)
{
    if (state == NULL) {
        return;
    }
    
    printf("WiFiStatus state cleaned up\r\n");
}

// ==================================
// 时间同步功能
// ==================================

/**
 * @brief 同步时间函数
 * @return 1-成功，0-失败
 */
static uint8_t WiFiStatus_sync_time(void)
{
    WiFiStatus_state_t *state = &g_wifistatus_state;
    
    // 标记已尝试同步
    state->time_sync_attempted = 1;
    state->time_sync_status = 0;
    
    // 检查连接状态
    if (!state->wifi_status || !state->server_status) {
        printf("WiFiStatus: Cannot sync time - WiFi or Server not connected\r\n");
        return 0;
    }
    
    // ====================== 获取时间（带重试） ======================
    uint8_t retry_count = 0;
    uint8_t max_retries = 3;
    uint8_t get_time_success = 0;
    
    printf("WiFiStatus: Starting time sync...\r\n");
    
    while (!get_time_success && retry_count < max_retries)
    {
        retry_count++;
        printf("WiFiStatus: Get Time attempt %d/%d\r\n", retry_count, max_retries);
        
        if (ESP8266_TCP_GetTime("1", state->time_buffer, sizeof(state->time_buffer)) == 1)
        {
            printf("WiFiStatus: ESP8266 Get Time Success: %s\r\n", state->time_buffer);
            get_time_success = 1;
        }
        else
        {
            printf("WiFiStatus: ESP8266 Get Time Error, attempt %d/%d\r\n", retry_count, max_retries);
            if (retry_count < max_retries)
            {
                // 等待5秒后重试
                vTaskDelay(pdMS_TO_TICKS(5000));
            }
        }
    }

    if (!get_time_success)
    {
        printf("WiFiStatus: Get Time failed after %d attempts, entering retry loop\r\n", max_retries);

        // 进入无限重试循环
        while (!get_time_success)
        {
            vTaskDelay(pdMS_TO_TICKS(30000)); // 每30秒重试一次
            printf("WiFiStatus: Retrying Get Time...\r\n");
            if (ESP8266_TCP_GetTime("1", state->time_buffer, sizeof(state->time_buffer)) == 1)
            {
                printf("WiFiStatus: ESP8266 Get Time Success after retry: %s\r\n", state->time_buffer);
                get_time_success = 1;
            }
        }
    }

    // 同步到RTC
    if (RTC_SetFromNetworkTime(state->time_buffer) != 1)
    {
        printf("WiFiStatus: RTC Sync Failed\r\n");
        state->time_sync_status = 0;
        return 0;
    }
    else
    {
        printf("WiFiStatus: RTC Sync Success\r\n");
        state->time_sync_status = 1;
        state->last_time_sync = xTaskGetTickCount();
        return 1;
    }
}

// ==================================
// 显示信息函数
// ==================================

static void WiFiStatus_display_info(void *context)
{
  WiFiStatus_state_t *state = (WiFiStatus_state_t *)context;
  if (state == NULL) {
    return;
  }
  
  // 清屏并显示标题
  // OLED_Clear();
  OLED_Printf_Line_32(0, "WiFiStatus");
  
  // 显示WiFi连接状态
  OLED_Printf_Line(2, "WiFi: %s", state->wifi_status ? "Connected" : "Disconnected");
  
  // 显示服务器连接状态
  OLED_Printf_Line(3, "Server: %s", state->server_status ? "Connected" : "Disconnected");
  
  // 显示时间同步状态
  if (state->time_sync_attempted) {
    OLED_Printf_Line(4, "Time Sync: %s", state->time_sync_status ? "Success" : "Failed");
  } else {
    OLED_Printf_Line(4, "Time Sync: Not Attempted");
  }
  
  // 显示时间信息
  if (state->time_sync_status && strlen(state->time_buffer) > 0) {
    OLED_Printf_Line(5, "Time: %s", state->time_buffer);
  }
  
  // 显示操作提示
  OLED_Printf_Line(6, "KEY0: Sync KEY1: Refresh");
  OLED_Printf_Line(7, "KEY2: Back");
}
