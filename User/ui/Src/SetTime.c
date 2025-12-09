#include "SetTime.h"

// ==================================
// 本页面变量定义
// ==================================
static SetTime_state_t s_SetTime_state = {0};

// ==================================
// 静态函数声明
// ==================================
static void SetTime_display_info(void);

/**
 * @brief 初始化时间设置页面
 * @return 创建的时间设置菜单项指针
 */
menu_item_t *SetTime_init(void)
{
  memset(&s_SetTime_state, 0, sizeof(s_SetTime_state));
  s_SetTime_state.need_refresh = 1;
  s_SetTime_state.last_update = xTaskGetTickCount();
  s_SetTime_state.set_step = 0;

  menu_item_t *SetTime_page = MENU_ITEM_CUSTOM("Set Time", SetTime_draw_function, &s_SetTime_state);
  if (SetTime_page == NULL)
  {
    return NULL;
  }

  menu_item_set_callbacks(SetTime_page, SetTime_on_enter, SetTime_on_exit, NULL, SetTime_key_handler);

  printf("SetTime_page initialized successfully\r\n");
  return SetTime_page;
}

/**
 * @brief 时间设置自定义绘制函数
 * @param context 绘制上下文
 */
void SetTime_draw_function(void *context)
{
  SetTime_state_t *state = (SetTime_state_t *)context;
  if (state == NULL)
  {
    return;
  }

  SetTime_display_info();

  OLED_Refresh_Dirty();
}

/**
 * @brief 时间设置按键处理函数
 * @param item 菜单项
 * @param key_event 按键事件
 */
void SetTime_key_handler(menu_item_t *item, uint8_t key_event)
{
  SetTime_state_t *state = (SetTime_state_t *)item->content.custom.draw_context;
  switch (key_event)
  {
  case MENU_EVENT_KEY_UP:
    // KEY0 - 增加当前设置项的值
    switch (state->set_step)
    {
    case 0: // 小时
      state->temp_hours = (state->temp_hours + 1) % 24;
      break;
    case 1: // 分钟
      state->temp_minutes = (state->temp_minutes + 1) % 60;
      break;
    case 2: // 秒
      state->temp_seconds = (state->temp_seconds + 1) % 60;
      break;
    }
    break;

  case MENU_EVENT_KEY_DOWN:
    // KEY1 - 减少当前设置项的值
    switch (state->set_step)
    {
    case 0: // 小时
      state->temp_hours = (state->temp_hours == 0) ? 23 : state->temp_hours - 1;
      break;
    case 1: // 分钟
      state->temp_minutes = (state->temp_minutes == 0) ? 59 : state->temp_minutes - 1;
      break;
    case 2: // 秒
      state->temp_seconds = (state->temp_seconds == 0) ? 59 : state->temp_seconds - 1;
      break;
    }
    break;

  case MENU_EVENT_KEY_SELECT:
    // KEY2 - 确认保存并返回
    RTC_SetTime_Manual(state->temp_hours, state->temp_minutes, state->temp_seconds);
    menu_back_to_parent();
    break;

  case MENU_EVENT_KEY_ENTER:
    // KEY3 - 切换到下一个设置项
    state->set_step++;
    if (state->set_step >= 3)
    {
      state->set_step = 0; // 回到第一项
    }
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
 * @brief 获取时间设置状态
 * @return 时间设置状态指针
 */
SetTime_state_t *SetTime_get_state(void)
{
  return &s_SetTime_state;
}

/**
 * @brief 刷新时间设置显示
 */
void SetTime_refresh_display(void)
{
  s_SetTime_state.need_refresh = 1;
  s_SetTime_state.last_update = xTaskGetTickCount();
}

/**
 * @brief 进入时间设置页面时的回调
 * @param item 菜单项
 */
void SetTime_on_enter(menu_item_t *item)
{
  printf("Enter SetTime page\r\n");
  OLED_Clear();
  
  // 获取当前RTC时间
  MyRTC_ReadTime();
  s_SetTime_state.temp_hours = RTC_data.hours;
  s_SetTime_state.temp_minutes = RTC_data.minutes;
  s_SetTime_state.temp_seconds = RTC_data.seconds;
  s_SetTime_state.set_step = 0;
  
  s_SetTime_state.need_refresh = 1;
}

/**
 * @brief 退出时间设置页面时的回调
 * @param item 菜单项
 */
void SetTime_on_exit(menu_item_t *item)
{
  printf("Exit SetTime page\r\n");
  OLED_Clear();
}

/**
 * @brief 显示时间设置界面信息
 */
static void SetTime_display_info(void)
{
  // 根据设置步骤高亮显示当前设置项
  switch (s_SetTime_state.set_step)
  {
  case 0: // 设置小时
    OLED_Clear_Line(1);
    OLED_Printf_Line_32(0, "[%02d]:%02d:%02d", s_SetTime_state.temp_hours, s_SetTime_state.temp_minutes, s_SetTime_state.temp_seconds);
    OLED_Clear_Line(2);
    OLED_Printf_Line(2, "    Set Hours");
    break;
  case 1: // 设置分钟
    OLED_Clear_Line(1);
    OLED_Printf_Line_32(0, "%02d:[%02d]:%02d", s_SetTime_state.temp_hours, s_SetTime_state.temp_minutes, s_SetTime_state.temp_seconds);
    OLED_Clear_Line(2);
    OLED_Printf_Line(2, "   Set Minutes");
    break;
  case 2: // 设置秒
    OLED_Clear_Line(1);
    OLED_Printf_Line_32(0, "%02d:%02d:[%02d]", s_SetTime_state.temp_hours, s_SetTime_state.temp_minutes, s_SetTime_state.temp_seconds);
    OLED_Clear_Line(2);
    OLED_Printf_Line(2, "   Set Seconds");
    break;
  }
  OLED_Printf_Line(3, "KEY0:+ KEY1:- KEY2:OK");
}
