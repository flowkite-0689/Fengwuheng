/**
 * @file Game2048.h
 * @brief 2048游戏页面头文件
 * @author flowkite-0689
 * @version v1.0
 * @date 2025.12.08
 */

#ifndef __GAME2048_H
#define __GAME2048_H

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "unified_menu.h"
#include "oled_print.h"
#include "Key.h"
#include "MPU6050.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// ==================================
// 宏定义
// ==================================

#define BOARD_SIZE 4
#define GAME2048_DIRECTION_TEXT_LEN 16
#define GAME2048_ANGLE_TEXT_LEN     16

// 游戏状态
typedef enum {
    GAME_STATE_PLAYING = 0,    // 游戏中
    GAME_STATE_GAME_OVER = 1,  // 游戏结束
    GAME_STATE_WIN = 2         // 获胜
} game_state_t;

// 显示模式
// typedef enum {
//     DISPLAY_MODE_NORMAL = 0,    // 正常模式
//     DISPLAY_MODE_SIMPLE = 1,    // 简洁模式
//     DISPLAY_MODE_MAX
// } game_display_mode_t;

// ==================================
// 2048游戏状态结构体
// ==================================

typedef struct game2048_state {
    // 游戏数据
    int board[BOARD_SIZE][BOARD_SIZE];  // 游戏棋盘
    int score;                         // 游戏得分
    game_state_t game_state;           // 游戏状态
    
    // 传感器数据
    float angle_x;           // X轴角度（前后倾斜）
    float angle_y;           // Y轴角度（左右倾斜）
    float last_angle_x;      // 上次X轴角度（用于平滑显示）
    float last_angle_y;      // 上次Y轴角度（用于平滑显示）
    
    // 显示控制
    uint8_t need_refresh;    // 需要刷新标志
    uint8_t sensor_ready;    // 传感器就绪状态
    // uint8_t display_mode;    // 显示模式
    uint8_t _reserved;       // 保留字节，用于对齐
    
    TickType_t last_update;  // 上次更新时间
    TickType_t last_move;    // 上次移动时间
    
    // 信息显示
    char direction_text[GAME2048_DIRECTION_TEXT_LEN]; // 方向文本
    char angle_text[GAME2048_ANGLE_TEXT_LEN];       // 角度文本
} game2048_state_t;

// ==================================
// 函数声明
// ==================================

// 初始化与销毁函数
menu_item_t* game2048_init(void);
void game2048_deinit(menu_item_t* item);

// 回调函数
void game2048_on_enter(menu_item_t* item);
void game2048_on_exit(menu_item_t* item);
void game2048_key_handler(menu_item_t* item, uint8_t key_event);
void game2048_draw_function(void* context);

// 游戏逻辑函数
void game2048_init_game(game2048_state_t* state);
void game2048_add_random_number(game2048_state_t* state);
int game2048_move(game2048_state_t* state, int direction);
int game2048_is_game_over(game2048_state_t* state);

// 传感器数据处理函数
void game2048_update_sensor_data(game2048_state_t* state);
void game2048_calculate_tilt_angles(short ax, short ay, short az, float* angle_x, float* angle_y);
int game2048_get_move_direction(game2048_state_t* state);

// 显示函数
void game2048_display_board(game2048_state_t* state);

// 工具函数
static inline uint8_t game2048_is_sensor_ready(game2048_state_t* state)
{
    return (state != NULL) ? state->sensor_ready : 0;
}

static inline void game2048_set_refresh(game2048_state_t* state, uint8_t refresh)
{
    if (state) {
        state->need_refresh = refresh;
    }
}

#endif // __GAME2048_H
