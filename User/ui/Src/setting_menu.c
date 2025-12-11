#include "setting_menu.h"
#include "SetDate.h"
#include "SetTime.h"
// ==================================
// 图标数组
// ==================================

menu_item_t *setting_menu_init(void)
{
  // 创建设置菜单
  menu_item_t *setting_menu = menu_item_create(
      "Setting Menu",
      MENU_TYPE_HORIZONTAL_ICON,
      (menu_content_t){0} // 不需要内容
  );

  if (setting_menu == NULL)
  {
    return NULL;
  }

  // 创建各个子菜单项
//创建时间设置项
  menu_item_t *SetTime_page = SetTime_init();
  if (SetTime_page != NULL)
  {
    SetTime_page->content.custom.icon_data = gImage_clock;

    menu_add_child(setting_menu, SetTime_page);
  }
//创建日期设置项
menu_item_t *SetDate_page = SetDate_init();
  if (SetDate_page != NULL)
  {
    SetDate_page->content.custom.icon_data = gImage_calendar;
    menu_add_child(setting_menu, SetDate_page);
  }

  return setting_menu;
}

void setting_menu_on_enter(menu_item_t *item)
{
  printf("Enter setting menu\r\n");
  OLED_Clear();
}

void setting_menu_on_exit(menu_item_t *item)
{
  printf("Exit setting menu\r\n");
}
