#ifndef _TESTLIST_MENU_H_
#define _TESTLIST_MENU_H_


#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "unified_menu.h"
#include "oled_print.h"


// ==================================
// 菜单选项枚举
// ==================================
typedef enum {
    TESTLIST_MENU_SPI_TEST = 0,
    TESTLIST_MENU_2048_OLED,
    TESTLIST_MENU_Frid,
    TESTLIST_MENU_iwdg,
    TESTLIST_MENU_AIR_LEVEL,
    TESTLIST_MENU_COUNT             // 选项总数
} testlist_menu_option_t;
// ==================================
// 函数声明
// ==================================

menu_item_t* testlist_menu_init(void);


void testlist_menu_on_enter(menu_item_t* item);


void testlist_menu_on_exit(menu_item_t* item);

#endif
