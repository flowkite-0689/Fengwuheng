#include "ParamSetting.h"

// 声明外部变量
extern uint16_t publish_delaytime;
extern uint16_t Sensordata_delaytime;

// 定义静态状态变量，避免动态内存分配
ParamSetting_state_t g_paramsetting_state = {0};

// ==================================
// 静态函数声明
// ==================================
static void ParamSetting_init_data(ParamSetting_state_t *state);
static void ParamSetting_cleanup_data(ParamSetting_state_t *state);
static void ParamSetting_display_info(void *context);

// 发布间隔进度条（line=1）
void OLED_DrawPublishBar_Line1(uint16_t delay_value)
{
  OLED_Clear_Line(1);
  // 标签
  OLED_ShowString(0, 16, (uint8_t *)"5", 12, 1);
  OLED_ShowString(110, 16, (uint8_t *)"60", 12, 1);
  // 进度条：x=20, y=18, w=88, h=8, 5~60秒
  OLED_DrawProgressBar(17, 18, 87, 8, delay_value, 5, 60, 1, 1,1);
}

// 传感器间隔进度条（line=3）
void OLED_DrawSensorBar_Line3(uint16_t delay_value)
{
  OLED_Clear_Line(3);
  OLED_ShowString(0, 48, (uint8_t *)"1", 12, 1);
  OLED_ShowString(110, 48, (uint8_t *)"10", 12, 1);
  OLED_DrawProgressBar(17, 52, 87, 8, delay_value, 1, 10, 1, 1,1);
}

/**
 * @brief 初始化参数设置页面
 * @return 创建的参数设置菜单项指针
 */
menu_item_t *ParamSetting_init(void)
{
  // 创建自定义菜单项，不分配具体状态数据
  menu_item_t *ParamSetting_page = MENU_ITEM_CUSTOM("ParamSetting", ParamSetting_draw_function, NULL);
  if (ParamSetting_page == NULL)
  {
    return NULL;
  }

  menu_item_set_callbacks(ParamSetting_page, ParamSetting_on_enter, ParamSetting_on_exit, NULL, ParamSetting_key_handler);

  printf("ParamSetting_page created successfully\r\n");
  return ParamSetting_page;
}

/**
 * @brief 参数设置自定义绘制函数
 * @param context 绘制上下文
 */
void ParamSetting_draw_function(void *context)
{
  ParamSetting_state_t *state = (ParamSetting_state_t *)context;
  if (state == NULL)
  {
    return;
  }

  ParamSetting_display_info(state);
  
  OLED_Refresh_Dirty();
}

void ParamSetting_key_handler(menu_item_t *item, uint8_t key_event)
{
  ParamSetting_state_t *state = (ParamSetting_state_t *)item->content.custom.draw_context;
  if (state == NULL) {
    return;
  }
  
  switch (key_event)
  {
  case MENU_EVENT_KEY_UP:
    // KEY0 - 增加当前参数值
    if (state->selected_item == 0) {
      // 增加发布间隔
      
      if (state->current_publish_delay < 60) {
        taskENTER_CRITICAL();
        state->current_publish_delay++;
        publish_delaytime = state->current_publish_delay;
         taskEXIT_CRITICAL();
        printf("Publish delay increased to %d seconds\r\n", state->current_publish_delay);
       
      }

    } else {
      // 增加传感器间隔
      if (state->current_sensor_delay < 10) {
         taskENTER_CRITICAL();
        state->current_sensor_delay++;
        Sensordata_delaytime = state->current_sensor_delay * 1000;
        taskEXIT_CRITICAL();
        printf("Sensor delay increased to %d seconds\r\n", state->current_sensor_delay);
      }
    }
    break;

  case MENU_EVENT_KEY_DOWN:
    // KEY1 - 减少当前参数值
    if (state->selected_item == 0) {
      // 减少发布间隔
      if (state->current_publish_delay > 5) {
         taskENTER_CRITICAL();
        state->current_publish_delay--;
        publish_delaytime = state->current_publish_delay;
         taskEXIT_CRITICAL();
        printf("Publish delay decreased to %d seconds\r\n", state->current_publish_delay);
      }
    } else {
      // 减少传感器间隔
      if (state->current_sensor_delay > 1) {
          taskENTER_CRITICAL();
        state->current_sensor_delay--;
        Sensordata_delaytime = state->current_sensor_delay * 1000;
         taskEXIT_CRITICAL();
        printf("Sensor delay decreased to %d seconds\r\n", state->current_sensor_delay);
      }
    }
    break;

  case MENU_EVENT_KEY_SELECT:
    // KEY2 - 确认并返回父菜单
    menu_back_to_parent();
    break;

  case MENU_EVENT_KEY_ENTER:
    // KEY3 - 切换选中的参数
    state->selected_item = (state->selected_item == 0) ? 1 : 0;
    printf("ParamSetting: Selected item %d\r\n", state->selected_item);
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
 * @brief 从上下文获取参数设置状态结构体指针
 * @param context 指向ParamSetting状态结构体的上下文指针
 * @return 返回指向ParamSetting状态结构体的指针
 */
ParamSetting_state_t *ParamSetting_get_state(void *context)
{
  return (ParamSetting_state_t *)context;
}

void ParamSetting_refresh_display(void *context)
{
  ParamSetting_state_t *state = (ParamSetting_state_t *)context;
  if (state == NULL) {
    return;
  }
  
  state->need_refresh = 1;
  state->last_update = xTaskGetTickCount();
}

void ParamSetting_on_enter(menu_item_t *item)
{
  printf("Enter ParamSetting page\r\n");
  
  // 使用静态分配的状态数据，直接初始化
  ParamSetting_init_data(&g_paramsetting_state);
  
  // 设置静态状态到菜单项上下文
  item->content.custom.draw_context = &g_paramsetting_state;
  
  // 清屏并标记需要刷新
  OLED_Clear();
  g_paramsetting_state.need_refresh = 1;
}

void ParamSetting_on_exit(menu_item_t *item)
{
  printf("Exit ParamSetting page\r\n");
  
  ParamSetting_state_t *state = (ParamSetting_state_t *)item->content.custom.draw_context;
  if (state == NULL) {
    return;
  }
  
  // 清理数据
  ParamSetting_cleanup_data(state);
  
  printf("ParamSetting state cleaned up\r\n");
  
  // 清空指针，防止野指针
  item->content.custom.draw_context = NULL;
  
  // 清屏
  OLED_Clear();
}

// ==================================
// 数据初始化与清理
// ==================================

/**
 * @brief 初始化参数设置数据
 * @param state 参数设置状态指针
 */
static void ParamSetting_init_data(ParamSetting_state_t *state)
{
    if (state == NULL) {
        return;
    }
    
    // 清零状态结构体
    memset(state, 0, sizeof(ParamSetting_state_t));
    
    // 初始化状态
    state->need_refresh = 1;
    state->last_update = xTaskGetTickCount();
    state->selected_item = 0;
    
    // 从外部变量获取当前值
    state->current_publish_delay = publish_delaytime;
    state->current_sensor_delay = Sensordata_delaytime / 1000; // 转换为秒
    
    printf("ParamSetting state initialized\r\n");
    printf("Current publish delay: %d seconds\r\n", state->current_publish_delay);
    printf("Current sensor delay: %d seconds\r\n", state->current_sensor_delay);
}

/**
 * @brief 清理数据
 * @param state 参数设置状态指针
 */
static void ParamSetting_cleanup_data(ParamSetting_state_t *state)
{
    if (state == NULL) {
        return;
    }
    
    printf("ParamSetting data cleaned up\r\n");
}

static void ParamSetting_display_info(void *context)
{
  ParamSetting_state_t *state = (ParamSetting_state_t *)context;
  if (state == NULL) {
    return;
  }
  
  // 根据当前选中的项目显示不同的内容
  switch (state->selected_item)
  {
  case 0: // 设置发布间隔
    OLED_Clear_Line(1);
    OLED_Printf_Line(0, "[%2d]s/%2ds", state->current_publish_delay, state->current_sensor_delay);
    OLED_Clear_Line(2);
    OLED_Printf_Line(2, "  Set Publish Delay");
    break;
  case 1: // 设置传感器间隔
    OLED_Clear_Line(1);
    OLED_Printf_Line(0, "%2ds/[%2d]s", state->current_publish_delay, state->current_sensor_delay);
    OLED_Clear_Line(2);
    OLED_Printf_Line(2, "  Set Sensor Delay");
    break;
  }
  
  // 显示操作提示
  OLED_Printf_Line(3, "KEY0:+ KEY1:- KEY2:OK");
  OLED_Printf_Line(4, "KEY3:Switch");
  
  // 显示进度条
  OLED_DrawPublishBar_Line1(state->current_publish_delay);
  OLED_DrawSensorBar_Line3(state->current_sensor_delay);
}
