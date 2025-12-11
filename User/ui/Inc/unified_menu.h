/**
 * @file unified_menu.h
 * @brief 统一菜单架构头文件 - 兼容横向图标菜单和竖向列表菜单
 * @author flowkite-0689
 * @version v1.0
 * @date 2025.11.27
 */

#ifndef __UNIFIED_MENU_H
#define __UNIFIED_MENU_H

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "oled_print.h"
#include "Key.h"
#include <stdio.h>
#include "beep.h"

// ==================================
// 菜单类型枚举
// ==================================

typedef enum {
    MENU_TYPE_HORIZONTAL_ICON,  // 横向图标菜单（如主菜单）
    MENU_TYPE_VERTICAL_LIST,    // 竖向列表菜单（如测试菜单）
    MENU_TYPE_CUSTOM           // 自定义页面（完全自定义绘制和交互）
} menu_type_t;

// ==================================
// 菜单项数据类型联合体
// ==================================

typedef struct {
    const unsigned char *icon_data;  // 图标数据
    // uint8_t icon_width;             // 图标宽度
    // uint8_t icon_height;            // 图标高度
} menu_icon_t;

typedef struct {
    const char *text;               // 文本内容
    // uint8_t max_length;             // 最大显示长度
} menu_text_t;
typedef struct {
      void (*draw_function)(void* context);     // 自定义绘制函数
      void* draw_context;                       // 绘制上下文数据
    const unsigned char *icon_data;  // 图标数据
     const char *text;  
  } menu_custom_t;




typedef union {
    menu_icon_t icon;               // 图标数据
    menu_text_t text;               // 文本数据
    menu_custom_t custom;                      // 自定义页面数据
} menu_content_t;

// ==================================
// 菜单项结构体
// ==================================

typedef struct menu_item {
    // 基本信息
    const char *name;                    // 菜单项名称（内部使用）
    menu_type_t type;                   // 菜单类型
    
    // 显示内容
    menu_content_t content;             // 显示内容（图标或文本）
    
    // 位置和布局信息
    uint8_t display_index;              // 显示索引
    uint8_t x_pos;                      // X坐标
    uint8_t y_pos;                      // Y坐标
    uint8_t width;                      // 宽度
    uint8_t height;                     // 高度
    
    // 状态信息
    uint8_t is_selected;                // 是否选中
    uint8_t is_visible;                 // 是否可见
    uint8_t is_enabled;                 // 是否启用
    
    // 回调函数
    void (*on_enter)(struct menu_item *item);        // 进入时回调
    void (*on_exit)(struct menu_item *item);         // 退出时回调
    void (*on_select)(struct menu_item *item);       // 选中时回调
    void (*on_key)(struct menu_item *item, uint8_t key); // 按键处理
    
    // 层次关系
    struct menu_item *parent;           // 父菜单
    struct menu_item **children;         // 子菜单数组
    uint8_t child_count;                 // 子菜单数量
    uint8_t selected_child;              // 选中的子项索引
    
    // 上下文数据
    void *context;                       // 自定义上下文
} menu_item_t;

// ==================================
// 菜单布局配置结构体
// ==================================

typedef struct {
    // 横向菜单配置
    struct {
        uint8_t visible_count;          // 可见项目数量（通常为3）
        uint8_t item_width;             // 项目宽度
        uint8_t item_height;            // 项目高度
        uint8_t spacing;                // 项目间距
        uint8_t start_x;                // 起始X坐标
        uint8_t start_y;                // 起始Y坐标
    } horizontal;
    
    // 竖向菜单配置
    struct {
        uint8_t items_per_page;         // 每页项目数量
        uint8_t item_height;            // 项目高度
        uint8_t start_x;                // 起始X坐标
        uint8_t start_y;                // 起始Y坐标
        uint8_t indent_spaces;           // 缩进空格数
        char selected_char;              // 选中字符
        char unselected_char;            // 未选中字符
    } vertical;
} menu_layout_config_t;

// ==================================
// 菜单系统结构体
// ==================================

typedef struct {
    // 当前状态
    menu_item_t *current_menu;           // 当前菜单
    menu_item_t *root_menu;              // 根菜单
    uint8_t menu_active;                 // 菜单激活状态
    
    // 布局配置
    menu_layout_config_t layout;         // 布局配置
    
    // 分页信息（用于竖向菜单）
    uint8_t current_page;                // 当前页码
    uint8_t total_pages;                  // 总页数
    uint8_t items_per_page;              // 每页项目数
    
    // 显示控制
    uint8_t need_refresh;                // 需要刷新标志
    uint32_t last_refresh_time;          // 上次刷新时间
    uint8_t blink_state;                 // 闪烁状态
    
    // FreeRTOS资源
    QueueHandle_t event_queue;           // 事件队列
    SemaphoreHandle_t display_mutex;     // 显示互斥量
    
    // 按键处理
    uint32_t last_key_time;              // 上次按键时间
    uint8_t key_debounce_time;           // 按键去抖时间(ms)
} menu_system_t;

// ==================================
// 菜单事件类型
// ==================================

typedef enum {
    MENU_EVENT_NONE,
    MENU_EVENT_KEY_UP,       // KEY0 - 上/左
    MENU_EVENT_KEY_DOWN,     // KEY1 - 下/右
    MENU_EVENT_KEY_SELECT,   // KEY2 - 选择/返回
    MENU_EVENT_KEY_ENTER,    // KEY3 - 确认/进入
    MENU_EVENT_REFRESH,      // 刷新显示
    MENU_EVENT_TIMEOUT,      // 超时
    MENU_EVENT_ALARM         // 闹钟事件
} menu_event_type_t;

typedef struct {
    menu_event_type_t type;
    uint32_t timestamp;
    uint8_t param;
} menu_event_t;

// ==================================
// 全局菜单系统实例
// ==================================

extern menu_system_t g_menu_sys;

// ==================================
// 菜单创建和管理API
// ==================================

/**
 * @brief 初始化菜单系统
 * @return 0-成功，其他-失败
 */
int8_t menu_system_init(void);

/**
 * @brief 创建菜单项
 * @param name 菜单名称
 * @param type 菜单类型
 * @param content 菜单内容
 * @return 创建的菜单项指针，NULL表示失败
 */
menu_item_t* menu_item_create(const char *name, menu_type_t type, menu_content_t content);

/**
 * @brief 添加子菜单项
 * @param parent 父菜单项
 * @param child 子菜单项
 * @return 0-成功，其他-失败
 */
int8_t menu_add_child(menu_item_t *parent, menu_item_t *child);

/**
 * @brief 设置菜单项位置
 * @param item 菜单项
 * @param x X坐标
 * @param y Y坐标
 * @param width 宽度
 * @param height 高度
 * @return 0-成功，其他-失败
 */
int8_t menu_item_set_position(menu_item_t *item, uint8_t x, uint8_t y, uint8_t width, uint8_t height);

/**
 * @brief 设置菜单项回调函数
 * @param item 菜单项
 * @param on_enter 进入回调
 * @param on_exit 退出回调
 * @param on_select 选中回调
 * @param on_key 按键处理回调
 * @return 0-成功，其他-失败
 */
int8_t menu_item_set_callbacks(menu_item_t *item, 
                               void (*on_enter)(menu_item_t*),
                               void (*on_exit)(menu_item_t*),
                               void (*on_select)(menu_item_t*),
                               void (*on_key)(menu_item_t*, uint8_t));

/**
 * @brief 删除指定的菜单项，并释放内存
 * @param menu 要删除的菜单项指针
 * @return 0-成功，其他-失败
 */
int8_t menu_item_delete(menu_item_t *menu);

/**
 * @brief 从父菜单中移除指定的子菜单项
 * @param parent 父菜单项
 * @param child 要移除的子菜单项
 * @return 0-成功，其他-失败
 */
int8_t menu_remove_child(menu_item_t *parent, menu_item_t *child);

// ==================================
// 菜单显示API
// ==================================

/**
 * @brief 刷新菜单显示
 */
void menu_refresh_display(void);

/**
 * @brief 显示横向图标菜单
 * @param menu 菜单项
 */
void menu_display_horizontal(menu_item_t *menu);

/**
 * @brief 显示竖向列表菜单
 * @param menu 菜单项
 */
void menu_display_vertical(menu_item_t *menu);

/**
 * @brief 显示自定义页面
 * @param menu 菜单项
 */
void menu_display_custom(menu_item_t *menu);

/**
 * @brief 清屏并重绘当前菜单
 */
void menu_clear_and_redraw(void);

// ==================================
// 菜单事件处理API
// ==================================

/**
 * @brief 处理菜单事件
 * @param event 菜单事件
 * @return 0-成功，其他-失败
 */
int8_t menu_process_event(menu_event_t *event);

/**
 * @brief 按键转换为菜单事件
 * @param key 按键值
 * @return 菜单事件
 */
menu_event_t menu_key_to_event(uint8_t key);

/**
 * @brief 处理横向菜单按键事件
 * @param menu 当前菜单
 * @param key 按键值
 * @return 0-成功，其他-失败
 */
int8_t menu_handle_horizontal_key(menu_item_t *menu, uint8_t key);

/**
 * @brief 处理竖向菜单按键事件
 * @param menu 当前菜单
 * @param key 按键值
 * @return 0-成功，其他-失败
 */
int8_t menu_handle_vertical_key(menu_item_t *menu, uint8_t key);

// ==================================
// 菜单导航API
// ==================================

/**
 * @brief 进入指定菜单
 * @param menu 目标菜单
 * @return 0-成功，其他-失败
 */
int8_t menu_enter(menu_item_t *menu);

/**
 * @brief 返回父菜单
 * @return 0-成功，其他-失败
 */
int8_t menu_back_to_parent(void);

/**
 * @brief 选择下一个菜单项
 * @return 0-成功，其他-失败
 */
int8_t menu_select_next(void);

/**
 * @brief 选择上一个菜单项
 * @return 0-成功，其他-失败
 */
int8_t menu_select_previous(void);

/**
 * @brief 进入选中的菜单项
 * @return 0-成功，其他-失败
 */
int8_t menu_enter_selected(void);

// ==================================
// FreeRTOS任务API
// ==================================

/**
 * @brief 菜单处理任务
 * @param pvParameters 任务参数
 */
void menu_task(void *pvParameters);

/**
 * @brief 按键处理任务
 * @param pvParameters 任务参数
 */
void menu_key_task(void *pvParameters);

//创建自定义菜单项
#define MENU_ITEM_CUSTOM(name, draw_func, context) \
    menu_item_create(name, MENU_TYPE_CUSTOM, \
        (menu_content_t){.custom = {draw_func, context}})

// 便利宏定义
// 创建图标菜单项
// #define MENU_ITEM_ICON(name, icon_ptr, w, h) \
//     menu_item_create(name, MENU_TYPE_HORIZONTAL_ICON, \
//         (menu_content_t){.icon = {icon_ptr, w, h}})

//// 创建文本菜单项
//#define MENU_ITEM_TEXT(name, text_str, max_len) \
//    menu_item_create(name, MENU_TYPE_VERTICAL_LIST, \
//        (menu_content_t){.text = {text_str, max_len}})

// 常用布局配置预设
#define LAYOUT_HORIZONTAL_MAIN() { \
    .horizontal = { \
        .visible_count = 3, \
        .item_width = 32, \
        .item_height = 32, \
        .spacing = 16, \
        .start_x = 0, \
        .start_y = 16 \
    } \
}

#define LAYOUT_VERTICAL_TEST() { \
    .vertical = { \
        .items_per_page = 4, \
        .item_height = 16, \
        .start_x = 0, \
        .start_y = 0, \
        .indent_spaces = 2, \
        .selected_char = '>', \
        .unselected_char = ' ' \
    } \
}

#endif // __UNIFIED_MENU_H
