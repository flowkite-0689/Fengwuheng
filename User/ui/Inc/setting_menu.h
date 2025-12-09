#ifndef _SETTING_MENU_H
#define _SETTING_MENU_H

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "unified_menu.h"
#include "oled_print.h"

typedef enum {
  SETTING_MENU_SETTIME = 0, // 设置时间界面
  SETTING_MENU_SETDATE, // 设置日期界面
  SETTING_MENU_COUNT // 选项总数

}setting_menu_option_t;


menu_item_t* setting_menu_init(void);

void setting_menu_on_enter(menu_item_t* item);

void setting_menu_on_exit(menu_item_t* item);

#endif
