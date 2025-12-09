/**
 * @file Game2048.c
 * @brief 2048游戏实现文件 - 支持OLED显示和MPU6050体感控制
 * @author flowkite-0689
 * @version v1.0
 * @date 2025.12.08
 */

#include "Game2048.h"
#include <stdlib.h>

// ==================================
// 静态函数声明
// ==================================
static void game2048_init_game_data(game2048_state_t *state);
static void game2048_cleanup_sensor_data(game2048_state_t *state);
static char game2048_number_to_char(int number);

// ==================================
// 页面初始化
// ==================================

/**
 * @brief 初始化2048游戏页面
 * @return 创建的2048游戏页面指针
 */
menu_item_t *game2048_init(void)
{
    // 创建自定义菜单项，不分配具体状态数据
    menu_item_t *game2048_page = MENU_ITEM_CUSTOM("2048 Game", game2048_draw_function, NULL);
    if (game2048_page == NULL) {
        return NULL;
    }
    
    // 设置回调函数
    menu_item_set_callbacks(game2048_page, 
                           game2048_on_enter, 
                           game2048_on_exit, 
                           NULL, 
                           game2048_key_handler);
    
    printf("2048 Game page created successfully\r\n");
    return game2048_page;
}

// ==================================
// 自定义绘制函数
// ==================================

/**
 * @brief 2048游戏自定义绘制函数
 * @param context 绘制上下文
 */
void game2048_draw_function(void *context)
{
    game2048_state_t *state = (game2048_state_t *)context;
    if (state == NULL) {
        // 显示错误信息
        OLED_Printf_Line(0, "2048 Game");
        OLED_Printf_Line(1, "Initializing...");
        OLED_Refresh_Dirty();
        return;
    }
    
    // 检查是否需要刷新
    TickType_t current_time = xTaskGetTickCount();
    if (state->need_refresh || (current_time - state->last_update) > pdMS_TO_TICKS(100)) {
        // 更新传感器数据（如果传感器就绪）
        if (state->sensor_ready) {
            game2048_update_sensor_data(state);
            
            // 检查体感控制
            int direction = game2048_get_move_direction(state);
            if (direction != -1 && (current_time - state->last_move) > pdMS_TO_TICKS(300)) {
                if (game2048_move(state, direction)) {
                    game2048_add_random_number(state);
                    state->last_move = current_time;
                    
                    // 检查游戏状态
                    if (game2048_is_game_over(state)) {
                        state->game_state = GAME_STATE_GAME_OVER;
                    }
                }
            }
        }
        
        // 显示游戏界面
        game2048_display_board(state);
        state->last_update = current_time;
        state->need_refresh = 0;
    }
    
    OLED_Refresh_Dirty();
}

// ==================================
// 按键处理
// ==================================

/**
 * @brief 2048游戏按键处理函数
 * @param item 菜单项
 * @param key_event 按键事件
 */
void game2048_key_handler(menu_item_t *item, uint8_t key_event)
{
    game2048_state_t *state = (game2048_state_t *)item->content.custom.draw_context;
    
    switch (key_event) {
        case MENU_EVENT_KEY_UP:
            // KEY0 - 重新开始游戏
            printf("2048: KEY0 pressed - Restart game\r\n");
            if (state) {
                game2048_init_game(state);
                state->need_refresh = 1;
            }
            break;
            
        case MENU_EVENT_KEY_DOWN:
            // KEY1 - 切换显示模式
            printf("2048: KEY1 pressed - Toggle mode\r\n");
            if (state) {
                state->display_mode = (state->display_mode + 1) % DISPLAY_MODE_MAX;
                state->need_refresh = 1;
            }
            break;
            
        case MENU_EVENT_KEY_SELECT:
            // KEY2 - 返回上一级
            printf("2048: KEY2 pressed - Return\r\n");
            menu_back_to_parent();
            break;
            
        case MENU_EVENT_KEY_ENTER:
            // KEY3 - 初始化传感器或方向控制
            printf("2048: KEY3 pressed - Initialize sensor\r\n");
            if (state && !state->sensor_ready) {
                if (MPU_Init() == 0) {
                    state->sensor_ready = 1;
                    printf("MPU6050 initialized successfully\r\n");
                    state->need_refresh = 1;
                } else {
                    printf("MPU6050 initialization failed\r\n");
                    OLED_Printf_Line(1,"MPU605");
                    OLED_Printf_Line(2,"nitialization failed");
                    OLED_Printf_Line(3,"check uart");
                    OLED_Refresh_Dirty();
                }
            }
            break;
            
        case MENU_EVENT_REFRESH:
            // 刷新显示
            if (state) {
                state->need_refresh = 1;
            }
            break;
            
        default:
            break;
    }
}

// ==================================
// 回调函数
// ==================================

/**
 * @brief 2048游戏页面进入回调
 * @param item 菜单项
 */
void game2048_on_enter(menu_item_t *item)
{
    printf("Enter 2048 Game page\r\n");
    
    // 分配状态数据结构
    game2048_state_t *state = (game2048_state_t *)pvPortMalloc(sizeof(game2048_state_t));
    if (state == NULL) {
        printf("Error: Failed to allocate 2048 game state memory!\r\n");
        return;
    }
    
    printf("MALLOC: game2048_on_enter, state addr=%p, size=%d bytes\r\n", 
           state, sizeof(game2048_state_t));
    
    // 初始化状态数据
    game2048_init_game_data(state);
    
    // 设置到菜单项上下文
    item->content.custom.draw_context = state;
    
    // 清屏并标记需要刷新
    OLED_Clear();
    state->need_refresh = 1;
}

/**
 * @brief 2048游戏页面退出回调
 * @param item 菜单项
 */
void game2048_on_exit(menu_item_t *item)
{
    printf("Exit 2048 Game page\r\n");
    
    game2048_state_t *state = (game2048_state_t *)item->content.custom.draw_context;
    if (state == NULL) {
        return;
    }
    
    // 清理传感器相关数据
    game2048_cleanup_sensor_data(state);
    
    // 释放状态结构体本身
    printf("FREE: game2048_on_exit, state addr=%p, size=%d bytes\r\n", 
           state, sizeof(game2048_state_t));
    vPortFree(state);
    
    // 清空指针，防止野指针
    item->content.custom.draw_context = NULL;
    
    // 清屏
    OLED_Clear();
}

// ==================================
// 游戏数据初始化与清理
// ==================================

/**
 * @brief 初始化游戏状态数据
 * @param state 游戏状态指针
 */
static void game2048_init_game_data(game2048_state_t *state)
{
    if (state == NULL) {
        return;
    }
    
    // 清零状态结构体
    memset(state, 0, sizeof(game2048_state_t));
    
    // 初始化状态
    state->need_refresh = 1;
    state->last_update = xTaskGetTickCount();
    state->last_move = xTaskGetTickCount();
    state->sensor_ready = 0;
    state->display_mode = DISPLAY_MODE_NORMAL;
    state->game_state = GAME_STATE_PLAYING;
    
    // 初始化文本缓冲区
    strcpy(state->direction_text, "---");
    strcpy(state->angle_text, "---");
    
    // 初始化游戏数据
    game2048_init_game(state);
    
    printf("2048 Game state initialized\r\n");
}

/**
 * @brief 清理传感器数据
 * @param state 游戏状态指针
 */
static void game2048_cleanup_sensor_data(game2048_state_t *state)
{
    if (state == NULL) {
        return;
    }
    
    
    
    printf("2048 Game sensor data cleaned up\r\n");
}

// ==================================
// 游戏逻辑函数
// ==================================

/**
 * @brief 初始化游戏
 * @param state 游戏状态
 */
void game2048_init_game(game2048_state_t *state)
{
    if (state == NULL) {
        return;
    }
    
    // 清空棋盘
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            state->board[i][j] = 0;
        }
    }
    
    // 重置分数和状态
    state->score = 0;
    state->game_state = GAME_STATE_PLAYING;
    
    // 添加两个初始数字
    game2048_add_random_number(state);
    game2048_add_random_number(state);
}

/**
 * @brief 在随机空位添加数字
 * @param state 游戏状态
 */
void game2048_add_random_number(game2048_state_t *state)
{
    if (state == NULL) {
        return;
    }
    
    int empty[BOARD_SIZE * BOARD_SIZE][2];
    int count = 0;
    
    // 找到所有空位置
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (state->board[i][j] == 0) {
                empty[count][0] = i;
                empty[count][1] = j;
                count++;
            }
        }
    }
    
    // 如果有空位置，随机添加数字
    if (count > 0) {
        int index = rand() % count;
        int num = (rand() % 10 < 9) ? 2 : 4; // 90%概率2，10%概率4
        state->board[empty[index][0]][empty[index][1]] = num;
    }
}

/**
 * @brief 移动并合并数字
 * @param state 游戏状态
 * @param direction 移动方向：0-左，1-右，2-上，3-下
 * @return 是否发生了移动
 */
int game2048_move(game2048_state_t *state, int direction)
{
    if (state == NULL) {
        return 0;
    }
    
    int moved = 0;
    int merged[BOARD_SIZE * BOARD_SIZE / 2][2];
    int merge_count = 0;
    
    if (direction == 0) { // 左移
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 1; j < BOARD_SIZE; j++) {
                if (state->board[i][j] != 0) {
                    int k = j;
                    while (k > 0 && state->board[i][k - 1] == 0) {
                        state->board[i][k - 1] = state->board[i][k];
                        state->board[i][k] = 0;
                        k--;
                        moved = 1;
                    }
                    if (k > 0 && state->board[i][k - 1] == state->board[i][k]) {
                        int already_merged = 0;
                        for (int z = 0; z < merge_count; z++) {
                            if (merged[z][0] == i && merged[z][1] == (k - 1)) {
                                already_merged = 1;
                            }
                        }
                        if (!already_merged) {
                            state->board[i][k - 1] *= 2;
                            state->score += state->board[i][k - 1];
                            merged[merge_count][0] = i;
                            merged[merge_count][1] = k - 1;
                            merge_count++;
                            state->board[i][k] = 0;
                            moved = 1;
                        }
                    }
                }
            }
        }
    }
    else if (direction == 1) { // 右移
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = BOARD_SIZE - 2; j >= 0; j--) {
                if (state->board[i][j] != 0) {
                    int k = j;
                    while (k < BOARD_SIZE - 1 && state->board[i][k + 1] == 0) {
                        state->board[i][k + 1] = state->board[i][k];
                        state->board[i][k] = 0;
                        k++;
                        moved = 1;
                    }
                    if (k < BOARD_SIZE - 1 && state->board[i][k + 1] == state->board[i][k]) {
                        int already_merged = 0;
                        for (int z = 0; z < merge_count; z++) {
                            if (merged[z][0] == i && merged[z][1] == (k + 1)) {
                                already_merged = 1;
                            }
                        }
                        if (!already_merged) {
                            state->board[i][k + 1] *= 2;
                            state->score += state->board[i][k + 1];
                            merged[merge_count][0] = i;
                            merged[merge_count][1] = k + 1;
                            merge_count++;
                            state->board[i][k] = 0;
                            moved = 1;
                        }
                    }
                }
            }
        }
    }
    else if (direction == 2) { // 上移
        for (int j = 0; j < BOARD_SIZE; j++) {
            for (int i = 1; i < BOARD_SIZE; i++) {
                if (state->board[i][j] != 0) {
                    int k = i;
                    while (k > 0 && state->board[k - 1][j] == 0) {
                        state->board[k - 1][j] = state->board[k][j];
                        state->board[k][j] = 0;
                        k--;
                        moved = 1;
                    }
                    if (k > 0 && state->board[k - 1][j] == state->board[k][j]) {
                        int already_merged = 0;
                        for (int z = 0; z < merge_count; z++) {
                            if (merged[z][0] == k - 1 && merged[z][1] == j) {
                                already_merged = 1;
                            }
                        }
                        if (!already_merged) {
                            state->board[k - 1][j] *= 2;
                            state->score += state->board[k - 1][j];
                            merged[merge_count][0] = k - 1;
                            merged[merge_count][1] = j;
                            merge_count++;
                            state->board[k][j] = 0;
                            moved = 1;
                        }
                    }
                }
            }
        }
    }
    else if (direction == 3) { // 下移
        for (int j = 0; j < BOARD_SIZE; j++) {
            for (int i = BOARD_SIZE - 2; i >= 0; i--) {
                if (state->board[i][j] != 0) {
                    int k = i;
                    while (k < BOARD_SIZE - 1 && state->board[k + 1][j] == 0) {
                        state->board[k + 1][j] = state->board[k][j];
                        state->board[k][j] = 0;
                        k++;
                        moved = 1;
                    }
                    if (k < BOARD_SIZE - 1 && state->board[k + 1][j] == state->board[k][j]) {
                        int already_merged = 0;
                        for (int z = 0; z < merge_count; z++) {
                            if (merged[z][0] == k + 1 && merged[z][1] == j) {
                                already_merged = 1;
                            }
                        }
                        if (!already_merged) {
                            state->board[k + 1][j] *= 2;
                            state->score += state->board[k + 1][j];
                            merged[merge_count][0] = k + 1;
                            merged[merge_count][1] = j;
                            merge_count++;
                            state->board[k][j] = 0;
                            moved = 1;
                        }
                    }
                }
            }
        }
    }
    
    return moved;
}

/**
 * @brief 检查游戏是否结束
 * @param state 游戏状态
 * @return 游戏是否结束
 */
int game2048_is_game_over(game2048_state_t *state)
{
    if (state == NULL) {
        return 1;
    }
    
    // 检查是否达到2048
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (state->board[i][j] == 2048) {
                state->game_state = GAME_STATE_WIN;
                return 1;
            }
        }
    }
    
    // 检查是否有空位
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (state->board[i][j] == 0) {
                return 0;
            }
        }
    }
    
    // 检查是否可以合并
    for (int i = 0; i < BOARD_SIZE - 1; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (state->board[i + 1][j] == state->board[i][j]) {
                return 0;
            }
        }
    }
    
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE - 1; j++) {
            if (state->board[i][j + 1] == state->board[i][j]) {
                return 0;
            }
        }
    }
    
    return 1;
}

// ==================================
// 传感器数据处理
// ==================================

/**
 * @brief 计算倾斜角度
 * @param ax X轴加速度
 * @param ay Y轴加速度
 * @param az Z轴加速度
 * @param angle_x 输出X轴角度
 * @param angle_y 输出Y轴角度
 */
void game2048_calculate_tilt_angles(short ax, short ay, short az, float *angle_x, float *angle_y)
{
    // 使用双精度浮点数计算以提高精度
    double ax_d = (double)ax;
    double ay_d = (double)ay;
    double az_d = (double)az;
    
    *angle_x = atan2(ax_d, sqrt(ay_d * ay_d + az_d * az_d)) * 180.0 / 3.14159265;
    *angle_y = atan2(ay_d, sqrt(ax_d * ax_d + az_d * az_d)) * 180.0 / 3.14159265;
}

/**
 * @brief 更新传感器数据
 * @param state 游戏状态
 */
void game2048_update_sensor_data(game2048_state_t *state)
{
    if (!state || !state->sensor_ready) {
        return;
    }
    
    short ax, ay, az;
    
    if (MPU_Get_Accelerometer(&ax, &ay, &az) == 0) {
        // 计算原始角度
        game2048_calculate_tilt_angles(ax, ay, az, &state->angle_x, &state->angle_y);
        
        // 平滑处理角度数据
        const float smoothing_factor = 0.3f;
        state->last_angle_x = state->last_angle_x * (1 - smoothing_factor) + state->angle_x * smoothing_factor;
        state->last_angle_y = state->last_angle_y * (1 - smoothing_factor) + state->angle_y * smoothing_factor;
        
        // 确定方向文本
        const float trigger_threshold = 20.0f;
        const float trend_threshold = 5.0f;
        
        if (fabsf(state->last_angle_x) > fabsf(state->last_angle_y)) {
            // X轴为主要倾斜方向
            if (state->last_angle_x < -trigger_threshold) {
                strcpy(state->direction_text, "right   ");
            } else if (state->last_angle_x > trigger_threshold) {
                strcpy(state->direction_text, "left ");
            } else if (state->last_angle_x < -trend_threshold) {
                strcpy(state->direction_text, "right~   ");
            } else if (state->last_angle_x > trend_threshold) {
                strcpy(state->direction_text, "left~");
            } else {
                strcpy(state->direction_text, "flat ");
            }
            snprintf(state->angle_text, 16, " %.1f^     ", state->last_angle_x);
        } else {
            // Y轴为主要倾斜方向
            if (state->last_angle_y > trigger_threshold) {
                strcpy(state->direction_text, "down");
            } else if (state->last_angle_y < -trigger_threshold) {
                strcpy(state->direction_text, "up  ");
            } else if (state->last_angle_y > trend_threshold) {
                strcpy(state->direction_text, "down~");
            } else if (state->last_angle_y < -trend_threshold) {
                strcpy(state->direction_text, "up~  ");
            } else {
                strcpy(state->direction_text, "flat   ");
            }
            snprintf(state->angle_text, 16, " %.1f^     ", state->last_angle_y);
        }
    }
}

/**
 * @brief 获取移动方向
 * @param state 游戏状态
 * @return 移动方向：-1表示无有效移动
 */
int game2048_get_move_direction(game2048_state_t *state)
{
    if (!state || !state->sensor_ready) {
        return -1;
    }
    
    
    static int need_reset = 0;
    int current_direction = -1;
    
    const float trigger_threshold = 25.0f;
    const float reset_threshold = 15.0f;
    
    if (fabsf(state->last_angle_x) > fabsf(state->last_angle_y)) {
        if (state->last_angle_x < -trigger_threshold) {
            current_direction = 1; // 上
        } else if (state->last_angle_x > trigger_threshold) {
            current_direction = 0; // 下
        }
    } else {
        if (state->last_angle_y > trigger_threshold) {
            current_direction = 3; // 右
        } else if (state->last_angle_y < -trigger_threshold) {
            current_direction = 2; // 左
        }
    }
    
    // 检查是否需要重置
    if (need_reset) {
        if ((state->last_angle_x < reset_threshold && state->last_angle_x > -reset_threshold) && 
            (state->last_angle_y < reset_threshold && state->last_angle_y > -reset_threshold)) {
            need_reset = 0;
            
        } else {
            return -1;
        }
    }
    
    if (current_direction != -1 && !need_reset) {
        need_reset = 1;
        return current_direction;
    }
    
    return -1;
}

// ==================================
// 显示函数
// ==================================

/**
 * @brief 数字转换为字符
 * @param number 数字
 * @return 对应的字符
 */
static char game2048_number_to_char(int number)
{
    if (number < 10) {
        return number + '0';
    } else if (number == 16) {
        return 'A';
    } else if (number == 32) {
        return 'B';
    } else if (number == 64) {
        return 'C';
    } else if (number == 128) {
        return 'D';
    } else if (number == 256) {
        return 'E';
    } else if (number == 512) {
        return 'F';
    } else if (number == 1024) {
        return 'G';
    } else if (number == 2048) {
        return 'H';
    } else {
        return '.';
    }
}

/**
 * @brief 显示游戏棋盘
 * @param state 游戏状态
 */
void game2048_display_board(game2048_state_t *state)
{
    if (!state) {
        return;
    }
    
    if (!state->sensor_ready) {
        // 显示初始化提示
        OLED_Printf_Line(0, "2048 Game");
        OLED_Printf_Line(1, "Press KEY3 to");
        OLED_Printf_Line(2, "initialize");
        OLED_Printf_Line(3, "MPU6050 sensor");
        return;
    }
    
    // 根据游戏状态显示不同内容
    if (state->game_state == GAME_STATE_GAME_OVER) {
        OLED_Printf_Line(0, "Game Over!");
        OLED_Printf_Line(1, "Score: %d", state->score);
        OLED_Printf_Line(2, "Press KEY0 to");
        OLED_Printf_Line(3, "restart game");
        return;
    } else if (state->game_state == GAME_STATE_WIN) {
        OLED_Printf_Line(0, "You Win!");
        OLED_Printf_Line(1, "Score: %d", state->score);
        OLED_Printf_Line(2, "Press KEY0 to");
        OLED_Printf_Line(3, "play again");
        return;
    }
    
    // 正常游戏显示
    for (int i = 0; i < BOARD_SIZE; i++) {
        char c[BOARD_SIZE];
        for (int j = 0; j < BOARD_SIZE; j++) {
            c[j] = game2048_number_to_char(state->board[i][j]);
        }
        
        if (i == 0) {
            OLED_Printf_Line(i, "%c  %c  %c  %c sc:%d",
                           c[0], c[1], c[2], c[3], state->score);
        } else {
            OLED_Printf_Line(i, "%c  %c  %c  %c %s",
                           c[0], c[1], c[2], c[3], 
                           (i-1 < 2) ? ((i-1 == 0) ? state->direction_text : state->angle_text) : "");
        }
    }
}
