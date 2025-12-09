#include "dht11.h"
#include "Delay.h"

void DHT11_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;        
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;    
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;  
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	PBout(5) = 1;


}


//1.请求读取温湿度数据时，配置STM32芯片引脚模式（输出）
void DHT11_Mode_Out_PP(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;        
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;    
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_Init(GPIOB, &GPIO_InitStruct);
}

//2.正在读取温湿度数据时，配置STM32芯片引脚模式（输入）
void DHT11_Mode_IPU(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;        
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;    
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz; 
   
	GPIO_Init(GPIOB, &GPIO_InitStruct);
}

// 读 8 位数据
uint8_t Read_Byte(void)
{
	uint8_t temp = 0;
	for (int i = 0; i < 8; i++)	// 高位先发,i=0高位的数据，i=7低位的数据
	{
		while(PBin(5) == 0);	// 等高电平
		
		delay_us_no_irq(40);	// 延时40us
		if (PBin(5) == 1)		// 数据1
		{
			while(PBin(5) == 1);// 等变成低电平
			temp |= (1 << (7 - i));
		}
		else					// 数据0
		{
			//temp默认的所有位为 0，不用处理接收的0数据
		}
	}
	return temp;
}


// 返回值：失败-1，成功0
int Read_DHT11(DHT11_Data_TypeDef* data)
{
	
	uint32_t count = 0;
	//3.控制芯片引脚发出开始信号（输出模式）
	DHT11_Mode_Out_PP();	
	PBout(5) = 0;
	Delay_ms(20);	// 低电平至少18ms
	PBout(5) = 1;
	delay_us_no_irq(30);	// 拉高30us
	
	//4.判断DHT11响应信号对不对（输入模式）
	DHT11_Mode_IPU();
//	while(PBin(5) == 1);		// 等dht11控制低电平
//	
	while(PBin(5) == 0)	// 统计低电平的时间
	{
		count++;
		if (count > 100)	// 1ms一直是低电平读失败了
		{
			DHT11_Mode_Out_PP();	
			PBout(5) = 1;
			return -1;
		}	
		delay_us_no_irq(10);
	}
	
	count = 0;
	while(PBin(5) == 1)	// 统计高电平的时间
	{
		count++;
		if (count > 100)	// 1ms一直是高电平读失败了
		{
			DHT11_Mode_Out_PP();	
			PBout(5) = 1;
			return -1;
		}	
		delay_us_no_irq(10);
	}
	
	//5.读40bit数据
	//    1）怎么判断数据是0？怎么判断数据是1？
	//    2）高位先发，你要处理接收到的0和1。
	data->humi_int = Read_Byte();
	data->humi_deci = Read_Byte();
	data->temp_int = Read_Byte();
	data->temp_deci = Read_Byte();
	data->check_sum = Read_Byte();
	
	DHT11_Mode_Out_PP();	
	PBout(5) = 1;
	//6.验证数据是否正确（比较"校验和" 数据)
	if (data->check_sum == (data->humi_int + data->humi_deci +
						data->temp_int + data->temp_deci) & 0xFF)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}




