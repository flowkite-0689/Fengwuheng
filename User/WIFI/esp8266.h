#ifndef ESP8266_H
#define ESP8266_H

#include "stm32f10x.h"
#include <stdint.h>
void ESP8266_Receive_Start(void);
uint8_t ESP8266_Connect_WiFi(char *ssid, char *password);
uint8_t ESP8266_Connect_Server(char *ip, char *port);
uint8_t ESP8266_TCP_Subscribe(char *uid, char *topic);
uint8_t ESP8266_TCP_Publish(char *uid, char *topic, char *data);
uint8_t ESP8266_TCP_Heartbeat(void);
#endif 
