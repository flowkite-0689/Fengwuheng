/**
 * @file unified_menu.c
 * @brief 统一菜单架构实现文件
 * @author flowkite-0689
 * @version v1.0
 * @date 2025.11.27
 */

#include "unified_menu.h"
#include <string.h>
#include <stdlib.h>

// ==================================
// 全局菜单系统实例
// ==================================

menu_system_t g_menu_sys = {0};

// ==================================
// 全局闹钟提醒页面
// ==================================

static menu_item_t *g_alarm_alert_page = NULL;

// ==================================
// 静态函数声明
// ==================================

static void menu_update_page_info(menu_item_t *menu);
static void menu_item_deselect_all(menu_item_t *menu);
static void menu_item_update_selection(menu_item_t *menu, uint8_t new_index);
static void menu_set_layout_for_type(menu_type_t type);

// ==================================
// 菜单系统初始化
// ==================================

int8_t menu_system_init(void)
{
    // 创建FreeRTOS资源
    g_menu_sys.event_queue = xQueueCreate(10, sizeof(menu_event_t));
    if (g_menu_sys.event_queue == NULL)
    {
        return -1;
    }

    g_menu_sys.display_mutex = xSemaphoreCreateMutex();
    if (g_menu_sys.display_mutex == NULL)
    {
        return -2;
    }

    // 初始化状态
    g_menu_sys.current_menu = NULL;
    g_menu_sys.root_menu = NULL;
    g_menu_sys.menu_active = 0;
    g_menu_sys.need_refresh = 1;
    g_menu_sys.last_refresh_time = xTaskGetTickCount();
    g_menu_sys.blink_state = 0;
    g_menu_sys.current_page = 0;
    g_menu_sys.total_pages = 1;
    g_menu_sys.items_per_page = 4;

    // 初始化按键处理
    g_menu_sys.last_key_time = 0;
    g_menu_sys.key_debounce_time = (uint8_t)500; // 500ms去抖

    // 设置默认布局配置
    g_menu_sys.layout = (menu_layout_config_t)LAYOUT_HORIZONTAL_MAIN();

    // 预先创建闹钟提醒页面
    // g_alarm_alert_page = alarm_alert_init();
    if (g_alarm_alert_page == NULL)
    {
        printf("Warning: Failed to create alarm alert page\r\n");
    }
    else
    {
        printf("Alarm alert page created successfully\r\n");
    }

    printf("Menu system initialized successfully\r\n");
    return 0;
}

// ==================================
// 菜单项创建和管理
// ==================================

menu_item_t *menu_item_create(const char *name, menu_type_t type, menu_content_t content)
{
    menu_item_t *item = (menu_item_t *)pvPortMalloc(sizeof(menu_item_t));
    if (item == NULL)
    {
        return NULL;
    }
    // printf("MALLOC: menu_item_create %s, size=%d bytes, addr=%p (menu_item_t structure allocation)\n", name, sizeof(menu_item_t), item);

    // 清零结构体
    memset(item, 0, sizeof(menu_item_t));

    // 设置基本信息
    item->name = name;
    item->type = type;
    item->content = content;
    item->display_index = 0;
    item->x_pos = 0;
    item->y_pos = 0;
    item->width = (type == MENU_TYPE_HORIZONTAL_ICON) ? 32 : 128;
    item->height = (type == MENU_TYPE_HORIZONTAL_ICON) ? 32 : 16;

    // 设置默认状态
    item->is_selected = 0;
    item->is_visible = 1;
    item->is_enabled = 1;

    // 设置默认关系
    item->parent = NULL;
    item->children = NULL;
    item->child_count = 0;
    item->selected_child = 0;

    // 设置默认上下文
    item->context = NULL;

    return item;
}

int8_t menu_add_child(menu_item_t *parent, menu_item_t *child)
{
    if (parent == NULL || child == NULL)
    {
        return -1;
    }

    // 不允许重复添加同一 child（防循环/重复）
    for (uint8_t i = 0; i < parent->child_count; i++)
    {
        if (parent->children && parent->children[i] == child)
        {
            return -3; // already exists
        }
    }

    // 分配新的指针数组（多一个 slot）
    menu_item_t **new_children = (menu_item_t **)pvPortMalloc(
        sizeof(menu_item_t *) * (parent->child_count + 1));
    if (new_children == NULL)
    {
        return -2; // malloc failed
    }
    // printf("MALLOC: menu_add_child %s->%s, size=%d bytes, addr=%p (new children pointer array allocation)\n",
    //    parent->name, child->name, sizeof(menu_item_t *) * (parent->child_count + 1), new_children);

    // 拷贝旧指针（如果有）
    if (parent->child_count > 0 && parent->children != NULL)
    {
        memcpy(new_children, parent->children, sizeof(menu_item_t *) * parent->child_count);
        // printf("FREE: menu_add_child %s, old children pointer array addr=%p, size=%d bytes (old children array release)\n",
        //    parent->name, parent->children, sizeof(menu_item_t *) * parent->child_count);
        vPortFree(parent->children); // 释放旧数组
    }

    // 添加新 child 指针到末尾
    new_children[parent->child_count] = child; // ←←← 存的是指针！不是结构体！

    // 更新 parent
    parent->children = new_children;
    parent->child_count++;
    child->parent = parent;

    return 0;
}

int8_t menu_item_set_position(menu_item_t *item, uint8_t x, uint8_t y, uint8_t width, uint8_t height)
{
    if (item == NULL)
    {
        return -1;
    }

    item->x_pos = x;
    item->y_pos = y;
    item->width = width;
    item->height = height;

    return 0;
}

int8_t menu_item_set_callbacks(menu_item_t *item,
                               void (*on_enter)(menu_item_t *),
                               void (*on_exit)(menu_item_t *),
                               void (*on_select)(menu_item_t *),
                               void (*on_key)(menu_item_t *, uint8_t))
{
    if (item == NULL)
    {
        return -1;
    }

    item->on_enter = on_enter;
    item->on_exit = on_exit;
    item->on_select = on_select;
    item->on_key = on_key;

    return 0;
}

int8_t menu_remove_child(menu_item_t *parent, menu_item_t *child)
{
    if (parent == NULL || child == NULL || parent->child_count == 0 || parent->children == NULL)
    {
        return -1;
    }

    // 查找 child 指针在 children 数组中的位置（指针比较！）
    int8_t index = -1;
    for (uint8_t i = 0; i < parent->child_count; i++)
    {
        if (parent->children[i] == child)
        { //
            index = i;
            break;
        }
    }
    if (index == -1)
        return -2; // 未找到

    // 调整 selected_child
    if (parent->selected_child == index)
    {
        if (parent->child_count > 1)
        {
            parent->selected_child = (index == 0) ? 0 : index - 1;
        }
        else
        {
            parent->selected_child = 0;
        }
    }
    else if (parent->selected_child > index)
    {
        parent->selected_child--;
    }

    // 重新分配 children 指针数组（少一个）
    menu_item_t **new_children = NULL;
    uint8_t new_count = parent->child_count - 1;

    if (new_count > 0)
    {
        new_children = (menu_item_t **)pvPortMalloc(sizeof(menu_item_t *) * new_count);
        if (new_children == NULL)
            return -3;

        uint8_t j = 0;
        for (uint8_t i = 0; i < parent->child_count; i++)
        {
            if (i != index)
            {
                new_children[j++] = parent->children[i]; // 拷贝指针
            }
        }
    }

    // 释放旧数组，更新
    // printf("FREE: menu_remove_child %s, old children pointer array addr=%p, size=%d bytes (old children array release during removal)\n",
    //        parent->name, parent->children, sizeof(menu_item_t *) * parent->child_count);
    vPortFree(parent->children);
    parent->children = new_children;
    parent->child_count = new_count;

    // 清除 child 的 parent 指针（防止野指针）
    child->parent = NULL;

    return 0;
}
int8_t menu_item_delete(menu_item_t *item)
{
    if (!item)
        return -1;
    if (item == g_menu_sys.current_menu || item == g_menu_sys.root_menu)
        return -2;

    //  加锁
    if (g_menu_sys.display_mutex)
    {
        if (xSemaphoreTake(g_menu_sys.display_mutex, pdMS_TO_TICKS(100)) != pdTRUE)
            return -5;
    }

// printf("================================\n");
// printf("Free heap before deletion: %d bytes\n", xPortGetFreeHeapSize());
// printf("Deleting menu item: %s (addr=%p)\n", item->name, item);
// printf("================================\n");

// //  用栈模拟递归（避免爆栈）
#define MAX_STACK_DEPTH 32 // 增加深度
    menu_item_t *stack[MAX_STACK_DEPTH];
    int top = 0;

    // 安全检查：防止循环引用
    for (int i = 0; i < top; i++)
    {
        if (stack[i] == item)
        {
            printf("ERROR: Circular reference detected!\n");
            if (g_menu_sys.display_mutex)
                xSemaphoreGive(g_menu_sys.display_mutex);
            return -6;
        }
    }

    stack[top++] = item;

    while (top > 0)
    {
        menu_item_t *cur = stack[--top];

        // 安全检查
        if (!cur)
            continue;

        printf("Processing: %s (child_count=%d)\n", cur->name, cur->child_count);

        // 子项入栈（后进先出）
        if (cur->child_count > 0 && cur->children != NULL)
        {
            for (int8_t i = cur->child_count - 1; i >= 0; i--)
            {
                if (cur->children[i] != NULL && top < MAX_STACK_DEPTH - 1)
                {
                    // 安全检查：防止重复入栈
                    int already_in_stack = 0;
                    for (int j = 0; j < top; j++)
                    {
                        if (stack[j] == cur->children[i])
                        {
                            already_in_stack = 1;
                            break;
                        }
                    }
                    if (!already_in_stack)
                    {
                        stack[top++] = cur->children[i];
                    }
                }
            }
        }

        // 释放当前项的资源
        // 1. 释放子项指针数组
        if (cur->children != NULL)
        {
            // printf("FREE: %s children array, addr=%p, size=%d bytes\n",
            //    cur->name, cur->children, sizeof(menu_item_t *) * cur->child_count);
            vPortFree(cur->children);
            cur->children = NULL;
            cur->child_count = 0;
        }

        // 2. 释放上下文（如果有）
        if (cur->context != NULL)
        {
            // printf("FREE: %s context, addr=%p\n", cur->name, cur->context);
            vPortFree(cur->context);
            cur->context = NULL;
        }

        // 3. 释放菜单项结构本身
        // printf("FREE: %s menu_item, addr=%p, size=%d bytes\n",
        //        cur->name, cur, sizeof(menu_item_t));
        vPortFree(cur);

        // printf("Heap after freeing %s: %d bytes\n", cur->name, xPortGetFreeHeapSize());
    }

    g_menu_sys.need_refresh = 1;
    if (g_menu_sys.display_mutex)
        xSemaphoreGive(g_menu_sys.display_mutex);

    printf("================================\n");
    printf("Free heap after deletion: %d bytes\n", xPortGetFreeHeapSize());
    printf("================================\n");

    return 0;
}
// ==================================
// 菜单显示实现
// ==================================

void menu_refresh_display(void)
{
    if (g_menu_sys.current_menu == NULL)
    {
        return;
    }

    if (xSemaphoreTake(g_menu_sys.display_mutex, pdMS_TO_TICKS(50)) != pdTRUE)
    {
        return;
    }

    switch (g_menu_sys.current_menu->type)
    {
    case MENU_TYPE_HORIZONTAL_ICON:
        menu_display_horizontal(g_menu_sys.current_menu);
        break;

    case MENU_TYPE_VERTICAL_LIST:
        menu_display_vertical(g_menu_sys.current_menu);
        break;

    case MENU_TYPE_CUSTOM:
        menu_display_custom(g_menu_sys.current_menu);
        break;

    default:
        break;
    }

    g_menu_sys.last_refresh_time = xTaskGetTickCount();
    g_menu_sys.need_refresh = 0;

    xSemaphoreGive(g_menu_sys.display_mutex);
}

void menu_display_horizontal(menu_item_t *menu)
{
    if (menu == NULL || menu->child_count == 0)
    {
        return;
    }

    // 计算可见范围（显示3个：左、中、右）
    uint8_t center_index = menu->selected_child;
    uint8_t left_index = (center_index == 0) ? menu->child_count - 1 : center_index - 1;
    uint8_t right_index = (center_index + 1) % menu->child_count;

    // 显示左侧图标（淡化）
    if (menu->children[left_index]->content.icon.icon_data)
    {
        if (menu->children[left_index]->type == MENU_TYPE_CUSTOM)
        {
            OLED_ShowPicture(0, 16, 32, 32,
                             menu->children[left_index]->content.custom.icon_data, 1);
        }
        else
        {
            OLED_ShowPicture(0, 16, 32, 32,
                             menu->children[left_index]->content.icon.icon_data, 1);
        }
    }

    // 显示中间图标（清晰）
    if (menu->children[center_index]->content.icon.icon_data)
    {
        OLED_Printf_Line(3,"       %s",menu->children[center_index]->name);
        if (menu->children[center_index]->type == MENU_TYPE_CUSTOM)
        {
            OLED_ShowPicture(48, 16, 32, 32,
                             menu->children[center_index]->content.custom.icon_data, 0);

        }
        else
        {
            OLED_ShowPicture(48, 16, 32, 32,
                             menu->children[center_index]->content.icon.icon_data, 0);
        }
    }

    // 显示右侧图标（淡化）
    if (menu->children[right_index]->content.icon.icon_data)
    {
        if (menu->children[right_index]->type == MENU_TYPE_CUSTOM)
        {
            OLED_ShowPicture(96, 16, 32, 32,
                             menu->children[right_index]->content.custom.icon_data, 1);
        }
        else
        {

            OLED_ShowPicture(96, 16, 32, 32,
                             menu->children[right_index]->content.icon.icon_data, 1);
        }
    }

    OLED_Refresh();
}

void menu_display_vertical(menu_item_t *menu)
{
    if (menu == NULL || menu->child_count == 0)
    {
        return;
    }

    // 更新分页信息
    menu_update_page_info(menu);

    // 计算当前页的项目范围
    uint8_t start_index = g_menu_sys.current_page * g_menu_sys.items_per_page;
    uint8_t end_index = start_index + g_menu_sys.items_per_page;
    if (end_index > menu->child_count)
    {
        end_index = menu->child_count;
    }

    // 显示当前页的项目（类似你想要的tsetlist_RE功能）
    for (uint8_t i = start_index; i < end_index; i++)
    {
        uint8_t line = i - start_index;
        char arrow = (i == menu->selected_child) ? '>' : ' ';

        OLED_Printf_Line(line, "%c %s", arrow, menu->children[i]->content.text.text);
    }

    // 如果本页不足4行，下面几行清空
    for (uint8_t i = end_index - start_index; i < g_menu_sys.items_per_page; i++)
    {
        OLED_Clear_Line(i);
    }

    OLED_Refresh_Dirty();
}

void menu_clear_and_redraw(void)
{
    OLED_Clear();
    menu_refresh_display();
}

// ==================================
// 菜单事件处理
// ==================================

menu_event_t menu_key_to_event(uint8_t key)
{
    menu_event_t event;
    memset(&event, 0, sizeof(menu_event_t));
    event.timestamp = xTaskGetTickCount();
    BEEP_Buzz(1);
    printf("key press - > %d\n", key - 1);
    switch (key)
    {
    case 1:
        event.type = MENU_EVENT_KEY_UP;
        break;
    case 2:
        event.type = MENU_EVENT_KEY_DOWN;
        break;
    case 3:
        event.type = MENU_EVENT_KEY_SELECT;
        break;
    case 4:
        event.type = MENU_EVENT_KEY_ENTER;
        break;
    default:
        event.type = MENU_EVENT_NONE;
        break;
    }

    return event;
}

int8_t menu_process_event(menu_event_t *event)
{
    if (event == NULL)
    {
        return -1;
    }

    // 处理闹钟事件（特殊处理，不需要当前菜单）
    if (event->type == MENU_EVENT_ALARM)
    {
        printf("Processing alarm event, index: %d\n", event->param);

        // 检查闹钟提醒页面是否已创建
        if (g_alarm_alert_page == NULL)
        {
            printf("Error: Alarm alert page not created\n");
            return -1;
        }

        // 触发闹钟提醒
        //        if (alarm_alert_trigger(event->param) == 0) {
        //            // 设置闹钟提醒页面的父菜单为当前菜单（如果存在）
        //            if (g_menu_sys.current_menu != NULL) {
        //                g_alarm_alert_page->parent = g_menu_sys.current_menu;
        //            }
        //            // 切换到闹钟提醒页面
        //            menu_enter(g_alarm_alert_page);
        //            printf("Switched to alarm alert page\n");
        //        }
        return 0;
    }

    if (g_menu_sys.current_menu == NULL)
    {
        return -1;
    }

    menu_item_t *current = g_menu_sys.current_menu;

    // 按键去抖处理
    uint32_t current_time = xTaskGetTickCount();
    if (event->type != MENU_EVENT_NONE && event->type != MENU_EVENT_REFRESH)
    {
        if (current_time - g_menu_sys.last_key_time < pdMS_TO_TICKS(g_menu_sys.key_debounce_time))
        {
            return 0; // 去抖，忽略按键
        }
        g_menu_sys.last_key_time = current_time;
    }

    // 调用自定义按键处理（如果存在）
    if (current->on_key)
    {
        current->on_key(current, event->type);
        // 返回0表示事件已处理
        return 0;
    }

    // 默认按键处理
    switch (current->type)
    {
    case MENU_TYPE_HORIZONTAL_ICON:
        return menu_handle_horizontal_key(current, event->type);

    case MENU_TYPE_VERTICAL_LIST:
        return menu_handle_vertical_key(current, event->type);

    default:
        return -2;
    }
}

int8_t menu_handle_horizontal_key(menu_item_t *menu, uint8_t key_event)
{
    if (menu == NULL || menu->child_count == 0)
    {
        return -1;
    }

    switch (key_event)
    {
    case MENU_EVENT_KEY_UP:
        // 上一个选项
        menu_item_update_selection(menu, (menu->selected_child == 0) ? menu->child_count - 1 : menu->selected_child - 1);
        break;

    case MENU_EVENT_KEY_DOWN:
        // 下一个选项
        menu_item_update_selection(menu, (menu->selected_child + 1) % menu->child_count);
        break;

    case MENU_EVENT_KEY_SELECT:
        menu_back_to_parent();
        break;

    case MENU_EVENT_KEY_ENTER:
        // 进入选中功能
        return menu_enter_selected();

    case MENU_EVENT_REFRESH:
        menu_display_horizontal(menu);
        break;

    default:
        return -3;
    }

    return 0;
}

int8_t menu_handle_vertical_key(menu_item_t *menu, uint8_t key_event)
{
    if (menu == NULL || menu->child_count == 0)
    {
        return -1;
    }

    switch (key_event)
    {
    case MENU_EVENT_KEY_UP:
        // 上一个选项（类似你想要的testlist循环选择）
        if (menu->selected_child == 0)
        {
            menu->selected_child = menu->child_count - 1;
        }
        else
        {
            menu->selected_child--;
        }
        printf("selected : %d\n", menu->selected_child);
        // 更新分页信息
        menu_update_page_info(menu);
        g_menu_sys.need_refresh = 1;
        break;

    case MENU_EVENT_KEY_DOWN:
        // 下一个选项（循环选择）
        menu->selected_child = (menu->selected_child + 1) % menu->child_count;
        printf("selected : %d\n", menu->selected_child);
        // 更新分页信息
        menu_update_page_info(menu);
        g_menu_sys.need_refresh = 1;
        break;

    case MENU_EVENT_KEY_SELECT:
        // 返回
        return menu_back_to_parent();

    case MENU_EVENT_KEY_ENTER:
        // 进入选中功能（类似test_enter_select功能）
        return menu_enter_selected();

    case MENU_EVENT_REFRESH:
        menu_display_vertical(menu);
        break;

    default:
        return -3;
    }

    return 0;
}

// ==================================
// 菜单导航实现
// ==================================

int8_t menu_enter(menu_item_t *menu)
{
    if (menu == NULL)
    {
        return -1;
    }

    // 调用退出回调
    if (g_menu_sys.current_menu && g_menu_sys.current_menu->on_exit)
    {
        g_menu_sys.current_menu->on_exit(g_menu_sys.current_menu);
    }

    // 设置新菜单
    g_menu_sys.current_menu = menu;
    g_menu_sys.menu_active = 1;
    g_menu_sys.need_refresh = 1;

    // 根据菜单类型设置布局配置
    menu_set_layout_for_type(menu->type);

    // 重置分页信息
    g_menu_sys.current_page = 0;
    menu_update_page_info(menu);

    // 调用进入回调
    if (menu->on_enter)
    {
        menu->on_enter(menu);
    }
    printf("\nparent : %s ,\n current : %s \n", g_menu_sys.current_menu->parent->name, g_menu_sys.current_menu->name);
    return 0;
}

int8_t menu_back_to_parent(void)
{
    printf("menu_back_to_parent\n");
    printf("\nparent : %s ,\n current : %s \n", g_menu_sys.current_menu->parent->name, g_menu_sys.current_menu->name);
    if (g_menu_sys.current_menu == NULL || g_menu_sys.current_menu->parent == NULL)
    {
        return -1;
    }
    OLED_Clear();
    menu_item_t *parent = g_menu_sys.current_menu->parent;

    // 调用退出回调
    if (g_menu_sys.current_menu->on_exit)
    {
        g_menu_sys.current_menu->on_exit(g_menu_sys.current_menu);
    }

    // 返回父菜单
    g_menu_sys.current_menu = parent;

    // 根据父菜单类型设置布局
    menu_set_layout_for_type(parent->type);

    // 重置分页信息
    g_menu_sys.current_page = 0;
    menu_update_page_info(parent);

    // 刷新显示
    g_menu_sys.need_refresh = 1;

    printf("back to ->  %s\n", parent->name);
    // 调用父菜单的进入回调
    if (parent->on_enter)
    {
        parent->on_enter(parent);
    }

    return 0;
}

int8_t menu_select_next(void)
{
    if (g_menu_sys.current_menu == NULL || g_menu_sys.current_menu->child_count == 0)
    {
        return -1;
    }

    menu_item_t *menu = g_menu_sys.current_menu;
    menu->selected_child = (menu->selected_child + 1) % menu->child_count;
    g_menu_sys.need_refresh = 1;

    return 0;
}

int8_t menu_select_previous(void)
{
    if (g_menu_sys.current_menu == NULL || g_menu_sys.current_menu->child_count == 0)
    {
        return -1;
    }

    menu_item_t *menu = g_menu_sys.current_menu;
    if (menu->selected_child == 0)
    {
        menu->selected_child = menu->child_count - 1;
    }
    else
    {
        menu->selected_child--;
    }
    g_menu_sys.need_refresh = 1;

    return 0;
}

int8_t menu_enter_selected(void)
{
    if (g_menu_sys.current_menu == NULL || g_menu_sys.current_menu->child_count == 0)
    {
        return -1;
    }

    menu_item_t *menu = g_menu_sys.current_menu;
    menu_item_t *selected = menu->children[menu->selected_child];

    printf("menu_enter_selected: current=%s, selected=%s, child_count=%d\n",
           menu->name, selected->name, selected->child_count);

    // 调用选中回调
    if (selected->on_select)
    {
        selected->on_select(selected);
    }

    // 设置父菜单关系
    selected->parent = g_menu_sys.current_menu;

    // 根据菜单类型决定如何进入
    if (selected->child_count > 0)
    {
        // 有子菜单的菜单项：直接进入该菜单
        printf("menu_enter_selected - Entering menu with children\n");
        printf("parent : %s ,\n current : %s \n", selected->parent->name, selected->name);
        return menu_enter(selected);
    }
    else
    {
        // 没有子菜单的菜单项：可能是功能页面或自定义页面
        printf("menu_enter_selected - Entering leaf node (custom page/function)\n");
        printf("parent : %s ,\n current : %s \n", selected->parent->name, selected->name);

        // 调用进入回调
        if (selected->on_enter)
        {
            selected->on_enter(selected);
        }

        // 进入该页面
        return menu_enter(selected);
    }
}

// ==================================
// FreeRTOS任务实现
// ==================================

void menu_task(void *pvParameters)
{
    const TickType_t delay_20ms = pdMS_TO_TICKS(20);
    menu_event_t event;

    while (1)
    {
        // 处理菜单事件
        if (xQueueReceive(g_menu_sys.event_queue, &event, 0) == pdPASS)
        {
            menu_process_event(&event);
        }

        // 定时刷新显示
        if (g_menu_sys.need_refresh ||
            (xTaskGetTickCount() - g_menu_sys.last_refresh_time) > pdMS_TO_TICKS(100))
        {
            menu_refresh_display();
        }

        vTaskDelay(delay_20ms);
    }
}

void menu_key_task(void *pvParameters)
{
    const TickType_t delay_10ms = pdMS_TO_TICKS(10);
    uint8_t key;

    while (1)
    {
        if ((key = KEY_Get()) != 0)
        {
            menu_event_t event = menu_key_to_event(key);
            if (event.type != MENU_EVENT_NONE)
            {
                xQueueSend(g_menu_sys.event_queue, &event, 0);
            }
        }

        vTaskDelay(delay_10ms);
    }
}

// ==================================
// 静态辅助函数实现
// ==================================

static void menu_update_page_info(menu_item_t *menu)
{
    if (menu == NULL || menu->type != MENU_TYPE_VERTICAL_LIST)
    {
        return;
    }

    g_menu_sys.items_per_page = g_menu_sys.layout.vertical.items_per_page;
    g_menu_sys.total_pages = (menu->child_count + g_menu_sys.items_per_page - 1) / g_menu_sys.items_per_page;

    // 确保当前页在有效范围内
    if (g_menu_sys.current_page >= g_menu_sys.total_pages)
    {
        g_menu_sys.current_page = g_menu_sys.total_pages - 1;
    }

    // 确保选中项在当前页内
    uint8_t page_start = g_menu_sys.current_page * g_menu_sys.items_per_page;
    uint8_t page_end = page_start + g_menu_sys.items_per_page - 1;

    if (menu->selected_child < page_start)
    {
        g_menu_sys.current_page = menu->selected_child / g_menu_sys.items_per_page;
    }
    else if (menu->selected_child > page_end)
    {
        g_menu_sys.current_page = menu->selected_child / g_menu_sys.items_per_page;
    }
}

static void menu_item_deselect_all(menu_item_t *menu)
{
    if (menu == NULL)
    {
        return;
    }

    // 取消选中所有子项
    for (uint8_t i = 0; i < menu->child_count; i++)
    {
        menu->children[i]->is_selected = 0;
    }
}

static void menu_item_update_selection(menu_item_t *menu, uint8_t new_index)
{
    if (menu == NULL || new_index >= menu->child_count)
    {
        return;
    }

    // 取消所有选中
    menu_item_deselect_all(menu);

    // 设置新选中项
    menu->selected_child = new_index;
    menu->children[new_index]->is_selected = 1;

    g_menu_sys.need_refresh = 1;
}

// ==================================
// 自定义页面显示实现
// ==================================

void menu_display_custom(menu_item_t *menu)
{
    if (menu == NULL || menu->content.custom.draw_function == NULL)
    {
        return;
    }

    // 调用自定义绘制函数
    menu->content.custom.draw_function(menu->content.custom.draw_context);
}

// ==================================
// 菜单布局设置函数
// ==================================

static void menu_set_layout_for_type(menu_type_t type)
{
    switch (type)
    {
    case MENU_TYPE_HORIZONTAL_ICON:
        g_menu_sys.layout = (menu_layout_config_t)LAYOUT_HORIZONTAL_MAIN();
        g_menu_sys.items_per_page = g_menu_sys.layout.horizontal.visible_count;
        break;

    case MENU_TYPE_VERTICAL_LIST:
        g_menu_sys.layout = (menu_layout_config_t)LAYOUT_VERTICAL_TEST();
        g_menu_sys.items_per_page = g_menu_sys.layout.vertical.items_per_page;
        break;

    case MENU_TYPE_CUSTOM:
        // 自定义页面使用默认布局
        g_menu_sys.layout = (menu_layout_config_t)LAYOUT_HORIZONTAL_MAIN();
        g_menu_sys.items_per_page = 4;
        break;

    default:
        break;
    }

    printf("Set layout for menu type: %d\r\n", type);
}
