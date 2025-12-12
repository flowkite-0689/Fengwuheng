#ifndef _PARAMSETTING_H_
#define _PARAMSETTING_H_

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "unified_menu.h"
#include "oled_print.h"
#include "esp8266.h"
#include "sensordata.h"

// 声明外部变量

typedef struct
{
   uint16_t current_publish_delay;
   uint16_t current_sensor_delay;
   uint8_t selected_item; // 0: publish_delaytime, 1: Sensordata_delaytime
   // 刷新标志
   uint8_t need_refresh; // 需要刷新
   uint32_t last_update; // 上次更新时间

} ParamSetting_state_t;

// 声明静态状态变量，避免动态内存分配
extern ParamSetting_state_t g_paramsetting_state;

/**
 * @brief 初始化参数设置页面
 * @return 创建的参数设置菜单项指针
 */
menu_item_t *ParamSetting_init(void);

/**
 * @brief 参数设置自定义绘制函数
 * @param context 绘制上下文
 */
void ParamSetting_draw_function(void *context);

void ParamSetting_key_handler(menu_item_t *item, uint8_t key_event);

ParamSetting_state_t *ParamSetting_get_state(void *context);

void ParamSetting_refresh_display(void *context);

void ParamSetting_on_enter(menu_item_t *item);

void ParamSetting_on_exit(menu_item_t *item);

#endif
