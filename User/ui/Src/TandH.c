#include "TandH.h"

// ==================================
// 本页面变量定义
// ==================================
static TandH_state_t s_TandH_state = {0};

// ==================================
// 静态函数声明
// ==================================
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

static void TandH_display_info(void);

/**
 * @brief 初始化温湿度页面
 * @return 创建的温湿菜单项指针
 */
menu_item_t *TandH_init(void)
{
  //
  memset(&s_TandH_state, 0, sizeof(s_TandH_state));
  s_TandH_state.need_refresh = 1;
  s_TandH_state.last_update = xTaskGetTickCount();
  s_TandH_state.last_date_H = 0;
  s_TandH_state.temp_int = 50;
  s_TandH_state.humi_int = 100;

  s_TandH_state.result = 1;

  DHT11_Init();

  menu_item_t *TandH_page = MENU_ITEM_CUSTOM("Temp&Humid", TandH_draw_function, &s_TandH_state);
  if (TandH_page == NULL)
  {
    return NULL;
  }

  menu_item_set_callbacks(TandH_page, TandH_on_enter, TandH_on_exit, NULL, TandH_key_handler);

  printf("TandH_page initialized successfully\r\n");
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

  TandH_update_dht11();

  TandH_display_info();

  OLED_Refresh_Dirty();
}

void TandH_key_handler(menu_item_t *item, uint8_t key_event)
{
  TandH_state_t *state = (TandH_state_t *)item->content.custom.draw_context;
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

void TandH_update_dht11(void)
{
  DHT11_Data_TypeDef dhtdata;

  s_TandH_state.result = Read_DHT11(&dhtdata);

  s_TandH_state.temp_int = dhtdata.temp_int;
  s_TandH_state.temp_deci = dhtdata.temp_deci;
  s_TandH_state.humi_int = dhtdata.humi_int;
  s_TandH_state.humi_deci = dhtdata.humi_deci;
}

TandH_state_t *TandH_get_state(void)
{
  return &s_TandH_state;
}

void TandH_refresh_display(void)
{
  s_TandH_state.need_refresh = 1;
  s_TandH_state.last_update = xTaskGetTickCount();
}

void TandH_on_enter(menu_item_t *item)
{
  printf("Enter TandH page\r\n");
  OLED_Clear();
  s_TandH_state.need_refresh = 1;
}

void TandH_on_exit(menu_item_t *item)
{
  printf("Exit TandH page\r\n");
    OLED_Clear();
}

static void TandH_display_info(void)
{
  if (s_TandH_state.result == 0)
    {
      OLED_Clear_Line(3);
      OLED_Printf_Line(0, "Temperature:%d.%dC ",
                       s_TandH_state.temp_int, s_TandH_state.temp_deci);
      OLED_Printf_Line(2, "Humidity:  %d.%d%%",
                       s_TandH_state.humi_int, s_TandH_state.humi_deci);
                       // 横向温度计（支持小数：25.5°C → 255）
    
// printf("Humi_int: %d, Humi_deci: %d\n", s_TandH_state.humi_int, s_TandH_state.humi_deci);
//                        printf("temp_int: %d, temp_deci: %d\n", s_TandH_state.temp_int, s_TandH_state.temp_deci);
    
    }
    else
    {
     
      // OLED_Clear_Line(2);
      // OLED_Printf_Line(2, "DHT11 Error!    ");
      // OLED_Printf_Line(3, "Code: %d        ", result);
    }
    int16_t temp_tenth = s_TandH_state.temp_int * 10 + s_TandH_state.temp_deci;
    if (temp_tenth >s_TandH_state.last_date_T)
    {
      
     if (temp_tenth-s_TandH_state.last_date_T>=30)
     {
        s_TandH_state.last_date_T+=70;
     }
     
        s_TandH_state.last_date_T++;
      
    }else if (temp_tenth  < s_TandH_state.last_date_T)
    {
      
        s_TandH_state.last_date_T-=17;
    
    }
      OLED_DrawTempBar_Line1(s_TandH_state.last_date_T);

    if (s_TandH_state.humi_int>s_TandH_state.last_date_H )
    {
      if (s_TandH_state.humi_int-s_TandH_state.last_date_H>=10)
      {
        s_TandH_state.last_date_H+=4;
      }
      
      s_TandH_state.last_date_H++;
    }else if (s_TandH_state.humi_int < s_TandH_state.last_date_H  )
    {
      s_TandH_state.last_date_H-=3;
    }
    

    // 横向湿度条
    OLED_DrawHumidityBar_Line3(s_TandH_state.last_date_H);
}
