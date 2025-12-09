/**
 * 用来给显示消息队列用的一些工具函数，目前来说只有17k,故不启用
 */
#ifndef _DISP_TOLS_H
#define _DISP_TOLS_H

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>
#include <string.h>
#include <stdarg.h> 
QueueHandle_t displayQueue; // 显示队列


// 显示命令类型定义
typedef enum
{
    DISPLAY_CMD_CLEAR,      // 清屏
    DISPLAY_CMD_CLEAR_LINE, // 清除指定行
    DISPLAY_CMD_PRINT_LINE, // 行打印
    DISPLAY_CMD_PRINT,      // 指定位置打印
    DISPLAY_CMD_PRINT_32,   // 32px字体行打印
    DISPLAY_CMD_REFRESH,    // 刷新显示
    DISPLAY_CMD_PICTURE,    // 显示图片
    DISPLAY_CMD_PROGRESS_BAR
} display_cmd_type_t;

// 显示消息结构体
typedef struct
{
    display_cmd_type_t cmd;
    uint8_t x;     // 左上角 X
    uint8_t y;     // 左上角 Y
    char text[32]; // 文本缓冲（PRINT 类命令用）

    union
    {
        struct
        {
            uint8_t width;
            uint8_t height;
            const unsigned char *data;
        } pic;

        struct
        {
            uint8_t width;
            uint8_t height;
            int32_t value;
            int32_t min_val;
            int32_t max_val;
            uint8_t show_border;
            uint8_t fill_mode;
        } progress;
    } extra;
} display_msg_t;




#endif
