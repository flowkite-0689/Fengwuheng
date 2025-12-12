#ifndef ESP8266_H
#define ESP8266_H


//Name : ElevatedNetwork.lt
//password : 798798798

#include "uart2.h"
#include "stm32f10x.h"
#include <stdint.h>
#include "sensordata.h"
extern uint8_t uart2_buffer[UART2_BUF_SIZE]; // uart2接收缓冲
extern uint8_t uart2_rx_len;     // uart2接收长度

extern uint8_t wifi_connected;
extern uint8_t Server_connected;

void ESP8266_Receive_Start(void);
uint8_t ESP8266_Connect_WiFi(char *ssid, char *password);
uint8_t ESP8266_Connect_Server(char *ip, char *port);
uint8_t ESP8266_TCP_Subscribe(char *uid, char *topic);
uint8_t ESP8266_TCP_Publish(char *uid, char *topic, char *data);
uint8_t ESP8266_TCP_Heartbeat(void);
uint8_t ESP8266_TCP_GetTime(char *uid, char *time_buffer, uint16_t buffer_size);

// 新增消息解析函数
uint8_t ESP8266_Parse_Command(const char *buffer, const char *topic, char *msg_value);
uint8_t ESP8266_Process_Sensor_Commands(const char *buffer);
#endif 
