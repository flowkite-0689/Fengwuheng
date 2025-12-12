#include "Light_page.h"
#include "esp8266.h"

// 定义静态状态变量，避免动态内存分配
Light_state_t g_light_state = {0};
// ==================================
// 静态函数声明
// ==================================
static void Light_init_sensor_data(Light_state_t *state);
static void Light_cleanup_sensor_data(Light_state_t *state);
static void Light_display_info(void *context);

// 光照进度条（line=1）
void OLED_DrawLightBar_Line1(uint16_t lux)
{
  OLED_Clear_Line(1);
  // 标签
  OLED_ShowString(0, 16, (uint8_t *)"0", 12, 1);
  OLED_ShowString(105, 16, (uint8_t *)"5000", 12, 1);
  // 进度条：x=25, y=18, w=78, h=8, 0~5000 lux
  OLED_DrawProgressBar(22, 18, 78, 8, lux, 0, 5000, 1, 1,1);
}

/**
 * @brief 初始化光照页面
 * @return 创建的光照菜单项指针
 */
menu_item_t *Light_init(void)
{
  // 创建自定义菜单项，不分配具体状态数据
  menu_item_t *Light_page = MENU_ITEM_CUSTOM("Light", Light_draw_function, NULL);
  if (Light_page == NULL)
  {
    return NULL;
  }

  menu_item_set_callbacks(Light_page, Light_on_enter, Light_on_exit, NULL, Light_key_handler);

  printf("Light_page created successfully\r\n");
  return Light_page;
}

/**
 * @brief 光照自定义绘制函数
 * @param context 绘制上下文
 */
void Light_draw_function(void *context)
{
  if (Light_ON)
  {
     Light_state_t *state = (Light_state_t *)context;
  if (state == NULL)
  {
    return;
  }

  // 光照数据由全局SensorData任务定期更新，无需单独读取

  Light_display_info(state);
   
  
  OLED_Refresh_Dirty();
  }
  else
  {
    OLED_Printf_Line_32(0,"No Data");
    OLED_Printf_Line_32(2,"light off");
    OLED_Refresh_Dirty();
  }
  
 
}

void Light_key_handler(menu_item_t *item, uint8_t key_event)
{
  Light_state_t *state = (Light_state_t *)item->content.custom.draw_context;
  if (state == NULL) {
    return;
  }
  
  switch (key_event)
  {
  case MENU_EVENT_KEY_UP:
    // KEY0 - 开启光照传感器
    printf("Light: KEY0 pressed\r\n");
    Light_ON=1;
    break;

  case MENU_EVENT_KEY_DOWN:
    // KEY1 - 关闭光照传感器
    printf("Light: KEY1 pressed\r\n");
    OLED_Clear();
    Light_ON=0;
    break;

  case MENU_EVENT_KEY_SELECT:
    // KEY2 - 返回上一级
    printf("Light: KEY2 pressed\r\n");
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
 * @brief 从上下文获取光照状态结构体指针
 * @param context 指向Light状态结构体的上下文指针
 * @return 返回指向Light状态结构体的指针
 */
Light_state_t *Light_get_state(void *context)
{
  return (Light_state_t *)context;
}

void Light_refresh_display(void *context)
{
  Light_state_t *state = (Light_state_t *)context;
  if (state == NULL) {
    return;
  }
  
  state->need_refresh = 1;
  state->last_update = xTaskGetTickCount();
}

void Light_on_enter(menu_item_t *item)
{
  printf("Enter Light page\r\n");
  
  // 使用静态分配的状态数据，直接初始化
  Light_init_sensor_data(&g_light_state);
  
  // 传感器数据已在全局SensorData任务中初始化和读取
  
  // 设置静态状态到菜单项上下文
  item->content.custom.draw_context = &g_light_state;
  
  // 清屏并标记需要刷新
  OLED_Clear();
  g_light_state.need_refresh = 1;
}

void Light_on_exit(menu_item_t *item)
{
  printf("Exit Light page\r\n");
  
  Light_state_t *state = (Light_state_t *)item->content.custom.draw_context;
  if (state == NULL) {
    return;
  }
  
  // 清理传感器相关数据
  Light_cleanup_sensor_data(state);
  
  printf("Light state cleaned up (no memory free needed)\r\n");
  
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
static void Light_init_sensor_data(Light_state_t *state)
{
    if (state == NULL) {
        return;
    }
    
    // 清零状态结构体
    memset(state, 0, sizeof(Light_state_t));
    
    // 初始化状态
    state->need_refresh = 1;
    state->last_update = xTaskGetTickCount();
    state->last_date_L = 0;
    state->result = 1;
    
    printf("Light state initialized\r\n");
}

/**
 * @brief 清理传感器数据
 * @param state 传感器状态指针
 */
static void Light_cleanup_sensor_data(Light_state_t *state)
{
    if (state == NULL) {
        return;
    }
    
    printf("Light sensor data cleaned up\r\n");
}

static void Light_display_info(void *context)
{
  Light_state_t *state = (Light_state_t *)context;
  if (state == NULL) {
    return;
  }
  
  OLED_Clear_Line(0);
  OLED_Printf_Line(0, "Light: %d lux", SensorData.light_data.lux);
  
  // 光照等级描述
  const char* light_desc = "";
  if (SensorData.light_data.lux < 50) {
    light_desc = "Dark";
  } else if (SensorData.light_data.lux < 200) {
    light_desc = "Dim";
  } else if (SensorData.light_data.lux < 500) {
    light_desc = "Normal";
  } else if (SensorData.light_data.lux < 2000) {
    light_desc = "Bright";
  } else {
    light_desc = "Very Bright";
  }
  
  OLED_Clear_Line(2);
  OLED_Printf_Line(2, "Level: %s", light_desc);
  
  // 渐进式更新光照显示值，避免突变
  if (SensorData.light_data.lux > state->last_date_L)
  {
    if (SensorData.light_data.lux - state->last_date_L >= 100)
    {
      state->last_date_L += 20;
    }
    else
    {
      state->last_date_L++;
    }
  }
  else if (SensorData.light_data.lux < state->last_date_L)
  {
    if (state->last_date_L - SensorData.light_data.lux >= 100)
    {
      state->last_date_L -= 20;
    }
    else
    {
      state->last_date_L--;
    }
  }
  
  // 确保值不为负
  if (state->last_date_L < 0) {
    state->last_date_L = 0;
  }
  
  // 绘制光照进度条
  OLED_DrawLightBar_Line1(state->last_date_L);
}
