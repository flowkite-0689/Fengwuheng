#include "soft_i2c.h"

// PB9->SDA, PB8->SCL
// 引脚初始化
void Soft_I2C_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;        
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;    // 开漏输出
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz; 
  
	GPIO_Init(GPIOB, &GPIO_InitStruct);	
	// 初始释放总线（拉高）
    SCL_H;
    SDA_H;
	printf("I2Cinit OK\n");
	
}

// PB9输入/输出配置
void Soft_I2C_Mode(GPIOMode_TypeDef GPIO_Mode)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;        
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode;    
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_Init(GPIOB, &GPIO_InitStruct);	
}

// 起始信号
void Soft_I2C_Start(void)
{
	// printf("Soft_I2C_Start - >\n");


	SDA_H;
	SCL_H;
	// printf("delay start->");

	I2C_DELAY;
	

	SDA_L;
	I2C_DELAY;
	
	SCL_L;
	// printf("<-Soft_I2C_Start\n");
}


// 停止信号
void Soft_I2C_Stop(void)
{
	SDA_L;
	SCL_H;
	I2C_DELAY;
	
	SDA_H;
	I2C_DELAY;
}



// 发送应答信号
void Soft_I2C_Send_Ack(void)
{
	SDA_L;
	I2C_DELAY;
	
	SCL_H;
	I2C_DELAY;
	
	SCL_L;
	I2C_DELAY;
	
	SDA_H;
}

// 发送非应答信号
void Soft_I2C_Send_NAck(void)
{
	SDA_H;
	I2C_DELAY;
	
	SCL_H;
	I2C_DELAY;
	
	SCL_L;
	I2C_DELAY;
	
	SDA_H;
}



// 接收应答0/非应答信号1,返回应答信号
uint8_t Soft_I2C_Wait_Ack(void)
{
	uint8_t ack = 0;
	SDA_H;	// 释放sda总线
	
	Soft_I2C_Mode(GPIO_Mode_IPU);// 设置输入模式
	I2C_DELAY;	// 让从机控制SDA一会，等待SDA稳定
	
	SCL_H;
	I2C_DELAY;
	if(SDAin == 1)
		ack = 1;
	else 
		ack = 0;
	SCL_L;
	I2C_DELAY;	// 让从机等一会后拉高
	
	Soft_I2C_Mode(GPIO_Mode_Out_OD);
	SDA_H;	// 释放sda总线
	return ack;
}

// 发送一个字节
void Soft_I2C_Send_Byte(uint8_t data)
{

	
	for (int i = 0; i < 8; i++)
	{
		// 先发高位
		if (data & 0x80) // 发送数据1
		{
			SDA_H;
		}
		else	// 发送数据0
		{
			SDA_L;	
		}
		I2C_DELAY;

		SCL_H;
		I2C_DELAY;

		SCL_L;
		I2C_DELAY;

		data <<= 1;
	}
	SDA_H;	// 释放总线
}

// 接收一个字节
uint8_t Soft_I2C_Read_Byte(void)
{
	uint8_t data = 0;
	SDA_H;	// 释放sda总线
	
	Soft_I2C_Mode(GPIO_Mode_IPU);// 设置输入模式
	I2C_DELAY;	// 让从机控制SDA一会，等待SDA稳定
	for (int i = 0; i < 8; i++) // 高位先收
	{
		data <<= 1;
		SCL_H;
		I2C_DELAY;
		if(SDAin == 1)
			data |= 1;
		
		SCL_L;
		I2C_DELAY;	// 让从机准备好发送的数据
	}
	Soft_I2C_Mode(GPIO_Mode_Out_OD);
	SDA_H;	// 释放sda总线
	return data;
}

/**
  * @brief  向指定设备寄存器写入一个字节
  * @param  dev_addr: 设备地址(7位)
  * @param  reg_addr: 寄存器地址
  * @param  data: 要写入的数据
  * @retval 0:成功, 1:失败
  */
uint8_t Soft_I2C_Write_Byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t data)
{
	// printf("Soft_I2C_Write_Byte start->\n");
	Soft_I2C_Start();
	Soft_I2C_Send_Byte(dev_addr << 1);	// 发送从机地址（写）
	// printf("Soft_I2C_Send_Byte : %d",dev_addr);
	if (Soft_I2C_Wait_Ack())
	{
		Soft_I2C_Stop();
		return 1;
	}
	Soft_I2C_Send_Byte(reg_addr);	// 发送寄存器地址
	if (Soft_I2C_Wait_Ack())
	{
		Soft_I2C_Stop();
		return 1;
	}
	Soft_I2C_Send_Byte(data);	// 发送1个字节数据
	if (Soft_I2C_Wait_Ack())
	{
		Soft_I2C_Stop();
		return 1;
	}
	Soft_I2C_Stop();
	return 0;
}

uint8_t Soft_I2C_Write_Bytes(uint8_t dev_addr, uint8_t reg_addr, uint32_t len, uint8_t *data)
{
	Soft_I2C_Start();
	Soft_I2C_Send_Byte(dev_addr << 1);	// 发送从机地址（写）
	if (Soft_I2C_Wait_Ack())
	{
		Soft_I2C_Stop();
		return 1;
	}
	Soft_I2C_Send_Byte(reg_addr);	// 发送寄存器地址
	if (Soft_I2C_Wait_Ack())
	{
		Soft_I2C_Stop();
		return 1;
	}
	
	for(uint32_t i = 0; i < len; i++)
	{
		Soft_I2C_Send_Byte(*data);	// 发送1个字节数据
		if (Soft_I2C_Wait_Ack())
		{
			Soft_I2C_Stop();
			return 1;
		}
		data++;
	}
	
	Soft_I2C_Stop();
	return 0;
}




/**
  * @brief  从指定设备寄存器读取一个字节
  * @param  dev_addr: 设备地址(7位)
  * @param  reg_addr: 寄存器地址
  * @param  data: 读取到的数据指针
  * @retval 0:成功, 1:失败
  */
uint8_t Soft_I2C_Read_Byte_From_Reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data)
{
	Soft_I2C_Start();
	Soft_I2C_Send_Byte(dev_addr << 1);	// 发送从机地址（写）
	if (Soft_I2C_Wait_Ack())
	{
		Soft_I2C_Stop();
		return 1;
	}
	Soft_I2C_Send_Byte(reg_addr);	// 发送寄存器地址
	if (Soft_I2C_Wait_Ack())
	{
		Soft_I2C_Stop();
		return 1;
	}
	Soft_I2C_Start();
	Soft_I2C_Send_Byte((dev_addr << 1) | 1);	// 发送从机地址（读）
	if (Soft_I2C_Wait_Ack())
	{
		Soft_I2C_Stop();
		return 1;
	}
	
	*data = Soft_I2C_Read_Byte();
	Soft_I2C_Send_NAck();
	Soft_I2C_Stop();
	return 0;
}

uint8_t Soft_I2C_Read_Bytes_From_Reg(uint8_t dev_addr, uint8_t reg_addr, uint32_t len, uint8_t *data)
{
	Soft_I2C_Start();
	Soft_I2C_Send_Byte(dev_addr << 1);	// 发送从机地址（写）
	if (Soft_I2C_Wait_Ack())
	{
		Soft_I2C_Stop();
		return 1;
	}
	Soft_I2C_Send_Byte(reg_addr);	// 发送寄存器地址
	if (Soft_I2C_Wait_Ack())
	{
		Soft_I2C_Stop();
		return 1;
	}
	Soft_I2C_Start();
	Soft_I2C_Send_Byte((dev_addr << 1) | 1);	// 发送从机地址（读）
	if (Soft_I2C_Wait_Ack())
	{
		Soft_I2C_Stop();
		return 1;
	}
	
	for(uint32_t i = 0; i < len; i++)
	{
		*data = Soft_I2C_Read_Byte();
		if (i == len - 1)		// 最后一次读，终止数据传输，发送Nack
			Soft_I2C_Send_NAck();
		else
			Soft_I2C_Send_Ack();
		data++;
	}
	
	Soft_I2C_Stop();
	return 0;
}

