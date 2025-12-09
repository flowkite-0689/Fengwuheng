#include "alarm_menu.h"
#include "alarm_add.h"
#include "alarm_list.h"

//+++++++++++++++++++++++=
// 页面
//++++++++++++++++++++++++=

// ==================================
// 图标数组
// ==================================

const unsigned char *alarm_menu_icons[] =
    {
        gImage_add, // 新建闹钟图标
        gImage_list // 闹钟列表图标
};

const char *alarm_menu_names[] =
    {
        "AlarmAdd",
        "AlarmList"};

menu_item_t *alarm_menu_init(void)
{
  menu_item_t *alarm_menu = menu_item_create(
      "Alarm Menu",
      MENU_TYPE_HORIZONTAL_ICON,
      (menu_content_t){0} // 主菜单不需要内容
  );
  if (alarm_menu == NULL)
  {
    return NULL;
  }
  for (uint8_t i = 0; i < ALARM_MENU_COUNT; i++)
  {
    menu_item_t *menu_item = MENU_ITEM_ICON(
        alarm_menu_names[i],
        alarm_menu_icons[i],
        32, // 图标宽度
        32  // 图标高度
    );

    if (menu_item != NULL)
    {
      // 设置回调函数
            menu_item_set_callbacks(menu_item, 
                                   alarm_menu_on_enter,  // 进入回调
                                   alarm_menu_on_exit,   // 退出回调
                                   NULL,               // 选中回调（不需要特殊处理）
                                   NULL); // 按键处理
 
  // 创建AlarmAdd页面
  if(i==0){
  menu_item_t *alarm_add_page = alarm_add_init();
  if (alarm_add_page != NULL) {
    menu_add_child(menu_item, alarm_add_page);
  }
}
  // 创建AlarmList页面
  if(i==1){
  menu_item_t *alarm_list_page = alarm_list_init();
  if (alarm_list_page != NULL) {
    menu_add_child(menu_item, alarm_list_page);
  }
}
  
                menu_add_child(alarm_menu, menu_item);                     
    }
    
  }
  return alarm_menu;
}
void alarm_menu_on_enter(menu_item_t *item)
{
  printf("Enter alarm menu\r\n");
    OLED_Clear();
}
void alarm_menu_on_exit(menu_item_t *item){
  printf("Exit alarm menu\r\n");
}
