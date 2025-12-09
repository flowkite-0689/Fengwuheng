#include "setting_menu.h"
#include "SetDate.h"
#include "SetTime.h"
// ==================================
// 图标数组
// ==================================
const unsigned char *setting_menu_icons[] =
    {
        gImage_clock,
        gImage_calendar};

const char *setting_menu_names[] =
    {
        "SetTime",
        "SetDate"};

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
  for (uint8_t i = 0; i < SETTING_MENU_COUNT; i++)
  {
    menu_item_t *menu_item = MENU_ITEM_ICON(
        setting_menu_names[i],
        setting_menu_icons[i],
        32, // 图标宽度
        32  // 图标高度
    );
    if (menu_item != NULL)
    {
      // 设置回调函数
      menu_item_set_callbacks(menu_item,
                              setting_menu_on_enter, // 进入回调
                              setting_menu_on_exit,  // 退出回调
                              NULL,                  // 选中回调（不需要特殊处理）
                              NULL);                 // 按键处理

              if (i==0)
              {
                menu_item_t* SetTime_page =SetTime_init();
                if (SetTime_page != NULL)
                {
                    menu_add_child(menu_item, SetTime_page);
                }
              }
              if (i == 1)
              {
                 menu_item_t* SetDate_page =SetDate_init();
                if (SetDate_page != NULL)
                {
                    menu_add_child(menu_item, SetDate_page);
                }
              }
              
              
    }
    menu_add_child(setting_menu, menu_item);
  }
  return setting_menu;
}

void setting_menu_on_enter(menu_item_t *item){
  printf("Enter setting menu\r\n");
    OLED_Clear();
}

void setting_menu_on_exit(menu_item_t *item)
{
  printf("Exit setting menu\r\n");
}
