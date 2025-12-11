/**
 * @file main_menu.c
 * @brief 横向主菜单实现文件
 * @author flowkite-0689
 * @version v1.0
 * @date 2025.12.05
 */

#include "main_menu.h"
//+++++++++++++++++++++++=
// 页面
//++++++++++++++++++++++++=
#include "TandH.h"
#include "setting_menu.h"
#include "testlist_menu.h"
// ==================================
// 图标数组
// ==================================

const unsigned char *main_menu_icons[] = {
    gImage_setting, // 设置
    gImage_TandH,   // 温湿度
    gImage_lightQD, // 光照强度
    gImage_test     // 测试
};

const char *main_menu_names[] = {
    "Stopwatch",
    "Settings",
    "TandH_page",
    "Flashlight",
    "Alarm",
    "Step Counter",
    "Test"};

// ==================================
// 函数实现
// ==================================

menu_item_t *main_menu_init(void)
{

    // 创建主菜单（横向图标菜单）
    menu_item_t *main_menu = menu_item_create(
        "Main Menu",
        MENU_TYPE_HORIZONTAL_ICON,
        (menu_content_t){0} // 主菜单不需要内容
    );

    if (main_menu == NULL)
    {
        return NULL;
    }
    menu_item_set_callbacks(main_menu,
                            main_menu_on_enter, // 进入回调
                            main_menu_on_exit,  // 退出回调
                            NULL,               // 选中回调（不需要特殊处理）
                            NULL);              // 按键处理

    menu_item_t *TandH_page = TandH_init();
    if (TandH_page != NULL)
    {

        TandH_page->content.custom.icon_data = gImage_TandH;
        menu_add_child(main_menu, TandH_page);
    }



    menu_item_t *setting_menu = setting_menu_init();
    if (setting_menu != NULL)
    {
        setting_menu->content.icon.icon_data = gImage_setting;
        menu_add_child(main_menu, setting_menu);
    }

    // 创建各个子菜单项
    // for (uint8_t i = 0; i < MAIN_MENU_COUNT; i++) {
    //     menu_item_t* menu_item = MENU_ITEM_ICON(
    //         main_menu_names[i],
    //         main_menu_icons[i],
    //         32,  // 图标宽度
    //         32   // 图标高度
    //     );

    //     if (menu_item != NULL) {
    //         // 设置回调函数
    //         menu_item_set_callbacks(menu_item,
    //                                NULL,               // 进入回调（不需要特殊处理）
    //                                NULL,               // 退出回调（不需要特殊处理）
    //                                NULL,               // 选中回调（不需要特殊处理）
    //                                NULL);

    //         if (i == MAIN_MENU_TEMPHUMI) { // "Temp&Humid" 是第3个菜单项（索引2）

    //         }
    //         if (i == MAIN_MENU_SETTINGS)
    //         {

    //         }

    //         if (i == MAIN_MENU_TEST)
    //         {

    //         }

    //         // 添加到主菜单
    //         menu_add_child(main_menu, menu_item);

    //     }
    // }

    return main_menu;
}

void main_menu_on_enter(menu_item_t *item)
{
    printf("================================\n");
    printf("Free heap before deletion: %d bytes\n", xPortGetFreeHeapSize());
    printf("Enter main menu\r\n");
    OLED_Clear();
    // 主菜单进入时的初始化操作
}

void main_menu_on_exit(menu_item_t *item)
{
    printf("Exit main menu\r\n");
    // 主菜单退出时的清理操作
}
