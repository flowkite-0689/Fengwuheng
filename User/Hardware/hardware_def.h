/**
 * @file hardware_def.h
 * @brief STM32F103 硬件引脚定义和位带操作宏
 * @author flowkite-0689  
 * @version v2.0
 * @date 2025.11.10
 * @note 移植自STM32F4版本，适配STM32F103硬件
 */

#ifndef _HARDWARE_DEF_H_
#define _HARDWARE_DEF_H_

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"

// ==================================
// STM32F103 位带操作宏定义
// ==================================

/**
 * @defgroup Bitband_Operations 位带操作宏定义
 * @brief 提供STM32F103位带操作的宏定义
 * @{
 */

#define BITBAND_PERIPH_BASE  0x42000000U
#define BITBAND_SRAM_BASE    0x22000000U  


#define BITBAND_PERIPH(reg, bit)  (*(volatile uint32_t*)(BITBAND_PERIPH_BASE + ((uint32_t)(reg) - PERIPH_BASE) * 32 + (bit) * 4))

#define GPIO_OUT(PORT, PIN)    BITBAND_PERIPH(&(PORT->ODR), PIN)
#define GPIO_IN(PORT, PIN)     BITBAND_PERIPH(&(PORT->IDR), PIN)
#define GPIO_SET(PORT, PIN)    (PORT->BSRR = (1UL << (PIN)))
#define GPIO_RST(PORT, PIN)    (PORT->BSRR = (1UL << ((PIN) + 16)))

/** @} */

// ==================================
// 硬件引脚定义
// ==================================

// LED定义 - 
#define LED3_PIN GPIO_Pin_3
#define LED3_PORT GPIOB  
#define LED3_NUM 3

#define LED4_PIN GPIO_Pin_4
#define LED4_PORT GPIOB
#define LED4_NUM 4

// 按键定义 - 


#define KEY1_PIN GPIO_Pin_12  
#define KEY1_PORT GPIOB
#define KEY1_NUM 12

#define KEY2_PIN GPIO_Pin_13
#define KEY2_PORT GPIOB
#define KEY2_NUM 13

#define KEY3_PIN GPIO_Pin_14
#define KEY3_PORT GPIOB
#define KEY3_NUM 14

#define KEY4_PIN GPIO_Pin_15
#define KEY4_PORT GPIOB
#define KEY4_NUM 15

// 蜂鸣器定义
#define BEEP0_PIN GPIO_Pin_4
#define BEEP0_PORT GPIOA
#define BEEP0_NUM 4

// ==================================
// 设备操作宏（保持接口不变）
// ==================================

// LED操作宏
#define LED3 GPIO_OUT(LED3_PORT, LED3_NUM)
#define LED4 GPIO_OUT(LED4_PORT, LED4_NUM)
#define LED3_STATE() GPIO_IN(LED3_PORT, LED3_NUM)
#define LED4_STATE() GPIO_IN(LED4_PORT, LED4_NUM)

// 蜂鸣器操作宏  
#define BEEP0(state) (state ? GPIO_SET(BEEP0_PORT, BEEP0_NUM) : GPIO_RST(BEEP0_PORT, BEEP0_NUM))
#define BEEP GPIO_OUT(BEEP0_PORT, BEEP0_NUM)

// 按键操作宏
#define KEY4 GPIO_IN(KEY4_PORT, KEY4_NUM)
#define KEY3 GPIO_IN(KEY3_PORT, KEY3_NUM)
#define KEY2 GPIO_IN(KEY2_PORT, KEY2_NUM)
#define KEY1 GPIO_IN(KEY1_PORT, KEY1_NUM)


// DHT11位带操作宏定义
#define PGout(n) GPIO_OUT(GPIOG, n)  // 输出
#define PGin(n)  GPIO_IN(GPIOG, n)   // 输入

// GPIOB位带操作宏定义（用于I2C）
#define PBout(n) GPIO_OUT(GPIOB, n)  // 输出
#define PBin(n)  GPIO_IN(GPIOB, n)   // 输入
#endif
