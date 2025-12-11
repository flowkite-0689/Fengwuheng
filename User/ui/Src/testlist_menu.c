#include "testlist_menu.h"



// 引用全局菜单系统变量
extern menu_system_t g_menu_sys;

const char *testlist_menu_text[] = {
  
    "2048_oled"};

menu_item_t *testlist_menu_init(void)
{
  menu_item_t *testlist_menu = menu_item_create(
      "TestList Menu",
      MENU_TYPE_VERTICAL_LIST,
      (menu_content_t){0} // 主菜单不需要内容
  );

  if (testlist_menu == NULL)
  {
    printf("testlist init fail\r\n");
    return NULL;
  }

  
 return testlist_menu;
}

void testlist_menu_on_enter(menu_item_t *item)
{
  printf("Enter testlist menu\r\n");
  OLED_Clear();
}

void testlist_menu_on_exit(menu_item_t *item)
{
   printf("Exit testlist menu\r\n");
}
