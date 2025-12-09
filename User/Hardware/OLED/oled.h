#ifndef __OLED_H
#define __OLED_H

#include "stm32f10x.h"

/************************************这些宏需要你补全************************************** */
#include "soft_i2c.h"
#define OLED_I2C_Init()									Soft_I2C_Init()
#define OLED_Send_Byte(dev_addr, reg_addr, data) 		Soft_I2C_Write_Byte(dev_addr, reg_addr, data)
#define OLED_Send_Bytes(dev_addr, reg_addr, len, pdata) Soft_I2C_Write_Bytes(dev_addr, reg_addr, len, pdata)
/****************************************end********************************************** */
#define OLED_CMD 0  // 写命令
#define OLED_DATA 1 // 写数据
void OLED_ClearPoint(uint8_t x, uint8_t y);
void OLED_ColorTurn(uint8_t i);
void OLED_DisplayTurn(uint8_t i);
void OLED_WR_Byte(uint8_t dat, uint8_t mode);
void OLED_DisPlay_On(void);
void OLED_DisPlay_Off(void);
void OLED_Refresh(void);
void OLED_Refresh_Area(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
void OLED_Set_Dirty_Area(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
void OLED_Refresh_Dirty(void);
void OLED_Clear(void);
void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t t);
void OLED_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t mode);
void OLED_DrawCircle(uint8_t x, uint8_t y, uint8_t r);
void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr, uint8_t size1, uint8_t mode);
void OLED_ShowString(uint8_t x, uint8_t y, uint8_t *chr, uint8_t size1, uint8_t mode);
void OLED_ShowNum(uint8_t x, uint8_t y, u32 num, uint8_t len, uint8_t size1, uint8_t mode);
void OLED_ShowChinese(uint8_t x, uint8_t y, uint8_t num, uint8_t size1, uint8_t mode);
void OLED_ScrollDisplay(uint8_t num, uint8_t space, uint8_t mode);
void OLED_ShowPicture(uint8_t x, uint8_t y, uint8_t sizex, uint8_t sizey, const uint8_t BMP[], uint8_t mode);
void OLED_Init(void);
#endif
