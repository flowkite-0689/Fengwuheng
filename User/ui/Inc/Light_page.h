#ifndef _LIGHT_UI_H_
#define _LIGHT_UI_H_

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "unified_menu.h"
#include "oled_print.h"
#include "sensordata.h"

typedef struct
{
   int16_t last_date_L;
   u8 result;
   // 刷新标志
   uint8_t need_refresh; // 需要刷新
   uint32_t last_update; // 上次更新时间

} Light_state_t;

// 声明静态状态变量，避免动态内存分配
extern Light_state_t g_light_state;

/**
 * @brief 初始化光照页面
 * @return 创建的光照菜单项指针
 */
menu_item_t *Light_init(void);

/**
 * @brief 光照自定义绘制函数
 * @param context 绘制上下文
 */
void Light_draw_function(void *context);

void Light_key_handler(menu_item_t *item, uint8_t key_event);

Light_state_t *Light_get_state(void *context);

void Light_refresh_display(void *context);

void Light_on_enter(menu_item_t *item);

void Light_on_exit(menu_item_t *item);

// 光照进度条（line=1）
void OLED_DrawLightBar_Line1(uint16_t lux);

#endif
