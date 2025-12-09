
#include "Delay.h"
/**
  * @brief  TIM2初始化，用于微秒级延时
  * @param  无
  * @retval 无
  */
void TIM2_Delay_Init(void) {
    TIM_TimeBaseInitTypeDef TIM_InitStruct;
    
    // 启用TIM2时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    
    // 配置TIM2
    TIM_InitStruct.TIM_Prescaler = 72 - 1;      // 72MHz / 72 = 1MHz (1μs分辨率)
    TIM_InitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_InitStruct.TIM_Period = 0xFFFF;         // 最大计数值65535
    TIM_InitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_InitStruct.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM2, &TIM_InitStruct);
}

/**
  * @brief  TIM2实现的微秒级延时
  * @param  us 延时时长（单位：微秒）
  * @retval 无
  */
void TIM2_Delay_us(uint32_t us) {
    TIM_SetCounter(TIM2, 0);      // 清零计数器
    TIM_Cmd(TIM2, ENABLE);        // 启动TIM2
    while (TIM_GetCounter(TIM2) < us);  // 等待计数达到目标值
    TIM_Cmd(TIM2, DISABLE);       // 关闭TIM2
}




/**
  * @brief  微秒级延时
  * @param  xus 延时时长，范围：0~233015
  * @retval 无
  */
void Delay_us(uint32_t xus)
{

	// SysTick->LOAD = 72 * xus;				//设置定时器重装值
	// SysTick->VAL = 0x00;					//清空当前计数值
	// SysTick->CTRL = 0x00000005;				//设置时钟源为HCLK，启动定时器
	// while(!(SysTick->CTRL & 0x00010000));	//等待计数到0
	// SysTick->CTRL = 0x00000004;				//关闭定时器
	TIM2_Delay_us(xus);
}

/**
  * @brief  毫秒级延时
  * @param  xms 延时时长，范围：0~4294967295
  * @retval 无
  */
void Delay_ms(uint32_t xms)
{
	while(xms--)
	{
		Delay_us(1000);
	}
}
 
/**
  * @brief  秒级延时
  * @param  xs 延时时长，范围：0~4294967295
  * @retval 无
  */
void Delay_s(uint32_t xs)
{
	while(xs--)
	{
		Delay_ms(1000);
	}
} 
void delay_us_no_irq(uint32_t us)
{
  TIM2_Delay_us(us);
}

void delay_ms(uint32_t xms)
{
  while(xms--)
	{
		Delay_us(1000);
	}
}
