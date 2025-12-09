/**
 * @file alarm_core.c
 * @brief 闹钟核心管理函数实现
 * @author flowkite-0689
 * @version v1.0
 * @date 2025.12.08
 */

#include "alarm_core.h"
#include <stdlib.h>
#include <stdio.h>
#include "rtc_date.h"
// ==================================
// 全局变量定义
// ==================================

Alarm_TypeDef g_alarms[MAX_ALARMS];
uint8_t g_alarm_count = 0;

// ==================================
// 闹钟管理函数实现
// ==================================

/**
 * @brief 添加闹钟
 * @param alarm 闹钟指针
 * @return 0成功，-1失败
 */
int Alarm_Add(Alarm_TypeDef *alarm)
{
    if (alarm == NULL || g_alarm_count >= MAX_ALARMS) {
        printf("Error: Cannot add alarm - full or null pointer\r\n");
        return -1;
    }
    
    // 生成唯一ID
    Alarm_GenerateId(g_alarms[g_alarm_count].id);
    
    // 复制闹钟数据
    memcpy(&g_alarms[g_alarm_count], alarm, sizeof(Alarm_TypeDef));
    
    printf("Alarm added: ID=%s, Time=%02d:%02d:%02d, Enabled=%d, Repeat=%d\r\n",
           g_alarms[g_alarm_count].id,
           alarm->hour, alarm->minute, alarm->second,
           alarm->enabled, alarm->repeat);
    
    g_alarm_count++;
    return 0;
}

/**
 * @brief 删除闹钟
 * @param index 闹钟索引
 * @return 0成功，-1失败
 */
int Alarm_Delete(uint8_t index)
{
    if (index >= g_alarm_count) {
        printf("Error: Invalid alarm index %d\r\n", index);
        return -1;
    }
    
    printf("Alarm deleted: ID=%s, Time=%02d:%02d:%02d\r\n",
           g_alarms[index].id,
           g_alarms[index].hour, g_alarms[index].minute, g_alarms[index].second);
    
    // 将后面的闹钟向前移动
    for (uint8_t i = index; i < g_alarm_count - 1; i++) {
        memcpy(&g_alarms[i], &g_alarms[i + 1], sizeof(Alarm_TypeDef));
    }
    
    g_alarm_count--;
    return 0;
}

/**
 * @brief 启用闹钟
 * @param index 闹钟索引
 * @return 0成功，-1失败
 */
int Alarm_Enable(uint8_t index)
{
    if (index >= g_alarm_count) {
        return -1;
    }
    
    g_alarms[index].enabled = 1;
    printf("Alarm enabled: ID=%s\r\n", g_alarms[index].id);
    return 0;
}

/**
 * @brief 禁用闹钟
 * @param index 闹钟索引
 * @return 0成功，-1失败
 */
int Alarm_Disable(uint8_t index)
{
    if (index >= g_alarm_count) {
        return -1;
    }
    
    g_alarms[index].enabled = 0;
    printf("Alarm disabled: ID=%s\r\n", g_alarms[index].id);
    return 0;
}

/**
 * @brief 更新闹钟
 * @param index 闹钟索引
 * @param alarm 新的闹钟数据
 * @return 0成功，-1失败
 */
int Alarm_Update(uint8_t index, Alarm_TypeDef *alarm)
{
    if (index >= g_alarm_count || alarm == NULL) {
        return -1;
    }
    
    memcpy(&g_alarms[index], alarm, sizeof(Alarm_TypeDef));
    printf("Alarm updated: ID=%s, Time=%02d:%02d:%02d\r\n",
           g_alarms[index].id,
           alarm->hour, alarm->minute, alarm->second);
    return 0;
}

/**
 * @brief 检查闹钟是否触发
 * @return 触发的闹钟索引，-1表示没有闹钟触发
 */
int Alarm_Check(void)
{
    // 读取当前RTC时间
    MyRTC_ReadTime();
    
    // 遍历所有闹钟
    for (uint8_t i = 0; i < g_alarm_count; i++) {
        Alarm_TypeDef *alarm = &g_alarms[i];
        
        // 检查闹钟是否启用
        if (!alarm->enabled) {
            continue;
        }
        
        // 检查时间是否匹配
        if (alarm->hour == RTC_data.hours && 
            alarm->minute == RTC_data.minutes && 
            alarm->second == RTC_data.seconds) {
            
            printf("Alarm triggered! Index: %d, Time: %02d:%02d:%02d\r\n", 
                   i, alarm->hour, alarm->minute, alarm->second);
            
            // 如果是单次闹钟，触发后自动禁用
            if (!alarm->repeat) {
                alarm->enabled = 0;
                printf("Single alarm disabled after trigger\r\n");
            }
            
            return (int)i;
        }
    }
    
    return -1; // 没有闹钟触发
}

/**
 * @brief 生成闹钟ID
 * @param id 输出ID缓冲区
 */
void Alarm_GenerateId(char *id)
{
    static uint16_t counter = 0;
    snprintf(id, ALARM_ID_LEN, "AL%04X", counter);
    counter = (counter + 1) % 0xFFFF;
}

/**
 * @brief 获取闹钟指针
 * @param index 闹钟索引
 * @return 闹钟指针
 */
Alarm_TypeDef* Alarm_Get(uint8_t index)
{
    if (index >= g_alarm_count) {
        return NULL;
    }
    return &g_alarms[index];
}

/**
 * @brief 获取闹钟数量
 * @return 闹钟数量
 */
uint8_t Alarm_GetCount(void)
{
    return g_alarm_count;
}
