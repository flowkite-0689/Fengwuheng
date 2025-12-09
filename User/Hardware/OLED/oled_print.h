#ifndef __OLED_PRINT_H__
#define __OLED_PRINT_H__

#include "oled.h"
#include "stm32f10x.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "logo.h"
#include "Delay.h"
// OLED显示区域定义
#define OLED_LINE_HEIGHT 16  // 每行高度（像素）
#define OLED_MAX_LINES   4   // 最大行数（128x64像素屏幕）
#define OLED_MAX_CHARS   16  // 每行最大字符数（8x16字体）

/**
 * @brief OLED打印函数 - 在指定位置格式化打印信息
 * @param x 起始X坐标（0-127）
 * @param y 起始Y坐标（0-63）
 * @param format 格式化字符串（类似printf）
 * @param ... 可变参数
 * @note 支持%d, %u, %f, %s, %c, %x等常用格式
 */
void OLED_Printf(uint8_t x, uint8_t y, const char* format, ...);

/**
 * @brief OLED行打印函数 - 在指定行打印信息
 * @param line 行号（0-3）
 * @param format 格式化字符串
 * @param ... 可变参数
 * @note 自动计算Y坐标，X坐标为0
 */
void OLED_Printf_Line(uint8_t line, const char* format, ...);

/**
 * @brief OLED清屏指定行
 * @param line 行号（0-3）
 */
void OLED_Clear_Line(uint8_t line);

/**
 * @brief OLED显示系统信息
 * @param mode 当前模式
 * @param info 信息字符串
 */
void OLED_Display_Info(uint8_t mode, const char* info);

/**
 * @brief OLED显示传感器数据
 * @param sensor_name 传感器名称
 * @param data1 数据1
 * @param data2 数据2
 * @param unit 单位
 */
void OLED_Display_Sensor(const char* sensor_name, float data1, float data2, const char* unit);
void OLED_Printf_Line_32(uint8_t line, const char* format, ...);
void OLED_DrawProgressBar(
    uint8_t x, uint8_t y,
    uint8_t width, uint8_t height,
    int32_t value,
    int32_t min_val, int32_t max_val,
    uint8_t show_border,
    uint8_t fill_mode
,
    uint8_t point_mode);
void OLED_Clear_Rect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);


void OLED_Show_many_Tupian(const unsigned char *pic_you_want_show[],int size_pic_list,int count);
#endif // __OLED_PRINT_H__
