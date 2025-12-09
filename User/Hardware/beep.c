#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "debug.h"
/**
  * 函    数：蜂鸣器初始化
  * 参    数：无
  * 返 回 值：无
  */
void Beep_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);		//开启GPIOA的时钟
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;				//推挽输出
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;					//PA4
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);						//将PA4引脚初始化为推挽输出
	
	/*设置默认电平*/
	GPIO_ResetBits(GPIOA, GPIO_Pin_4);							//PA4引脚输出高电平，蜂鸣器不响
  printf("beep init OK\n\n\n\n\n");
}

/**
  * 函    数：蜂鸣器开启
  * 参    数：无
  * 返 回 值：无
  */
void Beep_ON(void)
{
  printf("ON");
	GPIO_SetBits(GPIOA, GPIO_Pin_4);		//PA4引脚输出高电平，蜂鸣器响
}

/**
  * 函    数：蜂鸣器关闭
  * 参    数：无
  * 返 回 值：无
  */
void Beep_OFF(void)
{
	GPIO_ResetBits(GPIOA, GPIO_Pin_4);		//PA4引脚输出低电平，蜂鸣器不响
}

/**
  * 函    数：蜂鸣器状态翻转
  * 参    数：无
  * 返 回 值：无
  */
void Beep_Turn(void)
{
	if (GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_4) == 0)		//获取输出寄存器的状态，如果当前引脚输出低电平
	{
		GPIO_SetBits(GPIOA, GPIO_Pin_4);						//则设置PA4引脚为高电平，蜂鸣器响
	}
	else														//否则，即当前引脚输出高电平
	{
		GPIO_ResetBits(GPIOA, GPIO_Pin_4);						//则设置PA4引脚为低电平，蜂鸣器不响
	}
}

/**
  * 函    数：蜂鸣器鸣叫指定时间
  * 参    数：Time 鸣叫时间，单位：毫秒
  * 返 回 值：无
  */
void Beep_Time(uint32_t Time)
{
	Beep_ON();							//开启蜂鸣器
	Delay_ms(Time);						//延时
	Beep_OFF();							//关闭蜂鸣器
}

/**
  * 函    数：蜂鸣器单次提示音
  * 参    数：无
  * 返 回 值：无
  */
void Beep_Alert(void)
{
	Beep_Time(100);						//鸣叫100ms
}

/**
  * 函    数：蜂鸣器按键音
  * 参    数：无
  * 返 回 值：无
  */
void Beep_Key(void)
{
	Beep_Time(50);						//鸣叫50ms，短促按键音
}

/**
  * 函    数：蜂鸣器闹钟提示音
  * 参    数：无
  * 返 回 值：无
  */
void Beep_Alarm(void)
{
	// 鸣叫3次，每次200ms，间隔100ms
	for(uint8_t i = 0; i < 3; i++)
	{
		Beep_Time(200);
		Delay_ms(100);
	}
}

/**
  * 函    数：蜂鸣器通知提示音
  * 参    数：无
  * 返 回 值：无
  */
void Beep_Notification(void)
{
	// 鸣叫2次，每次150ms，间隔50ms
	for(uint8_t i = 0; i < 2; i++)
	{
		Beep_Time(150);
		Delay_ms(50);
	}
}

/**
  * 函    数：蜂鸣器低电量提示音
  * 参    数：无
  * 返 回 值：无
  */
void Beep_LowBattery(void)
{
	// 长鸣500ms
	Beep_Time(500);
}

/**
  * 函    数：蜂鸣器错误提示音
  * 参    数：无
  * 返 回 值：无
  */
void Beep_Error(void)
{
	// 长短结合的声音
	Beep_Time(200);
	Delay_ms(100);
	Beep_Time(100);
	Delay_ms(100);
	Beep_Time(50);
}
void BEEP_Buzz( uint32_t duration_ms){
    Beep_ON();
    Delay_ms(duration_ms);
    Beep_OFF();
}
