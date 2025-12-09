#include "oled.h"
#include "stdlib.h"
#include "oledfont.h"

static uint8_t OLED_GRAM[144][8];
static uint8_t dirty_flag = 0;
static uint8_t dirty_x1 = 127, dirty_y1 = 63, dirty_x2 = 0, dirty_y2 = 0;

// 发送一个字节
// mode:数据/命令标志 0,表示命令;1,表示数据;
void OLED_WR_Byte(uint8_t dat, uint8_t mode)
{
	
	if (mode)
	{
		printf("OLED_Send_Byte satrt ->\n");
		OLED_Send_Byte(0x3c, 0x40, dat); 
		printf("<- OLED_Send_Byte end\n");
	}
	else
	{
		OLED_Send_Byte(0x3c, 0x00, dat); 
	}
}

// 反显函数,0正常显示，1 反色显示
void OLED_ColorTurn(uint8_t i)
{
	if (i == 0)
	{
		OLED_WR_Byte(0xA6, OLED_CMD); // 正常显示
	}
	if (i == 1)
	{
		OLED_WR_Byte(0xA7, OLED_CMD); // 反色显示
	}
}

// 屏幕旋转180度,0正常显示 1 屏幕翻转显示
void OLED_DisplayTurn(uint8_t i)
{
	if (i == 0)
	{
		OLED_WR_Byte(0xC8, OLED_CMD); // 正常显示
		OLED_WR_Byte(0xA1, OLED_CMD);
	}
	if (i == 1)
	{
		OLED_WR_Byte(0xC0, OLED_CMD); // 反转显示
		OLED_WR_Byte(0xA0, OLED_CMD);
	}
}


// 开启OLED显示
void OLED_DisPlay_On(void)
{
	OLED_WR_Byte(0x8D, OLED_CMD); // 电荷泵使能
	OLED_WR_Byte(0x14, OLED_CMD); // 开启电荷泵
	OLED_WR_Byte(0xAF, OLED_CMD); // 点亮屏幕
}

// 关闭OLED显示
void OLED_DisPlay_Off(void)
{
	OLED_WR_Byte(0x8D, OLED_CMD); // 电荷泵使能
	OLED_WR_Byte(0x10, OLED_CMD); // 关闭电荷泵
	OLED_WR_Byte(0xAE, OLED_CMD); // 关闭屏幕
}

// 更新显存到OLED,更新后显示的才是你配置后的内容
void OLED_Refresh(void)
{
	uint8_t i, n;
	uint8_t data[128];
	for (i = 0; i < 8; i++)
	{
		for (n = 0; n < 128; n++)
		{
			data[n] = OLED_GRAM[n][i];
		}

		OLED_WR_Byte(0xb0 + i, OLED_CMD); // 设置行起始地址
		OLED_WR_Byte(0x00, OLED_CMD);	  // 设置低列起始地址
		OLED_WR_Byte(0x10, OLED_CMD);	  // 设置高列起始地址
		OLED_Send_Bytes(0x3c, 0x40, 128, data);
	}
}

// 局部刷新函数，只刷新指定区域 (x1,y1) 到 (x2,y2)
void OLED_Refresh_Area(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
	uint8_t i, n, start_page, end_page, start_col, end_col;
	uint8_t data[128];
	
	// 参数检查和修正
	if (x1 > x2) { uint8_t temp = x1; x1 = x2; x2 = temp; }
	if (y1 > y2) { uint8_t temp = y1; y1 = y2; y2 = temp; }
	
	// 边界限制
	if (x1 >= 128) x1 = 127;
	if (x2 >= 128) x2 = 127;
	if (y1 >= 64) y1 = 63;
	if (y2 >= 64) y2 = 63;
	
	// 计算页面范围（每页8行）
	start_page = y1 / 8;
	end_page = y2 / 8;
	
	// 计算列范围
	start_col = x1;
	end_col = x2;
	
	// 刷新指定区域
	for (i = start_page; i <= end_page; i++)
	{
		// 设置页面地址
		OLED_WR_Byte(0xb0 + i, OLED_CMD);
		
		// 只刷新指定的列范围
		for (n = start_col; n <= end_col; n++)
		{
			data[n - start_col] = OLED_GRAM[n][i];
		}
		
		// 设置列地址
		OLED_WR_Byte(start_col & 0x0f, OLED_CMD);        // 低列地址
		OLED_WR_Byte(0x10 | (start_col >> 4), OLED_CMD); // 高列地址
		
		// 发送数据
		OLED_Send_Bytes(0x3c, 0x40, end_col - start_col + 1, data);
	}
}

// 标记脏区域，用于自动局部刷新
void OLED_Set_Dirty_Area(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
	// 参数检查和修正
	if (x1 > x2) { uint8_t temp = x1; x1 = x2; x2 = temp; }
	if (y1 > y2) { uint8_t temp = y1; y1 = y2; y2 = temp; }
	
	// 边界限制
	if (x1 >= 128) x1 = 127;
	if (x2 >= 128) x2 = 127;
	if (y1 >= 64) y1 = 63;
	if (y2 >= 64) y2 = 63;
	
	// 更新脏区域
	if (!dirty_flag) {
		dirty_x1 = x1; dirty_y1 = y1; dirty_x2 = x2; dirty_y2 = y2;
		dirty_flag = 1;
	} else {
		if (x1 < dirty_x1) dirty_x1 = x1;
		if (y1 < dirty_y1) dirty_y1 = y1;
		if (x2 > dirty_x2) dirty_x2 = x2;
		if (y2 > dirty_y2) dirty_y2 = y2;
	}
}

// 刷新脏区域
void OLED_Refresh_Dirty(void)
{
	if (dirty_flag) {
		OLED_Refresh_Area(dirty_x1, dirty_y1, dirty_x2, dirty_y2);
		dirty_flag = 0;
		dirty_x1 = 127; dirty_y1 = 63; dirty_x2 = 0; dirty_y2 = 0;
	}
}
// 清屏函数
void OLED_Clear(void)
{
	uint8_t i, n;
	for (i = 0; i < 8; i++)
	{
		for (n = 0; n < 128; n++)
		{
			OLED_GRAM[n][i] = 0; // 清除所有数据
		}
	}
	OLED_Refresh(); // 更新显示
}

// 画点
// x:0~127
// y:0~63
// t:1 填充 0,清空
void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t t)
{
	uint8_t i, m, n;
	i = y / 8;
	m = y % 8;
	n = 1 << m;
	if (t)
	{
		OLED_GRAM[x][i] |= n;
	}
	else
	{
		OLED_GRAM[x][i] = ~OLED_GRAM[x][i];
		OLED_GRAM[x][i] |= n;
		OLED_GRAM[x][i] = ~OLED_GRAM[x][i];
	}
}

// 画线
// x1,y1:起点坐标
// x2,y2:结束坐标
void OLED_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t mode)
{
	uint16_t t;
	int xerr = 0, yerr = 0, delta_x, delta_y, distance;
	int incx, incy, uRow, uCol;
	delta_x = x2 - x1; // 计算坐标增量
	delta_y = y2 - y1;
	uRow = x1; // 画线起点坐标
	uCol = y1;
	if (delta_x > 0)
		incx = 1; // 设置单步方向
	else if (delta_x == 0)
		incx = 0; // 垂直线
	else
	{
		incx = -1;
		delta_x = -delta_x;
	}
	if (delta_y > 0)
		incy = 1;
	else if (delta_y == 0)
		incy = 0; // 水平线
	else
	{
		incy = -1;
		delta_y = -delta_x;
	}
	if (delta_x > delta_y)
		distance = delta_x; // 选取基本增量坐标轴
	else
		distance = delta_y;
	for (t = 0; t < distance + 1; t++)
	{
		OLED_DrawPoint(uRow, uCol, mode); // 画点
		xerr += delta_x;
		yerr += delta_y;
		if (xerr > distance)
		{
			xerr -= distance;
			uRow += incx;
		}
		if (yerr > distance)
		{
			yerr -= distance;
			uCol += incy;
		}
	}
}
// x,y:圆心坐标
// r:圆的半径
void OLED_DrawCircle(uint8_t x, uint8_t y, uint8_t r)
{
	int a, b, num;
	a = 0;
	b = r;
	while (2 * b * b >= r * r)
	{
		OLED_DrawPoint(x + a, y - b, 1);
		OLED_DrawPoint(x - a, y - b, 1);
		OLED_DrawPoint(x - a, y + b, 1);
		OLED_DrawPoint(x + a, y + b, 1);

		OLED_DrawPoint(x + b, y + a, 1);
		OLED_DrawPoint(x + b, y - a, 1);
		OLED_DrawPoint(x - b, y - a, 1);
		OLED_DrawPoint(x - b, y + a, 1);

		a++;
		num = (a * a + b * b) - r * r; // 计算画的点离圆心的距离
		if (num > 0)
		{
			b--;
			a--;
		}
	}
}

// 在指定位置显示一个字符,包括部分字符
// x:0~127
// y:0~63
// size1:选择字体 6x8/6x12/8x16/12x24
// mode:0,反色显示;1,正常显示
void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr, uint8_t size1, uint8_t mode)
{
	uint8_t i, m, temp, size2, chr1;
	uint8_t x0 = x, y0 = y;
	if (size1 == 8)
		size2 = 6;
	else
		size2 = (size1 / 8 + ((size1 % 8) ? 1 : 0)) * (size1 / 2); // 得到字体一个字符对应点阵集所占的字节数
	chr1 = chr - ' ';											   // 计算偏移后的值
	for (i = 0; i < size2; i++)
	{
		if (size1 == 8)
		{
			temp = asc2_0806[chr1][i];
		} // 调用0806字体
		else if (size1 == 12)
		{
			temp = asc2_1206[chr1][i];
		} // 调用1206字体
		else if (size1 == 16)
		{
			temp = asc2_1608[chr1][i];
		} // 调用1608字体
		else if (size1 == 24)
		{
			temp = asc2_2412[chr1][i];
		} // 调用2412字体
		else
			return;
		for (m = 0; m < 8; m++)
		{
			if (temp & 0x01)
				OLED_DrawPoint(x, y, mode);
			else
				OLED_DrawPoint(x, y, !mode);
			temp >>= 1;
			y++;
		}
		x++;
		if ((size1 != 8) && ((x - x0) == size1 / 2))
		{
			x = x0;
			y0 = y0 + 8;
		}
		y = y0;
	}
}

// 显示字符串
// x,y:起点坐标
// size1:字体大小,8 / 16 / 24
//*chr:字符串起始地址
// mode:0,反色显示;1,正常显示
void OLED_ShowString(uint8_t x, uint8_t y, uint8_t *chr, uint8_t size1, uint8_t mode)
{
	while ((*chr >= ' ') && (*chr <= '~')) // 判断是不是非法字符!
	{
		OLED_ShowChar(x, y, *chr, size1, mode);
		if (size1 == 8)
			x += 6;
		else
			x += size1 / 2;
		chr++;
	}
}

// m^n
u32 OLED_Pow(uint8_t m, uint8_t n)
{
	u32 result = 1;
	while (n--)
	{
		result *= m;
	}
	return result;
}

// 显示数字
// x,y :起点坐标
// num :要显示的数字
// len :数字的位数
// size:字体大小
// mode:0,反色显示;1,正常显示
void OLED_ShowNum(uint8_t x, uint8_t y, u32 num, uint8_t len, uint8_t size1, uint8_t mode)
{
	uint8_t t, temp, m = 0;
	if (size1 == 8)
		m = 2;
	for (t = 0; t < len; t++)
	{
		temp = (num / OLED_Pow(10, len - t - 1)) % 10;
		if (temp == 0)
		{
			OLED_ShowChar(x + (size1 / 2 + m) * t, y, '0', size1, mode);
		}
		else
		{
			OLED_ShowChar(x + (size1 / 2 + m) * t, y, temp + '0', size1, mode);
		}
	}
}

// 显示汉字
// x,y:起点坐标
// num:汉字对应的序号
// mode:0,反色显示;1,正常显示
//void OLED_ShowChinese(uint8_t x, uint8_t y, uint8_t num, uint8_t size1, uint8_t mode)
//{
//	uint8_t m, temp;
//	uint8_t x0 = x, y0 = y;
//	uint16_t i, size3 = (size1 / 8 + ((size1 % 8) ? 1 : 0)) * size1; // 得到字体一个字符对应点阵集所占的字节数
//	for (i = 0; i < size3; i++)
//	{
//		if (size1 == 16)
//		{
//			temp = Hzk1[num][i];
//		} // 调用16*16字体
//		else if (size1 == 24)
//		{
//			temp = Hzk2[num][i];
//		} // 调用24*24字体
//		else if (size1 == 32)
//		{
//			temp = Hzk3[num][i];
//		} // 调用32*32字体
//		else if (size1 == 64)
//		{
//			temp = Hzk4[num][i];
//		} // 调用64*64字体
//		else
//			return;
//		for (m = 0; m < 8; m++)
//		{
//			if (temp & 0x01)
//				OLED_DrawPoint(x, y, mode);
//			else
//				OLED_DrawPoint(x, y, !mode);
//			temp >>= 1;
//			y++;
//		}
//		x++;
//		if ((x - x0) == size1)
//		{
//			x = x0;
//			y0 = y0 + 8;
//		}
//		y = y0;
//	}
//}

// num 显示汉字的个数
// space 每一遍显示的间隔
// mode:0,反色显示;1,正常显示
//void OLED_ScrollDisplay(uint8_t num, uint8_t space, uint8_t mode)
//{
//	uint8_t i, n, t = 0, m = 0, r;
//	while (1)
//	{
//		if (m == 0)
//		{
//			OLED_ShowChinese(128, 24, t, 16, mode); // 写入一个汉字保存在OLED_GRAM[][]数组中
//			t++;
//		}
//		if (t == num)
//		{
//			for (r = 0; r < 16 * space; r++) // 显示间隔
//			{
//				for (i = 1; i < 144; i++)
//				{
//					for (n = 0; n < 8; n++)
//					{
//						OLED_GRAM[i - 1][n] = OLED_GRAM[i][n];
//					}
//				}
//				OLED_Refresh();
//			}
//			t = 0;
//		}
//		m++;
//		if (m == 16)
//		{
//			m = 0;
//		}
//		for (i = 1; i < 144; i++) // 实现左移
//		{
//			for (n = 0; n < 8; n++)
//			{
//				OLED_GRAM[i - 1][n] = OLED_GRAM[i][n];
//			}
//		}
//		OLED_Refresh();
//	}
//}

// x,y：起点坐标
// sizex,sizey,图片长宽
// BMP[]：要写入的图片数组
// mode:0,反色显示;1,正常显示
void OLED_ShowPicture(uint8_t x, uint8_t y, uint8_t sizex, uint8_t sizey, const uint8_t BMP[], uint8_t mode)
{
	uint16_t j = 0;
	uint8_t i, n, temp, m;
	uint8_t x0 = x, y0 = y;
	sizey = sizey / 8 + ((sizey % 8) ? 1 : 0);
	for (n = 0; n < sizey; n++)
	{
		for (i = 0; i < sizex; i++)
		{
			temp = BMP[j];
			j++;
			for (m = 0; m < 8; m++)
			{
				if (temp & 0x01)
					OLED_DrawPoint(x, y, mode);
				else
					OLED_DrawPoint(x, y, !mode);
				temp >>= 1;
				y++;
			}
			x++;
			if ((x - x0) == sizex)
			{
				x = x0;
				y0 = y0 + 8;
			}
			y = y0;
		}
	}
}
// OLED initialization
void OLED_Init(void)
{
	OLED_I2C_Init();
  printf("OLED start initialization\n");
	
	// printf("Step 1: Turn off OLED panel\n");
	OLED_WR_Byte(0xAE, OLED_CMD); //--turn off oled panel
	
	// printf("Step 2: Set low column address\n");
	OLED_WR_Byte(0x00, OLED_CMD); //---set low column address
	
	// printf("Step 3: Set high column address\n");
	OLED_WR_Byte(0x10, OLED_CMD); //---set high column address
	
	// printf("Step 4: Set start line address\n");
	OLED_WR_Byte(0x40, OLED_CMD); //--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
	
	// printf("Step 5: Set contrast control register\n");
	OLED_WR_Byte(0x81, OLED_CMD); //--set contrast control register
	// printf("Step 6: Set SEG output current brightness\n");
	OLED_WR_Byte(0xCF, OLED_CMD); // Set SEG Output Current Brightness
	
	// printf("Step 7: Set SEG/Column mapping (0xa1 normal)\n");
	OLED_WR_Byte(0xA1, OLED_CMD); //--Set SEG/Column Mapping     0xa0左右反置 0xa1正常
	
	// printf("Step 8: Set COM/Row scan direction (0xc8 normal)\n");
	OLED_WR_Byte(0xC8, OLED_CMD); // Set COM/Row Scan Direction   0xc0上下反置 0xc8正常
	
	// printf("Step 9: Set normal display mode\n");
	OLED_WR_Byte(0xA6, OLED_CMD); //--set normal display
	
	// printf("Step 10: Set multiplex ratio (1 to 64)\n");
	OLED_WR_Byte(0xA8, OLED_CMD); //--set multiplex ratio(1 to 64)
	// printf("Step 11: Set 1/64 duty\n");
	OLED_WR_Byte(0x3f, OLED_CMD); //--1/64 duty
	
	// printf("Step 12: Set display offset\n");
	OLED_WR_Byte(0xD3, OLED_CMD); //-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
	// printf("Step 13: No offset\n");
	OLED_WR_Byte(0x00, OLED_CMD); //-not offset
	
	// printf("Step 14: Set display clock divide ratio/oscillator frequency\n");
	OLED_WR_Byte(0xd5, OLED_CMD); //--set display clock divide ratio/oscillator frequency
	// printf("Step 15: Set divide ratio, clock as 100 frames/sec\n");
	OLED_WR_Byte(0x80, OLED_CMD); //--set divide ratio, Set Clock as 100 Frames/Sec
	
	// printf("Step 16: Set pre-charge period\n");
	OLED_WR_Byte(0xD9, OLED_CMD); //--set pre-charge period
	// printf("Step 17: Set pre-charge as 15 clocks & discharge as 1 clock\n");
	OLED_WR_Byte(0xF1, OLED_CMD); // Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
	
	// printf("Step 18: Set COM pins hardware configuration\n");
	OLED_WR_Byte(0xDA, OLED_CMD); //--set com pins hardware configuration
	// printf("Step 19: COM pins configuration value 0x12\n");
	OLED_WR_Byte(0x12, OLED_CMD);
	
	// printf("Step 20: Set VCOMH\n");
	OLED_WR_Byte(0xDB, OLED_CMD); //--set vcomh
	// printf("Step 21: Set VCOM deselect level\n");
	OLED_WR_Byte(0x30, OLED_CMD); // Set VCOM Deselect Level
	
	// printf("Step 22: Set page addressing mode\n");
	OLED_WR_Byte(0x20, OLED_CMD); //-Set Page Addressing Mode (0x00/0x01/0x02)
	// printf("Step 23: Page addressing mode value 0x02\n");
	OLED_WR_Byte(0x02, OLED_CMD); //
	
	// printf("Step 24: Set charge pump enable/disable\n");
	OLED_WR_Byte(0x8D, OLED_CMD); //--set Charge Pump enable/disable
	// printf("Step 25: Enable charge pump\n");
	OLED_WR_Byte(0x14, OLED_CMD); //--set(0x10) disable
	
	// printf("Step 26: Clear screen\n");
	OLED_Clear();
	
	// printf("Step 27: Turn on OLED display\n");
	OLED_WR_Byte(0xAF, OLED_CMD);
	
	printf("OLED initialization complete\n");
}


