#ifndef SOFT_I2C_H
#define SOFT_I2C_H


#include "hardware_def.h"
#include "Delay.h"
#include "debug.h"

#define SCL_H GPIO_OUT(GPIOB,8) = 1
#define SCL_L GPIO_OUT(GPIOB,8) = 0
#define SDA_H	GPIO_OUT(GPIOB,9) = 1
#define SDA_L	GPIO_OUT(GPIOB,9) = 0
#define SDAin	GPIO_IN(GPIOB,9)
#define I2C_DELAY  Delay_us(3)

void Soft_I2C_Init(void);
uint8_t Soft_I2C_Write_Byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t data);
uint8_t Soft_I2C_Write_Bytes(uint8_t dev_addr, uint8_t reg_addr, uint32_t len, uint8_t *data);
uint8_t Soft_I2C_Read_Byte_From_Reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data);
uint8_t Soft_I2C_Read_Bytes_From_Reg(uint8_t dev_addr, uint8_t reg_addr, uint32_t len, uint8_t *data);
#endif

