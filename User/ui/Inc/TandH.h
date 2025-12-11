#ifndef _TANDH_H_
#define _TANDH_H_

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "unified_menu.h"
#include "oled_print.h"
#include "sensordata.h"

typedef struct
{
   // 温湿度信息
   uint8_t humi_int;  // 湿度的整数部分
   uint8_t humi_deci; // 湿度的小数部分
   uint8_t temp_int;  // 温度的整数部分
   uint8_t temp_deci; // 温度的小数部分

   int16_t last_date_T;
   int16_t last_date_H;
   u8 result;

   // 刷新标志
   uint8_t need_refresh; // 需要刷新
   uint32_t last_update; // 上次更新时间

} TandH_state_t;

/**
 * @brief 初始化温湿度页面
 * @return 创建的温湿菜单项指针
 */
menu_item_t *TandH_init(void);

/**
 * @brief 温湿度自定义绘制函数
 * @param context 绘制上下文
 */
void TandH_draw_function(void *context);

void TandH_key_handler(menu_item_t *item, uint8_t key_event);

TandH_state_t *TandH_get_state(void *context);

void TandH_refresh_display(void *context);

void TandH_on_enter(menu_item_t *item);

void TandH_on_exit(menu_item_t *item);

#endif
