#include "SetDate.h"

// ==================================
// 本页面变量定义
// ==================================
static SetDate_state_t s_SetDate_state = {0};

// ==================================
// 静态函数声明
// ==================================
static void SetDate_display_info(void);

/**
 * @brief 获取月份的最大天数
 * @param year 年份（两位数）
 * @param month 月份（1-12）
 * @return 该月的最大天数
 */
uint8_t get_max_days_in_month(u8 year, u8 month)
{
    if (month == 4 || month == 6 || month == 9 || month == 11)
    {
        return 30;
    }
    else if (month == 2)
    {
        // 判断闰年：能被4整除但不能被100整除，或者能被400整除
        u16 full_year = year;
        if ((full_year % 4 == 0 && full_year % 100 != 0) || full_year % 400 == 0)
        {
            return 29;
        }
        else
        {
            return 28;
        }
    }
    else
    {
        return 31;
    }
}

/**
 * @brief 初始化日期设置页面
 * @return 创建的日期设置菜单项指针
 */
menu_item_t *SetDate_init(void)
{
  memset(&s_SetDate_state, 0, sizeof(s_SetDate_state));
  s_SetDate_state.need_refresh = 1;
  s_SetDate_state.last_update = xTaskGetTickCount();
  s_SetDate_state.set_step = 0;

  menu_item_t *SetDate_page = MENU_ITEM_CUSTOM("Set Date", SetDate_draw_function, &s_SetDate_state);
  if (SetDate_page == NULL)
  {
    return NULL;
  }

  menu_item_set_callbacks(SetDate_page, SetDate_on_enter, SetDate_on_exit, NULL, SetDate_key_handler);

  printf("SetDate_page initialized successfully\r\n");
  return SetDate_page;
}

/**
 * @brief 日期设置自定义绘制函数
 * @param context 绘制上下文
 */
void SetDate_draw_function(void *context)
{
  SetDate_state_t *state = (SetDate_state_t *)context;
  if (state == NULL)
  {
    return;
  }

  SetDate_display_info();

  OLED_Refresh_Dirty();
}

/**
 * @brief 日期设置按键处理函数
 * @param item 菜单项
 * @param key_event 按键事件
 */
void SetDate_key_handler(menu_item_t *item, uint8_t key_event)
{
  SetDate_state_t *state = (SetDate_state_t *)item->content.custom.draw_context;
  switch (key_event)
  {
  case MENU_EVENT_KEY_UP:
    // KEY0 - 增加当前设置项的值
    switch (state->set_step)
    {
    case 0: // 年
      state->temp_year = (state->temp_year + 1) ;
      // 如果当前日期在新年份的2月29日后，且新年份不是闰年，调整日期
      if (state->temp_month == 2 && state->temp_day == 29)
      {
        uint8_t max_days = get_max_days_in_month(state->temp_year, state->temp_month);
        if (state->temp_day > max_days)
        {
          state->temp_day = max_days;
        }
      }
      break;
    case 1: // 月
      state->temp_month++;
      if (state->temp_month > 12)
        state->temp_month = 1;
      // 调整日期，确保不超过新月份的最大天数
      {
        uint8_t max_days = get_max_days_in_month(state->temp_year, state->temp_month);
        if (state->temp_day > max_days)
        {
          state->temp_day = max_days;
        }
      }
      break;
    case 2: // 日
    {
      uint8_t max_days = get_max_days_in_month(state->temp_year, state->temp_month);
      state->temp_day++;
      if (state->temp_day > max_days)
        state->temp_day = 1;
    }
    break;
    }
    break;

  case MENU_EVENT_KEY_DOWN:
    // KEY1 - 减少当前设置项的值
    switch (state->set_step)
    {
    case 0: // 年
      state->temp_year = (state->temp_year == 2000) ? 2099 : state->temp_year - 1;
      // 如果当前日期在新年份的2月29日后，且新年份不是闰年，调整日期
      if (state->temp_month == 2 && state->temp_day == 29)
      {
        uint8_t max_days = get_max_days_in_month(state->temp_year, state->temp_month);
        if (state->temp_day > max_days)
        {
          state->temp_day = max_days;
        }
      }
      break;
    case 1: // 月
      state->temp_month--;
      if (state->temp_month == 0)
        state->temp_month = 12;
      // 调整日期，确保不超过新月份的最大天数
      {
        uint8_t max_days = get_max_days_in_month(state->temp_year, state->temp_month);
        if (state->temp_day > max_days)
        {
          state->temp_day = max_days;
        }
      }
      break;
    case 2: // 日
    {
      uint8_t max_days = get_max_days_in_month(state->temp_year, state->temp_month);
      state->temp_day--;
      if (state->temp_day == 0)
        state->temp_day = max_days;
    }
    break;
    }
    break;

  case MENU_EVENT_KEY_SELECT:
    // KEY2 - 确认保存并返回
    RTC_SetDate_Manual(s_SetDate_state.temp_year, s_SetDate_state.temp_month, s_SetDate_state.temp_day);
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
 * @brief 获取日期设置状态
 * @return 日期设置状态指针
 */
SetDate_state_t *SetDate_get_state(void)
{
  return &s_SetDate_state;
}

/**
 * @brief 刷新日期设置显示
 */
void SetDate_refresh_display(void)
{
  s_SetDate_state.need_refresh = 1;
  s_SetDate_state.last_update = xTaskGetTickCount();
}

/**
 * @brief 进入日期设置页面时的回调
 * @param item 菜单项
 */
void SetDate_on_enter(menu_item_t *item)
{
  printf("Enter SetDate page\r\n");
  OLED_Clear();
  
  // 获取当前RTC日期
  MyRTC_ReadTime();
  printf("year:%d",RTC_data.year);
  s_SetDate_state.temp_year = RTC_data.year;
  s_SetDate_state.temp_month = RTC_data.mon;
  s_SetDate_state.temp_day = RTC_data.day;
  s_SetDate_state.set_step = 0;
  
  s_SetDate_state.need_refresh = 1;
}

/**
 * @brief 退出日期设置页面时的回调
 * @param item 菜单项
 */
void SetDate_on_exit(menu_item_t *item)
{
  printf("Exit SetDate page\r\n");
  OLED_Clear();
}

/**
 * @brief 显示日期设置界面信息
 */
static void SetDate_display_info(void)
{
  // 根据设置步骤高亮显示当前设置项
  switch (s_SetDate_state.set_step)
  {
  case 0: // 设置年
    OLED_Clear_Line(1);
    OLED_Printf_Line_32(0, "[%02d]/%02d/%02d", s_SetDate_state.temp_year-2000, s_SetDate_state.temp_month, s_SetDate_state.temp_day);
    OLED_Clear_Line(2);
    OLED_Printf_Line(2, "     Set Year");
    break;
  case 1: // 设置月
    OLED_Clear_Line(1);
    OLED_Printf_Line_32(0, "%02d/[%02d]/%02d", s_SetDate_state.temp_year-2000, s_SetDate_state.temp_month, s_SetDate_state.temp_day);
    OLED_Clear_Line(2);
    OLED_Printf_Line(2, "    Set Month");
    break;
  case 2: // 设置日
    OLED_Clear_Line(1);
    OLED_Printf_Line_32(0, "%02d/%02d/[%02d]", s_SetDate_state.temp_year-2000, s_SetDate_state.temp_month, s_SetDate_state.temp_day);
    OLED_Clear_Line(2);
    OLED_Printf_Line(2, "      Set Day");
    break;
  }

  OLED_Printf_Line(3, "KEY0:+ KEY1:- KEY2:OK");
}
