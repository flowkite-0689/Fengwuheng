#include "TandH.h"
#include "esp8266.h"

// 定义静态状态变量，避免动态内存分配
TandH_state_t g_tandh_state = {0};
// ==================================
// 静态函数声明
// ==================================
static void TandH_init_sensor_data(TandH_state_t *state);
static void TandH_cleanup_sensor_data(TandH_state_t *state);
static void TandH_display_info(void *context);
// 温度进度条（line=1）
void OLED_DrawTempBar_Line1(int16_t temp_tenth) // 0.1°C
{
  OLED_Clear_Line(1);
  // 标签
  OLED_ShowString(0, 16, (uint8_t *)"0", 12, 1);
  OLED_ShowString(110, 16, (uint8_t *)"50", 12, 1);
  // 进度条：x=20, y=18, w=88, h=8, 0~500 (0.0~50.0°C)
  OLED_DrawProgressBar(17, 18, 87, 8, temp_tenth, 0, 500, 1, 1,1);
}

// 湿度进度条（line=3）
void OLED_DrawHumidityBar_Line3(uint8_t humi)
{
  OLED_Clear_Line(3);
  OLED_ShowString(0, 48, (uint8_t *)"0", 12, 1);
  OLED_ShowString(110, 48, (uint8_t *)"100", 12, 1);
  OLED_DrawProgressBar(17, 52, 87, 8, humi, 0, 100, 1, 1,1);
}

/**
 * @brief 初始化温湿度页面
 * @return 创建的温湿菜单项指针
 */
menu_item_t *TandH_init(void)
{
  // 创建自定义菜单项，不分配具体状态数据
  menu_item_t *TandH_page = MENU_ITEM_CUSTOM("Temp&Humid", TandH_draw_function, NULL);
  if (TandH_page == NULL)
  {
    return NULL;
  }

  menu_item_set_callbacks(TandH_page, TandH_on_enter, TandH_on_exit, NULL, TandH_key_handler);

  printf("TandH_page created successfully\r\n");
  return TandH_page;
}

/**
 * @brief 温湿度自定义绘制函数
 * @param context 绘制上下文
 */
void TandH_draw_function(void *context)
{

  if (DHT11_ON)
  {
     TandH_state_t *state = (TandH_state_t *)context;
  if (state == NULL)
  {
    return;
  }

  // 温湿度数据由全局SensorData任务定期更新，无需单独读取

  TandH_display_info(state);
   
  
  OLED_Refresh_Dirty();
  }
  else
  {
    OLED_Printf_Line_32(0,"No Data");
    OLED_Printf_Line_32(2,"dht11 off");
    OLED_Refresh_Dirty();
  }
  
 
}

void TandH_key_handler(menu_item_t *item, uint8_t key_event)
{
  TandH_state_t *state = (TandH_state_t *)item->content.custom.draw_context;
  if (state == NULL) {
    return;
  }
  
  switch (key_event)
  {
  case MENU_EVENT_KEY_UP:
    // KEY0 - 可以用来切换某些状态或进入特定功能
    printf("Index: KEY0 pressed\r\n");
    DHT11_ON=1;
    break;

  case MENU_EVENT_KEY_DOWN:
    // KEY1 - 可以用来切换某些状态或进入特定功能
    printf("Index: KEY1 pressed\r\n");
    OLED_Clear();
    DHT11_ON=0;
    break;

  case MENU_EVENT_KEY_SELECT:
    // KEY2 - 可以返回上一级或特定功能
    printf("Index: KEY2 pressed\r\n");
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
 * @brief 从上下文获取温湿度状态结构体指针
 * @param context 指向TandH状态结构体的上下文指针
 * @return 返回指向TandH状态结构体的指针
 */
TandH_state_t *TandH_get_state(void *context)
{
  return (TandH_state_t *)context;
}

void TandH_refresh_display(void *context)
{
  TandH_state_t *state = (TandH_state_t *)context;
  if (state == NULL) {
    return;
  }
  
  state->need_refresh = 1;
  state->last_update = xTaskGetTickCount();
}

void TandH_on_enter(menu_item_t *item)
{
  printf("Enter TandH page\r\n");
  
  // 使用静态分配的状态数据，直接初始化
  TandH_init_sensor_data(&g_tandh_state);
  
  // 传感器数据已在全局SensorData任务中初始化和读取
  
  // 设置静态状态到菜单项上下文
  item->content.custom.draw_context = &g_tandh_state;
  
  // 清屏并标记需要刷新
  OLED_Clear();
  g_tandh_state.need_refresh = 1;
}

void TandH_on_exit(menu_item_t *item)
{
  printf("Exit TandH page\r\n");
  
  TandH_state_t *state = (TandH_state_t *)item->content.custom.draw_context;
  if (state == NULL) {
    return;
  }
  
  // 清理传感器相关数据
  TandH_cleanup_sensor_data(state);
  
  printf("TandH state cleaned up (no memory free needed)\r\n");
  
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
static void TandH_init_sensor_data(TandH_state_t *state)
{
    if (state == NULL) {
        return;
    }
    
    // 清零状态结构体
    memset(state, 0, sizeof(TandH_state_t));
    
    // 初始化状态
    state->need_refresh = 1;
    state->last_update = xTaskGetTickCount();
    state->last_date_H = 0;
    state->result = 1;
    
    printf("TandH state initialized\r\n");
}

/**
 * @brief 清理传感器数据
 * @param state 传感器状态指针
 */
static void TandH_cleanup_sensor_data(TandH_state_t *state)
{
    if (state == NULL) {
        return;
    }
    
    printf("TandH sensor data cleaned up\r\n");
}


static void TandH_display_info(void *context)
{
  TandH_state_t *state = (TandH_state_t *)context;
  if (state == NULL) {
    return;
  }
  
 
      OLED_Clear_Line(3);
      OLED_Printf_Line(0, "Temperature:%d.%dC ",
                       SensorData.dht11_data.temp_int,  SensorData.dht11_data.temp_deci);
      OLED_Printf_Line(2, "Humidity:  %d.%d%%",
                        SensorData.dht11_data.humi_int, SensorData.dht11_data.humi_deci);
                       // 横向温度计（支持小数：25.5°C → 255）
    

    int16_t temp_tenth =  SensorData.dht11_data.temp_int * 10 +  SensorData.dht11_data.temp_deci;
    if (temp_tenth >state->last_date_T)
    {
      
     if (temp_tenth-state->last_date_T>=30)
     {
        state->last_date_T+=10;
     }
     
        state->last_date_T++;
      
    }else if (temp_tenth  < state->last_date_T)
    {
      
        state->last_date_T--;
    
    }
      OLED_DrawTempBar_Line1(state->last_date_T);

    if ( SensorData.dht11_data.humi_int>state->last_date_H )
    {
      if ( SensorData.dht11_data.humi_int-state->last_date_H>=10)
      {
        state->last_date_H+=4;
      }
      
      state->last_date_H++;
    }else if ( SensorData.dht11_data.humi_int < state->last_date_H  )
    {
      state->last_date_H-=3;
    }
    

    // 横向湿度条
    OLED_DrawHumidityBar_Line3(state->last_date_H);
    
     

}
