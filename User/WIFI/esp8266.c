/**
 * @file esp8266.c
 * @author Jaychen (719095404@qq.com)
 * @brief esp8266连接巴法云TCP设备
 * @version 0.1
 * @date 2025-12-09
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "esp8266.h"
#include <string.h>
#include <stdio.h>
#include <FreeRTOS.h>
#include <task.h>
#include "uart2.h"

extern uint8_t uart2_buffer[UART2_BUF_SIZE]; // uart2接收缓冲
extern uint8_t uart2_rx_len;     // uart2接收长度

void ESP8266_Receive_Start(void)
{
    uart2_rx_len = 0;
    memset(uart2_buffer, 0, sizeof(uart2_buffer));
}

/**
 * @brief 发送指令，并指定时间内接收指定的数据
 *
 * @param cmd  AT指令注意带\r\n
 * @param wait_string   等待字符串
 * @param timeout  超时时间ms
 * @return uint8_t 0：超时，1：成功
 */
uint8_t ESP8266_Send_AT_Cmd(const char *cmd, const char *wait_string, uint16_t timeout)
{
    uart2_rx_len = 0;
    memset(uart2_buffer, 0, sizeof(uart2_buffer));
    UART2_SendDataToWiFi_Poll((uint8_t *)cmd, strlen(cmd));    // 发送命令
    //printf("ESP8266 Send cmd: %s", cmd);
loop:
    while (uart2_rx_len <= 0 && timeout > 0)
    {
        timeout--;
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    if (uart2_rx_len > 0 && timeout > 0 && strstr((const char *)uart2_buffer, wait_string))
    {
        //printf("uart2_rx_len = %d, uart2_buf = %s\n", uart2_rx_len, uart2_buffer);
        uart2_rx_len = 0; // 清空接收缓冲,继续接收
        return 1;         // 成功
    }
    else if (timeout > 0)
    {
        uart2_rx_len = 0; // 清空接收缓冲,继续接收
        goto loop;
    }
    uart2_rx_len = 0; // 清空接收缓冲,继续接收
    return 0;
}

// 退出透传模式
uint8_t ESP8266_Exit_Transmit_Mode(void)
{
    if (ESP8266_Send_AT_Cmd("+++", NULL, 2000) != 1) // 退出透传模式
    {
        printf("ESP8266 Exit Transmit Mode , Error\r\n");
        return 0;
    }
    printf("ESP8266 Exit Transmit Mode , Success\r\n");
    return 1;
}

uint8_t ESP8266_Connect_WiFi(char *ssid, char *password)
{
    uint8_t i = 0;
    char cmd[50];                                       // 指令缓冲
    ESP8266_Exit_Transmit_Mode();                      // 退出透传模式

    while (ESP8266_Send_AT_Cmd("AT\r\n", "OK", 500) != 1) // 测试模块状态
    {
        i++;
        if (i >= 3)
        {
            printf("ESP8266 Send cmd: AT , Error\r\n");
            return 0;
        }
    }
    if (ESP8266_Send_AT_Cmd("ATE0\r\n", "OK", 500) != 1) // 关闭回显
    {
        printf("ESP8266 Send cmd: ATE0 , Error\r\n");
        return 0;
    }
    
    if (ESP8266_Send_AT_Cmd("AT+CWMODE=3\r\n", "OK", 500) != 1) // 设置为station模式
    {
        printf("ESP8266 Send cmd: AT+CWMODE=3 , Error\r\n");
        return 0;
    }

    snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, password); // 拼接指令
    if (ESP8266_Send_AT_Cmd(cmd, "OK", 8000) != 1)                           // 连接WiFi
    {
        printf("ESP8266 Send cmd: %s, Error\r\n", cmd);
        return 0;
    }
    return 1;
}

// 连接服务器bemfa.com，TCP端口8344, MQTT端口：9501，进入透传模式
uint8_t ESP8266_Connect_Server(char *ip, char *port)
{
    char cmd[50];                                            // 指令缓冲
    if (ESP8266_Send_AT_Cmd("AT+CIPMODE=1\r\n", "OK", 2000) != 1) // 设置透传模式
    {
        printf("ESP8266 Send cmd: AT+CIPMODE=1 , Error\r\n");
        return 0;
    }
    // 连接服务器和端口AT+CIPSTART="TCP","bemfa.com",8344
    snprintf(cmd, sizeof(cmd), "AT+CIPSTART=\"TCP\",\"%s\",%s\r\n", ip, port);
    if (ESP8266_Send_AT_Cmd(cmd, "OK", 5000) != 1) // 连接服务器
    {
        printf("ESP8266 Send cmd: %s, Error\r\n", cmd);
        return 0;
    }
    // 进入透传模式，后面发的都会无条件传输
    if (ESP8266_Send_AT_Cmd("AT+CIPSEND\r\n", "OK", 3000) != 1) // 进入透传模式
    {
        printf("ESP8266 Send cmd: AT+CIPSEND\r\n, Error\r\n");
        return 0;
    }
    return 1;
}

// 订阅主题
uint8_t ESP8266_TCP_Subscribe(char *uid, char *topic)
{
    char cmd[128]; // 指令缓冲
    snprintf(cmd, sizeof(cmd), "cmd=1&uid=%s&topic=%s", uid, topic);
    if (ESP8266_Send_AT_Cmd(cmd, "cmd=1&res=1", 1000) != 1) // 等待订阅成功
    {
        return 0; // 订阅失败
    }
    return 1; // 订阅成功
}

// 发布主题
uint8_t ESP8266_TCP_Publish(char *uid, char *topic, char *data)
{
    char cmd[128]; // 指令缓冲
    // cmd=2&uid=4d9ec352e0376f2110a0c601a2857225&topic=light002&msg=#32#27.80#ON#
    snprintf(cmd, sizeof(cmd), "cmd=2&uid=%s&topic=%s&msg=%s", uid, topic, data);
    if (ESP8266_Send_AT_Cmd(cmd, "cmd=2&res=1", 1000) != 1) // 等待发布成功
    {
        return 0; // 失败
    }
    return 1;
}

// 发送心跳包
uint8_t ESP8266_TCP_Heartbeat(void)
{
    if (ESP8266_Send_AT_Cmd("cmd=0&msg=ping", "cmd=0&res=1", 1000) != 1) // 等待
    {
        return 0;
    }
    return 1;
}
