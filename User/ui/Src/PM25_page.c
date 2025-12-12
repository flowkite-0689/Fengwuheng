#include "PM25_page.h"
#include "esp8266.h"

// 定义静态状态变量，避免动态内存分配
PM25_state_t g_pm25_state = {0};
// ==================================
// 静态函数声明
// ==================================
static void PM25_init_sensor_data(PM25_state_t *state);
static void PM25_cleanup_sensor_data(PM25_state_t *state);
static void PM25_display_info(void *context);

// PM2.5进度条（line=1）
void OLED_DrawPM25Bar_Line1(uint16_t pm25_value)
{
  OLED_Clear_Line(1);
  // 标签
  OLED_ShowString(0, 16, (uint8_t *)"0", 12, 1);
  OLED_ShowString(105, 16, (uint8_t *)"300", 12, 1);
  // 进度条：x=25, y=18, w=78, h=8, 0~300 ug/m3
  OLED_DrawProgressBar(22, 18, 78, 8, pm25_value, 0, 300, 1, 1,1);
}

// PM2.5等级描述获取函数
const char* PM25_GetLevelString(uint8_t level)
{
  switch(level)
  {
    case PM25_LEVEL_GOOD: return "Good";
    case PM25_LEVEL_MODERATE: return "Moderate";
    case PM25_LEVEL_UNHEALTHY_SENSITIVE: return "Unhealthy(S)";
    case PM25_LEVEL_UNHEALTHY: return "Unhealthy";
    case PM25_LEVEL_VERY_UNHEALTHY: return "Very Unh";
    case PM25_LEVEL_HAZARDOUS: return "Hazardous";
    default: return "Unknown";
  }
}

/**
 * @brief 初始化PM2.5页面
 * @return 创建的PM2.5菜单项指针
 */
menu_item_t *PM25_init(void)
{
  // 创建自定义菜单项，不分配具体状态数据
  menu_item_t *PM25_page = MENU_ITEM_CUSTOM("PM2.5", PM25_draw_function, NULL);
  if (PM25_page == NULL)
  {
    return NULL;
  }

  menu_item_set_callbacks(PM25_page, PM25_on_enter, PM25_on_exit, NULL, PM25_key_handler);

  printf("PM25_page created successfully\r\n");
  return PM25_page;
}

/**
 * @brief PM2.5自定义绘制函数
 * @param context 绘制上下文
 */
void PM25_draw_function(void *context)
{
  if (PM25_ON)
  {
     PM25_state_t *state = (PM25_state_t *)context;
  if (state == NULL)
  {
    return;
  }

  // PM2.5数据由全局SensorData任务定期更新，无需单独读取

  PM25_display_info(state);
   
  
  OLED_Refresh_Dirty();
  }
  else
  {
    OLED_Printf_Line_32(0,"No Data");
    OLED_Printf_Line_32(2,"pm25 off");
    OLED_Refresh_Dirty();
  }
  
 
}

void PM25_key_handler(menu_item_t *item, uint8_t key_event)
{
  PM25_state_t *state = (PM25_state_t *)item->content.custom.draw_context;
  if (state == NULL) {
    return;
  }
  
  switch (key_event)
  {
  case MENU_EVENT_KEY_UP:
    // KEY0 - 开启PM2.5传感器
    printf("PM25: KEY0 pressed\r\n");
    PM25_ON=1;
    break;

  case MENU_EVENT_KEY_DOWN:
    // KEY1 - 关闭PM2.5传感器
    printf("PM25: KEY1 pressed\r\n");
    OLED_Clear();
    PM25_ON=0;
    break;

  case MENU_EVENT_KEY_SELECT:
    // KEY2 - 返回上一级
    printf("PM25: KEY2 pressed\r\n");
    menu_back_to_parent();
    break;

  case MENU_EVENT_KEY_ENTER:

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
 * @brief 从上下文获取PM2.5状态结构体指针
 * @param context 指向PM2.5状态结构体的上下文指针
 * @return 返回指向PM2.5状态结构体的指针
 */
PM25_state_t *PM25_get_state(void *context)
{
  return (PM25_state_t *)context;
}

void PM25_refresh_display(void *context)
{
  PM25_state_t *state = (PM25_state_t *)context;
  if (state == NULL) {
    return;
  }
  
  state->need_refresh = 1;
  state->last_update = xTaskGetTickCount();
}

void PM25_on_enter(menu_item_t *item)
{
  printf("Enter PM25 page\r\n");
  
  // 使用静态分配的状态数据，直接初始化
  PM25_init_sensor_data(&g_pm25_state);
  
  // 传感器数据已在全局SensorData任务中初始化和读取
  
  // 设置静态状态到菜单项上下文
  item->content.custom.draw_context = &g_pm25_state;
  
  // 清屏并标记需要刷新
  OLED_Clear();
  g_pm25_state.need_refresh = 1;
}

void PM25_on_exit(menu_item_t *item)
{
  printf("Exit PM25 page\r\n");
  
  PM25_state_t *state = (PM25_state_t *)item->content.custom.draw_context;
  if (state == NULL) {
    return;
  }
  
  // 清理传感器相关数据
  PM25_cleanup_sensor_data(state);
  
  printf("PM25 state cleaned up (no memory free needed)\r\n");
  
  // 清空指针，防止野指针
  item->content.custom.draw_context = NULL;
  
  // 清屏
  OLED_Clear();
}

// ==================================
// 传感器数据初始化与清理
// ==================================

/**
 * @brief 初始化传感器状态数据
 * @param state 传感器状态指针
 */
static void PM25_init_sensor_data(PM25_state_t *state)
{
    if (state == NULL) {
        return;
    }
    
    // 清零状态结构体
    memset(state, 0, sizeof(PM25_state_t));
    
    // 初始化状态
    state->need_refresh = 1;
    state->last_update = xTaskGetTickCount();
    state->last_date_PM = 0;
    state->result = 1;
    
    printf("PM25 state initialized\r\n");
}

/**
 * @brief 清理传感器数据
 * @param state 传感器状态指针
 */
static void PM25_cleanup_sensor_data(PM25_state_t *state)
{
    if (state == NULL) {
        return;
    }
    
    printf("PM25 sensor data cleaned up\r\n");
}

static void PM25_display_info(void *context)
{
  PM25_state_t *state = (PM25_state_t *)context;
  if (state == NULL) {
    return;
  }
  
  // 显示PM2.5数值，保留一位小数
  OLED_Clear_Line(0);
  OLED_Printf_Line(0, "PM2.5: %.1f ug/m3", SensorData.pm25_data.pm25_value);
  
  // 显示空气质量等级
  OLED_Clear_Line(2);
  OLED_Printf_Line(2, "Quality: %s", PM25_GetLevelString(SensorData.pm25_data.level));
  OLED_Clear_Line(3);
  // 渐进式更新PM2.5显示值，避免突变
  uint16_t current_pm25 = (uint16_t)(SensorData.pm25_data.pm25_value);
  
  if (current_pm25 > state->last_date_PM)
  {
    if (current_pm25 - state->last_date_PM >= 30)
    {
      state->last_date_PM += 10;
    }
    else
    {
      state->last_date_PM++;
    }
  }
  else if (current_pm25 < state->last_date_PM)
  {
    if (state->last_date_PM - current_pm25 >= 30)
    {
      state->last_date_PM -= 10;
    }
    else
    {
      state->last_date_PM--;
    }
  }
  
  // 确保值不为负
  if (state->last_date_PM < 0) {
    state->last_date_PM = 0;
  }
  
  // 绘制PM2.5进度条
  OLED_DrawPM25Bar_Line1(state->last_date_PM);
}
