#include "air_level.h"
#include <stdlib.h>  // 添加stdlib.h用于动态内存管理

// ==================================
// 静态函数声明
// ==================================
static void air_level_display_info_wrapper(void *context);
static void air_level_init_sensor_data(air_level_state_t *state);
static void air_level_cleanup_sensor_data(air_level_state_t *state);

// ==================================
// 页面初始化
// ==================================

/**
 * @brief 初始化水平仪页面
 * @return 创建的水平仪页面指针
 */
menu_item_t *air_level_init(void)
{
    // 创建自定义菜单项，不分配具体状态数据
    menu_item_t *air_level_page = MENU_ITEM_CUSTOM("Air Level", air_level_draw_function, NULL);
    if (air_level_page == NULL) {
        return NULL;
    }
    
    // 设置回调函数
    menu_item_set_callbacks(air_level_page, 
                           air_level_on_enter, 
                           air_level_on_exit, 
                           NULL, 
                           air_level_key_handler);
    
    printf("Air Level page created successfully\r\n");
    return air_level_page;
}

// ==================================
// 自定义绘制函数
// ==================================

/**
 * @brief 水平仪自定义绘制函数
 * @param context 绘制上下文
 */
void air_level_draw_function(void *context)
{
    air_level_state_t *state = (air_level_state_t *)context;
    if (state == NULL || !state->sensor_ready) {
        // 显示错误信息
        OLED_Printf_Line(0, "Air Level");
        OLED_Printf_Line(1, "Press KEY3 to");
        OLED_Printf_Line(2, "initialize");
        OLED_Printf_Line(3, "sensor");
        OLED_Refresh_Dirty();
        return;
    }
    
    // 更新传感器数据
    air_level_update_data(state);
    
    // 显示水平仪信息
    air_level_display_info(state);
    
    OLED_Refresh_Dirty();
}

// ==================================
// 按键处理
// ==================================

/**
 * @brief 水平仪按键处理函数
 * @param item 菜单项
 * @param key_event 按键事件
 */
void air_level_key_handler(menu_item_t *item, uint8_t key_event)
{
    air_level_state_t *state = (air_level_state_t *)item->content.custom.draw_context;
    
    switch (key_event) {
        case MENU_EVENT_KEY_UP:
            // KEY0 - 可以用来切换显示模式
            printf("Air Level: KEY0 pressed - Toggle mode\r\n");
            if (state) {
                state->display_mode = (state->display_mode + 1) % 2; // 假设有2种显示模式
                state->need_refresh = 1;
            }
            break;
            
        case MENU_EVENT_KEY_DOWN:
            // KEY1 - 可以用来复位角度
            printf("Air Level: KEY1 pressed - Reset angles\r\n");
            if (state) {
                state->last_angle_x = 0;
                state->last_angle_y = 0;
                state->need_refresh = 1;
            }
            break;
            
        case MENU_EVENT_KEY_SELECT:
            // KEY2 - 返回上一级
            printf("Air Level: KEY2 pressed - Return\r\n");
            menu_back_to_parent();
            break;
            
        case MENU_EVENT_KEY_ENTER:
            // KEY3 - 初始化传感器
            printf("Air Level: KEY3 pressed - Initialize sensor\r\n");
            if (state && !state->sensor_ready) {
                if (MPU_Init() == 0) {
                    state->sensor_ready = 1;
                    printf("MPU6050 initialized successfully\r\n");
                    state->need_refresh = 1;
                } else {
                    printf("MPU6050 initialization failed\r\n");
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
 * @brief 水平仪页面进入回调
 * @param item 菜单项
 */
void air_level_on_enter(menu_item_t *item)
{
    printf("Enter Air Level page\r\n");
    
    // 分配状态数据结构
    air_level_state_t *state = (air_level_state_t *)pvPortMalloc(sizeof(air_level_state_t));
    if (state == NULL) {
        printf("Error: Failed to allocate air level state memory!\r\n");
        return;
    }
    
    printf("MALLOC: air_level_on_enter, state addr=%p, size=%d bytes\r\n", 
           state, sizeof(air_level_state_t));
    
    // 初始化状态数据
    air_level_init_sensor_data(state);
    
    // 设置到菜单项上下文
    item->content.custom.draw_context = state;
    
    // 可以在这里分配其他动态数据，比如校准数据、历史记录等
    // state->calibration_data = (calibration_t *)pvPortMalloc(sizeof(calibration_t));
    
    // 清屏并标记需要刷新
    OLED_Clear();
    state->need_refresh = 1;
}

/**
 * @brief 水平仪页面退出回调
 * @param item 菜单项
 */
void air_level_on_exit(menu_item_t *item)
{
    printf("Exit Air Level page\r\n");
    
    air_level_state_t *state = (air_level_state_t *)item->content.custom.draw_context;
    if (state == NULL) {
        return;
    }
    
    // 清理传感器相关数据
    air_level_cleanup_sensor_data(state);
    
    // 释放其他动态分配的数据
    // if (state->calibration_data) {
    //     vPortFree(state->calibration_data);
    // }
    
    // 释放状态结构体本身
    printf("FREE: air_level_on_exit, state addr=%p, size=%d bytes\r\n", 
           state, sizeof(air_level_state_t));
    vPortFree(state);
    
    // 清空指针，防止野指针
    item->content.custom.draw_context = NULL;
    
    // 清屏
    OLED_Clear();
}

// ==================================
// 传感器数据初始化与清理
// ==================================

/**
 * @brief 初始化传感器状态数据
 * @param state 传感器状态指针
 */
static void air_level_init_sensor_data(air_level_state_t *state)
{
    if (state == NULL) {
        return;
    }
    
    // 清零状态结构体
    memset(state, 0, sizeof(air_level_state_t));
    
    // 初始化状态
    state->need_refresh = 1;
    state->last_update = xTaskGetTickCount();
    state->sensor_ready = 0;
    state->display_mode = 0; // 默认显示模式
    
    // 初始化文本缓冲区
    strcpy(state->direction_text, "---");
    strcpy(state->angle_text, "---");
    
    // 初始化角度数据
    state->angle_x = 0.0f;
    state->angle_y = 0.0f;
    state->last_angle_x = 0.0f;
    state->last_angle_y = 0.0f;
    
    printf("Air Level state initialized\r\n");
}

/**
 * @brief 清理传感器数据
 * @param state 传感器状态指针
 */
static void air_level_cleanup_sensor_data(air_level_state_t *state)
{
    if (state == NULL) {
        return;
    }
    
    // 如果有需要关闭的传感器，在这里执行
    // 例如：MPU_Deinit();
    
    printf("Air Level sensor data cleaned up\r\n");
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
void calculate_tilt_angles(short ax, short ay, short az, float *angle_x, float *angle_y)
{
    // 使用双精度浮点数计算以提高精度
    double ax_d = (double)ax;
    double ay_d = (double)ay;
    double az_d = (double)az;
    
    *angle_x = atan2(ax_d, sqrt(ay_d * ay_d + az_d * az_d)) * 180.0 / 3.14159265;
    *angle_y = atan2(ay_d, sqrt(ax_d * ax_d + az_d * az_d)) * 180.0 / 3.14159265;
}

/**
 * @brief 获取当前倾斜方向和角度
 * @param state 水平仪状态
 */
void air_level_update_data(air_level_state_t *state)
{
    if (!state || !state->sensor_ready) {
        return;
    }
    
    short ax, ay, az;
    
    if (MPU_Get_Accelerometer(&ax, &ay, &az) == 0) {
        // 计算原始角度
        calculate_tilt_angles(ax, ay, az, &state->angle_x, &state->angle_y);
        
        // 平滑处理角度数据（避免跳动）
        const float smoothing_factor = 0.3f;
        state->last_angle_x = state->last_angle_y * (1 - smoothing_factor) + state->angle_y * smoothing_factor;
        state->last_angle_y = state->last_angle_x * (1 - smoothing_factor) + state->angle_x * smoothing_factor;
        
        // 确定方向文本
        const float trigger_threshold = 20.0f;
        const float trend_threshold = 5.0f;
        
        if (fabsf(state->last_angle_x) > fabsf(state->last_angle_y)) {
            // X轴为主要倾斜方向
            if (state->last_angle_x < -trigger_threshold) {
                strcpy(state->direction_text, "up   ");
            } else if (state->last_angle_x > trigger_threshold) {
                strcpy(state->direction_text, "down ");
            } else if (state->last_angle_x < -trend_threshold) {
                strcpy(state->direction_text, "up~   ");
            } else if (state->last_angle_x > trend_threshold) {
                strcpy(state->direction_text, "down~");
            } else {
                strcpy(state->direction_text, "flat ");
            }
            snprintf(state->angle_text, 16, " %.1f^     ", state->last_angle_x);
        } else {
            // Y轴为主要倾斜方向
            if (state->last_angle_y < -trigger_threshold) {
                strcpy(state->direction_text, "right");
            } else if (state->last_angle_y > trigger_threshold) {
                strcpy(state->direction_text, "left  ");
            } else if (state->last_angle_y < -trend_threshold) {
                strcpy(state->direction_text, "right~");
            } else if (state->last_angle_y > trend_threshold) {
                strcpy(state->direction_text, "left~  ");
            } else {
                strcpy(state->direction_text, "flat   ");
            }
            snprintf(state->angle_text, 16, " %.1f^     ", state->last_angle_y);
        }
        
        // 标记需要刷新
        state->need_refresh = 1;
    }
}

// ==================================
// 显示函数
// ==================================

/**
 * @brief 显示水平仪信息
 * @param state 水平仪状态
 */
void air_level_display_info(air_level_state_t *state)
{
    if (!state) {
        return;
    }
    
    if (!state->sensor_ready) {
        OLED_Printf_Line(0, "Air Level");
        OLED_Printf_Line(1, "Press KEY3 to");
        OLED_Printf_Line(2, "initialize");
        OLED_Printf_Line(3, "MPU6050 sensor");
        return;
    }
    
    // 根据显示模式切换显示内容
    if (state->display_mode == 0) {
        // 模式0：详细信息
        OLED_Printf_Line(0, "Direction: %s", state->direction_text);
        OLED_Printf_Line(1, "Angle: %s", state->angle_text);
        OLED_Printf_Line(2, "X: %0.1f^", state->last_angle_x);
        OLED_Printf_Line(3, "Y: %0.1f^", state->last_angle_y);
    } else {
        // 模式1：简洁模式
        OLED_Printf_Line(0, "Air Level");
        OLED_Printf_Line(1, "Dir: %s", state->direction_text);
        OLED_Printf_Line(2, "X:%0.1f^ Y:%0.1f^", state->last_angle_x, state->last_angle_y);
        OLED_Printf_Line(3, "KEY0:Mode KEY1:Reset");
    }
    
    // 绘制水平指示器（仅在传感器就绪时）
    // 垂直进度条显示Y轴倾斜（左右倾斜）
    int y_angle_scaled = (int)(state->last_angle_y * 10);
    OLED_DrawProgressBar(96, 0, 2, 64, -y_angle_scaled, -900, 900, 0, 1, 0);
    
    // 水平进度条显示X轴倾斜（前后倾斜）
    int x_angle_scaled = (int)(state->last_angle_x * 10);
    OLED_DrawProgressBar(64, 32, 64, 2, -x_angle_scaled, -900, 900, 0, 1, 0);
}

/**
 * @brief 静态显示函数包装器（供内部使用）
 */
static void air_level_display_info_wrapper(void *context)
{
    air_level_display_info((air_level_state_t *)context);
}

