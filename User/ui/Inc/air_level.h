/**
 * @file air_level.h
 * @brief 水平仪页面头文件
 * @author flowkite-0689
 * @version v1.0
 * @date 2025.12.06
 */

#ifndef __AIR_LEVEL_H
#define __AIR_LEVEL_H

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "unified_menu.h"
#include "oled_print.h"
#include "Key.h"
#include "MPU6050_hardware_i2c.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// ==================================
// 宏定义
// ==================================

#define AIR_LEVEL_DIRECTION_TEXT_LEN 16
#define AIR_LEVEL_ANGLE_TEXT_LEN     16

// 显示模式
typedef enum {
    DISPLAY_MODE_DETAIL = 0,    // 详细模式
    DISPLAY_MODE_SIMPLE = 1,    // 简洁模式
    DISPLAY_MODE_MAX
} air_display_mode_t;

// ==================================
// 水平仪状态结构体
// ==================================

typedef struct air_level_state {
    // 传感器数据
    float angle_x;           // X轴角度（前后倾斜）
    float angle_y;           // Y轴角度（左右倾斜）
    float last_angle_x;      // 上次X轴角度（用于平滑显示）
    float last_angle_y;      // 上次Y轴角度（用于平滑显示）
    
    // 显示控制
    uint8_t need_refresh;    // 需要刷新标志
    uint8_t sensor_ready;    // 传感器就绪状态
    uint8_t display_mode;    // 显示模式
    uint8_t _reserved;       // 保留字节，用于对齐
    
    TickType_t last_update;  // 上次更新时间
    
    // 信息显示
    char direction_text[AIR_LEVEL_DIRECTION_TEXT_LEN]; // 方向文本
    char angle_text[AIR_LEVEL_ANGLE_TEXT_LEN];         // 角度文本
} air_level_state_t;

// ==================================
// 函数声明
// ==================================

// 初始化与销毁函数
menu_item_t* air_level_init(void);
void air_level_deinit(menu_item_t* item);

// 回调函数
void air_level_on_enter(menu_item_t* item);
void air_level_on_exit(menu_item_t* item);
void air_level_key_handler(menu_item_t* item, uint8_t key_event);
void air_level_draw_function(void* context);

// 数据处理函数
void air_level_update_data(air_level_state_t* state);
void calculate_tilt_angles(short ax, short ay, short az, 
                          float* angle_x, float* angle_y);

// 显示函数
void air_level_display_info(air_level_state_t* state);

// 工具函数（如果需要的话）
static inline uint8_t air_level_is_sensor_ready(air_level_state_t* state)
{
    return (state != NULL) ? state->sensor_ready : 0;
}

static inline void air_level_set_refresh(air_level_state_t* state, uint8_t refresh)
{
    if (state) {
        state->need_refresh = refresh;
    }
}

#endif // __AIR_LEVEL_H
