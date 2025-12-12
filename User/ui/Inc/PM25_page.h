#ifndef _PM25_UI_H_
#define _PM25_UI_H_

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "unified_menu.h"
#include "oled_print.h"
#include "sensordata.h"

// 声明外部传感器状态变量
extern uint8_t DHT11_ON;
extern uint8_t Light_ON;
extern uint8_t PM25_ON;

typedef struct
{
   int16_t last_date_PM;
   u8 result;
   // 刷新标志
   uint8_t need_refresh; // 需要刷新
   uint32_t last_update; // 上次更新时间

} PM25_state_t;

// 声明静态状态变量，避免动态内存分配
extern PM25_state_t g_pm25_state;

/**
 * @brief 初始化PM2.5页面
 * @return 创建的PM2.5菜单项指针
 */
menu_item_t *PM25_init(void);

/**
 * @brief PM2.5自定义绘制函数
 * @param context 绘制上下文
 */
void PM25_draw_function(void *context);

void PM25_key_handler(menu_item_t *item, uint8_t key_event);

PM25_state_t *PM25_get_state(void *context);

void PM25_refresh_display(void *context);

void PM25_on_enter(menu_item_t *item);

void PM25_on_exit(menu_item_t *item);

// PM2.5进度条（line=1）
void OLED_DrawPM25Bar_Line1(uint16_t pm25_value);

// PM2.5等级描述获取函数
const char* PM25_GetLevelString(uint8_t level);

#endif
