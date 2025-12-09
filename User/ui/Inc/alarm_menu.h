#ifndef _ALARM_MENU_H
#define _ALARM_MENU_H

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "unified_menu.h"
#include "oled_print.h"


// ==================================
// 菜单选项枚举
// ==================================

typedef enum {

  ALARM_MENU_CREATE = 0,
  ALARM_MENU_LIST,
    ALARM_MENU_COUNT             // 选项总数
} alarm_menu_option_t;

// ==================================
// 函数声明
// ==================================
menu_item_t* alarm_menu_init(void);
void alarm_menu_on_enter(menu_item_t* item);
void alarm_menu_on_exit(menu_item_t* item);

#endif
