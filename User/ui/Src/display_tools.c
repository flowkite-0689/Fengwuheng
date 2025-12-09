#include "display_tools.h"

// 显示辅助函数
void display_clear(void)
{
    display_msg_t msg = {.cmd = DISPLAY_CMD_CLEAR};
    xQueueSend(displayQueue, &msg, 0);
}

void display_clear_line(uint8_t line)
{
    display_msg_t msg = {
        .cmd = DISPLAY_CMD_CLEAR_LINE,
        .x = line};
    xQueueSend(displayQueue, &msg, 0);
}

void display_print_line(uint8_t line, const char *format, ...)
{
    display_msg_t msg = {
        .cmd = DISPLAY_CMD_PRINT_LINE,
        .x = line};

    va_list args;
    va_start(args, format);
    vsnprintf(msg.text, sizeof(msg.text), format, args);
    va_end(args);

    // 确保结尾为 \0（vsnprintf 会自动加，但防 buffer 满时截断无 \0）
    msg.text[sizeof(msg.text) - 1] = '\0';

    xQueueSend(displayQueue, &msg, 0);
}

void display_print_line_32(uint8_t line, const char *format, ...)
{
    display_msg_t msg = {
        .cmd = DISPLAY_CMD_PRINT_32,
        .x = line};

    va_list args;
    va_start(args, format);
    vsnprintf(msg.text, sizeof(msg.text), format, args);
    va_end(args);

    // 确保结尾为 \0（vsnprintf 会自动加，但防 buffer 满时截断无 \0）
    msg.text[sizeof(msg.text) - 1] = '\0';

    xQueueSend(displayQueue, &msg, 0);
}

void display_print(uint8_t x, uint8_t y, const char *text)
{
    display_msg_t msg = {
        .cmd = DISPLAY_CMD_PRINT,
        .x = x,
        .y = y};
    strncpy(msg.text, text, sizeof(msg.text) - 1);
    msg.text[sizeof(msg.text) - 1] = '\0';
    xQueueSend(displayQueue, &msg, 0);
}

void display_DrawProgressBar(
    uint8_t x, uint8_t y,
    uint8_t width, uint8_t height,
    int32_t value,
    int32_t min_val, int32_t max_val,
    uint8_t show_border,
    uint8_t fill_mode)
{
    display_msg_t msg; // 初始化为0（避免联合体垃圾数据）
  memset(&msg, 0, sizeof(msg));  
	msg.cmd = DISPLAY_CMD_PROGRESS_BAR;
    msg.x = x;
    msg.y = y;

    msg.extra.progress.width = width;
    msg.extra.progress.height = height;
    msg.extra.progress.value = value;
    msg.extra.progress.min_val = min_val;
    msg.extra.progress.max_val = max_val;
    msg.extra.progress.show_border = show_border;
    msg.extra.progress.fill_mode = fill_mode;

    xQueueSend(displayQueue, &msg, 0);
}

void display_refresh(void)
{
    display_msg_t msg = {.cmd = DISPLAY_CMD_REFRESH};
    xQueueSend(displayQueue, &msg, 0);
}
