#ifndef __HARDWARE_I2C_H
#define __HARDWARE_I2C_H

#include "stm32f10x.h"
#include "stm32f10x_i2c.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

// MPU6050 I2C address
#define MPU6050_ADDRESS 0x68

// Function prototypes
void MPU6050_I2C_Init(void);
uint8_t MPU6050_WaitEvent(I2C_TypeDef* I2Cx, uint32_t I2C_EVENT);
uint8_t MPU6050_WriteReg(uint8_t RegAddress, uint8_t Data);
uint8_t MPU6050_ReadReg(uint8_t RegAddress);
uint8_t MPU6050_ReadBytes(uint8_t RegAddress, uint8_t len, uint8_t *pData);

// Hardware I2C functions for eMPL library (avoid conflict with soft_i2c)
uint8_t Hardware_I2C_Write_Bytes(uint8_t dev_addr, uint8_t reg_addr, uint32_t len, uint8_t *pdata);
uint8_t Hardware_I2C_Read_Bytes_From_Reg(uint8_t dev_addr, uint8_t reg_addr, uint32_t len, uint8_t *pdata);
void Hardware_I2C_Init(void);
void MPU6050_I2C_Deinit(void);
#endif
