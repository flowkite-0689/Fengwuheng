#include "TandH.h"
#include <stdlib.h>  // 添加stdlib.h用于动态内存管理
#include "esp8266.h"
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
  TandH_state_t *state = (TandH_state_t *)context;
  if (state == NULL)
  {
    return;
  }

  TandH_update_dht11(state);

  TandH_display_info(state);
   
  
  OLED_Refresh_Dirty();
  vTaskDelay(1000);
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
    break;

  case MENU_EVENT_KEY_DOWN:
    // KEY1 - 可以用来切换某些状态或进入特定功能
    printf("Index: KEY1 pressed\r\n");
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

void TandH_update_dht11(void *context)
{
  TandH_state_t *state = (TandH_state_t *)context;
  if (state == NULL) {
    return;
  }
  
  DHT11_Data_TypeDef dhtdata;

  state->result = Read_DHT11(&dhtdata);

  state->temp_int = dhtdata.temp_int;
  state->temp_deci = dhtdata.temp_deci;
  state->humi_int = dhtdata.humi_int;
  state->humi_deci = dhtdata.humi_deci;
}

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
  
  // 分配状态数据结构
  TandH_state_t *state = (TandH_state_t *)pvPortMalloc(sizeof(TandH_state_t));
  if (state == NULL) {
    printf("Error: Failed to allocate TandH state memory!\r\n");
    return;
  }
  
  printf("MALLOC: TandH_on_enter, state addr=%p, size=%d bytes\r\n", 
         state, sizeof(TandH_state_t));
  
  // 初始化状态数据
  TandH_init_sensor_data(state);
  
  // 初始化DHT11传感器
  DHT11_Init();
  
  // 设置到菜单项上下文
  item->content.custom.draw_context = state;
  
  // 清屏并标记需要刷新
  OLED_Clear();
  state->need_refresh = 1;
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
  
  // 释放状态结构体本身
  printf("FREE: TandH_on_exit, state addr=%p, size=%d bytes\r\n", 
         state, sizeof(TandH_state_t));
  vPortFree(state);
  
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
    state->temp_int = 50;
    state->humi_int = 100;
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
    
    // 如果有需要关闭的传感器，在这里执行
    // 例如：DHT11_Deinit();
    
    printf("TandH sensor data cleaned up\r\n");
}


static void TandH_display_info(void *context)
{
  TandH_state_t *state = (TandH_state_t *)context;
  if (state == NULL) {
    return;
  }
  
  if (state->result == 0)
    {
      OLED_Clear_Line(3);
      OLED_Printf_Line(0, "Temperature:%d.%dC ",
                       state->temp_int, state->temp_deci);
      OLED_Printf_Line(2, "Humidity:  %d.%d%%",
                       state->humi_int, state->humi_deci);
                       // 横向温度计（支持小数：25.5°C → 255）
    
// printf("Humi_int: %d, Humi_deci: %d\n", state->humi_int, state->humi_deci);
//                        printf("temp_int: %d, temp_deci: %d\n", state->temp_int, state->temp_deci);
    
    }
    else
    {
     
      // OLED_Clear_Line(2);
      // OLED_Printf_Line(2, "DHT11 Error!    ");
      // OLED_Printf_Line(3, "Code: %d        ", result);
    }
    int16_t temp_tenth = state->temp_int * 10 + state->temp_deci;
    if (temp_tenth >state->last_date_T)
    {
      
     if (temp_tenth-state->last_date_T>=30)
     {
        state->last_date_T+=70;
     }
     
        state->last_date_T++;
      
    }else if (temp_tenth  < state->last_date_T)
    {
      
        state->last_date_T-=17;
    
    }
      OLED_DrawTempBar_Line1(state->last_date_T);

    if (state->humi_int>state->last_date_H )
    {
      if (state->humi_int-state->last_date_H>=10)
      {
        state->last_date_H+=4;
      }
      
      state->last_date_H++;
    }else if (state->humi_int < state->last_date_H  )
    {
      state->last_date_H-=3;
    }
    

    // 横向湿度条
    OLED_DrawHumidityBar_Line3(state->last_date_H);
    //发布数据到巴法云
     

}
