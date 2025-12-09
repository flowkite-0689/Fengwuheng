#include "Stopwatch.h"

// ==================================
// 本页面变量定义
// ==================================
static Stopwatch_state_t s_Stopwatch_state = {0};

// ==================================
// 静态函数声明
// ==================================
static void Stopwatch_display_info(void);

/**
 * @brief 初始化秒表页面
 * @return 创建的秒表菜单项指针
 */
menu_item_t *Stopwatch_init(void)
{
  memset(&s_Stopwatch_state, 0, sizeof(s_Stopwatch_state));
  s_Stopwatch_state.need_refresh = 1;
  s_Stopwatch_state.last_update = xTaskGetTickCount();

  menu_item_t *Stopwatch_page = MENU_ITEM_CUSTOM("Stopwatch", Stopwatch_draw_function, &s_Stopwatch_state);
  if (Stopwatch_page == NULL)
  {
    return NULL;
  }

  menu_item_set_callbacks(Stopwatch_page, Stopwatch_on_enter, Stopwatch_on_exit, NULL, Stopwatch_key_handler);

  printf("Stopwatch_page initialized successfully\r\n");
  return Stopwatch_page;
}

/**
 * @brief 秒表自定义绘制函数
 * @param context 绘制上下文
 */
void Stopwatch_draw_function(void *context)
{
  Stopwatch_state_t *state = (Stopwatch_state_t *)context;
  if (state == NULL)
  {
    return;
  }

  // 如果正在运行，计算实时时间
  if (state->running)
  {
    uint32_t current_time = xTaskGetTickCount();
    state->elapsed_time = (current_time - state->start_time) + state->pause_time;
  }

  Stopwatch_display_info();

  OLED_Refresh_Dirty();
}

/**
 * @brief 秒表按键处理函数
 * @param item 菜单项
 * @param key_event 按键事件
 */
void Stopwatch_key_handler(menu_item_t *item, uint8_t key_event)
{
  Stopwatch_state_t *state = (Stopwatch_state_t *)item->content.custom.draw_context;
  switch (key_event)
  {
  case MENU_EVENT_KEY_UP:
    // KEY0 - 启动/继续
    if (!state->running)
    {
      state->running = 1;
      state->start_time = xTaskGetTickCount();
      printf("Stopwatch started\r\n");
    }
    break;

  case MENU_EVENT_KEY_DOWN:
    // KEY1 - 暂停
    if (state->running)
    {
      state->running = 0;
      state->pause_time += (xTaskGetTickCount() - state->start_time);
      printf("Stopwatch paused\r\n");
    }
    break;

  case MENU_EVENT_KEY_SELECT:
    // KEY2 - 退出
    printf("Exiting stopwatch\r\n");
    menu_back_to_parent();
    break;

  case MENU_EVENT_KEY_ENTER:
    // KEY3 - 重置
    state->running = 0;
    state->start_time = 0;
    state->pause_time = 0;
    state->elapsed_time = 0;
    printf("Stopwatch reset\r\n");
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
 * @brief 获取秒表状态
 * @return 秒表状态指针
 */
Stopwatch_state_t *Stopwatch_get_state(void)
{
  return &s_Stopwatch_state;
}

/**
 * @brief 刷新秒表显示
 */
void Stopwatch_refresh_display(void)
{
  s_Stopwatch_state.need_refresh = 1;
  s_Stopwatch_state.last_update = xTaskGetTickCount();
}

/**
 * @brief 进入秒表页面时的回调
 * @param item 菜单项
 */
void Stopwatch_on_enter(menu_item_t *item)
{
  printf("Enter Stopwatch page\r\n");
  OLED_Clear();
  
  // 初始化秒表状态
  s_Stopwatch_state.running = 0;
  s_Stopwatch_state.start_time = 0;
  s_Stopwatch_state.pause_time = 0;
  s_Stopwatch_state.elapsed_time = 0;
  
  s_Stopwatch_state.need_refresh = 1;
}

/**
 * @brief 退出秒表页面时的回调
 * @param item 菜单项
 */
void Stopwatch_on_exit(menu_item_t *item)
{
  printf("Exit Stopwatch page\r\n");
  OLED_Clear();
}

/**
 * @brief 显示秒表界面信息
 */
static void Stopwatch_display_info(void)
{
  uint32_t total_seconds = s_Stopwatch_state.elapsed_time / 1000;
  uint32_t minutes = (total_seconds / 60) % 60;
  uint32_t seconds = total_seconds % 60;
  uint32_t milliseconds = s_Stopwatch_state.elapsed_time % 1000 / 10;
  
  OLED_Printf_Line_32(0, " %02lu:%02lu:%02lu", minutes, seconds, milliseconds);
  
  if(s_Stopwatch_state.running) {
    OLED_Printf_Line(2, "    RUNNING");
    OLED_Printf_Line(3, "KEY1:Pause KEY2:Exit");
  } else {
    OLED_Printf_Line(2, "    PAUSED");
    OLED_Printf_Line(3, "KEY0:Start KEY3:Reset");
  }
}
