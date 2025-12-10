/**
 * @file main_menu.h
 * @brief 横向主菜单头文件
 * @author flowkite-0689
 * @version v1.0
 * @date 2025.12.05
 */

#ifndef __MAIN_MENU_H
#define __MAIN_MENU_H

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "unified_menu.h"
#include "oled_print.h"

// ==================================
// 菜单选项枚举
// ==================================

typedef enum
{

    MAIN_MENU_SETTINGS = 0, // 设置
    MAIN_MENU_TEMPHUMI,     // 温湿度
    MAIN_MENU_LIGHT,        // 光照
    MAIN_MENU_TEST,         // 测试
    MAIN_MENU_COUNT         // 选项总数
} main_menu_option_t;

// ==================================
// 函数声明
// ==================================

/**
 * @brief 初始化主菜单
 * @return 创建的主菜单项指针
 */
menu_item_t *main_menu_init(void);

/**
 * @brief 主菜单进入回调
 * @param item 菜单项
 */
void main_menu_on_enter(menu_item_t *item);

/**
 * @brief 主菜单退出回调
 * @param item 菜单项
 */
void main_menu_on_exit(menu_item_t *item);

#endif // __MAIN_MENU_H
