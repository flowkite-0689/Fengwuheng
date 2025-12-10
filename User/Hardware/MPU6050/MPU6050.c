
#include "MPU6050.h"
#include "stdio.h"
#include "eMPL/inv_mpu.h"
#include "eMPL/inv_mpu_dmp_motion_driver.h"
#include "../debug.h"
#include "../../System/Delay.h"

u8 MPU_Init(void)
{
	u8 res;
	MPU6050_IIC_Init();								   // 初始化IIC总线
	MPU_Write_Byte(MPU_ADDR, MPU_PWR_MGMT1_REG, 0X80); // 复位MPU6050
	delay_ms(100);
	MPU_Write_Byte(MPU_ADDR, MPU_PWR_MGMT1_REG, 0X00); // 唤醒MPU6050
	MPU_Set_Gyro_Fsr(3);							   // 陀螺仪传感器,±2000dps
	MPU_Set_Accel_Fsr(0);							   // 加速度传感器,±2g
	MPU_Set_Rate(50);								   // 设置采样率50Hz
	MPU_Write_Byte(MPU_ADDR, MPU_INT_EN_REG, 0X00);	   // 关闭所有中断
	MPU_Write_Byte(MPU_ADDR, MPU_USER_CTRL_REG, 0X00); // I2C主模式关闭
	MPU_Write_Byte(MPU_ADDR, MPU_FIFO_EN_REG, 0X00);   // 关闭FIFO
	MPU_Write_Byte(MPU_ADDR, MPU_INTBP_CFG_REG, 0X80); // INT引脚低电平有效
	MPU_Read_Byte(MPU_ADDR, MPU_DEVICE_ID_REG, &res);
	printf("RES=%X\r\n", res);
	if (res == MPU_ADDR) // 器件ID正确
	{
		MPU_Write_Byte(MPU_ADDR, MPU_PWR_MGMT1_REG, 0X01); // 设置CLKSEL,PLL X轴为参考
		MPU_Write_Byte(MPU_ADDR, MPU_PWR_MGMT2_REG, 0X00); // 加速度与陀螺仪都工作
		MPU_Set_Rate(50);								   // 设置采样率为50Hz
	}
	else
		return 1;
	return 0;
}
// 设置MPU6050陀螺仪传感器满量程范围
// fsr:0,±250dps;1,±500dps;2,±1000dps;3,±2000dps
// 返回值:0,设置成功
//     其他,设置失败
u8 MPU_Set_Gyro_Fsr(u8 fsr)
{
	return MPU_Write_Byte(MPU_ADDR, MPU_GYRO_CFG_REG, fsr << 3); // 设置陀螺仪满量程范围
}
// 设置MPU6050加速度传感器满量程范围
// fsr:0,±2g;1,±4g;2,±8g;3,±16g
// 返回值:0,设置成功
//     其他,设置失败
u8 MPU_Set_Accel_Fsr(u8 fsr)
{
	return MPU_Write_Byte(MPU_ADDR, MPU_ACCEL_CFG_REG, fsr << 3); // 设置加速度传感器满量程范围
}
// 设置MPU6050的数字低通滤波器
// lpf:数字低通滤波频率(Hz)
// 返回值:0,设置成功
//     其他,设置失败
u8 MPU_Set_LPF(u16 lpf)
{
	u8 data = 0;
	if (lpf >= 188)
		data = 1;
	else if (lpf >= 98)
		data = 2;
	else if (lpf >= 42)
		data = 3;
	else if (lpf >= 20)
		data = 4;
	else if (lpf >= 10)
		data = 5;
	else
		data = 6;
	return MPU_Write_Byte(MPU_ADDR, MPU_CFG_REG, data); // 设置数字低通滤波器
}
// 设置MPU6050的采样率(假定Fs=1KHz)
// rate:4~1000(Hz)
// 返回值:0,设置成功
//     其他,设置失败
u8 MPU_Set_Rate(u16 rate)
{
	u8 data;
	if (rate > 1000)
		rate = 1000;
	if (rate < 4)
		rate = 4;
	data = 1000 / rate - 1;
	data = MPU_Write_Byte(MPU_ADDR, MPU_SAMPLE_RATE_REG, data); // 设置数字低通滤波器
	return MPU_Set_LPF(rate / 2);								// 自动设置LPF为采样率的一半
}

// 得到温度值
// 返回值:温度值(扩大了100倍)
short MPU_Get_Temperature(void)
{
	u8 buf[2];
	short raw;
	float temp;
	MPU_Read_Bytes(MPU_ADDR, MPU_TEMP_OUTH_REG, 2, buf);
	raw = ((u16)buf[0] << 8) | buf[1];
	temp = 36.53 + ((double)raw) / 340;
	return temp * 100;
}
// 得到陀螺仪值(原始值)
// gx,gy,gz:陀螺仪x,y,z轴的原始读数(带符号)
// 返回值:0,成功
//     其他,错误代码
u8 MPU_Get_Gyroscope(short *gx, short *gy, short *gz)
{
	u8 buf[6], res;
	res = MPU_Read_Bytes(MPU_ADDR, MPU_GYRO_XOUTH_REG, 6, buf);
	if (res == 0)
	{
		*gx = ((u16)buf[0] << 8) | buf[1];
		*gy = ((u16)buf[2] << 8) | buf[3];
		*gz = ((u16)buf[4] << 8) | buf[5];
	}
	return res;
	;
}
// 得到加速度值(原始值)
// gx,gy,gz:陀螺仪x,y,z轴的原始读数(带符号)
// 返回值:0,成功
//     其他,错误代码
u8 MPU_Get_Accelerometer(short *ax, short *ay, short *az)
{
	u8 buf[6], res;
	res = MPU_Read_Bytes(MPU_ADDR, MPU_ACCEL_XOUTH_REG, 6, buf);
	if (res == 0)
	{
		*ax = ((u16)buf[0] << 8) | buf[1];
		*ay = ((u16)buf[2] << 8) | buf[3];
		*az = ((u16)buf[4] << 8) | buf[5];
	}
	return res;
	;
}

//传送数据给匿名四轴上位机软件(V2.6版本)
//fun:功能字. 0XA0~0XAF
//data:数据缓存区,最多28字节!!
//len:data区有效数据个数
void MPU_NimingReport(u8 fun,u8*data,u8 len)
{
	u8 send_buf[32];
	u8 i;
	if(len>28)return;	//最多28字节数据 
	send_buf[len+3]=0;	//校验数置零
	send_buf[0]=0X88;	//帧头
	send_buf[1]=fun;	//功能字
	send_buf[2]=len;	//数据长度
	for(i=0;i<len;i++)send_buf[3+i]=data[i];			//复制数据
	for(i=0;i<len+3;i++)send_buf[len+3]+=send_buf[i];	//计算校验和	
	for(i=0;i<len+4;i++) MUP_uart_send_bytes(&send_buf[i],1);//发送数据到串口 
}
//通过串口上报结算后的姿态数据给电脑
//aacx,aacy,aacz:x,y,z三个方向上面的加速度值
//gyrox,gyroy,gyroz:x,y,z三个方向上面的陀螺仪值
//roll:横滚角.单位0.01度。 -18000 -> 18000 对应 -180.00  ->  180.00度
//pitch:俯仰角.单位 0.01度。-9000 - 9000 对应 -90.00 -> 90.00 度
//yaw:航向角.单位为0.1度 0 -> 3600  对应 0 -> 360.0度
void MPU_ReportImu(short aacx,short aacy,short aacz,short gyrox,short gyroy,short gyroz,short roll,short pitch,short yaw)
{
	u8 tbuf[28]; 
	u8 i;
	for(i=0;i<28;i++)tbuf[i]=0;//清0
	tbuf[0]=(aacx>>8)&0XFF;
	tbuf[1]=aacx&0XFF;
	tbuf[2]=(aacy>>8)&0XFF;
	tbuf[3]=aacy&0XFF;
	tbuf[4]=(aacz>>8)&0XFF;
	tbuf[5]=aacz&0XFF; 
	tbuf[6]=(gyrox>>8)&0XFF;
	tbuf[7]=gyrox&0XFF;
	tbuf[8]=(gyroy>>8)&0XFF;
	tbuf[9]=gyroy&0XFF;
	tbuf[10]=(gyroz>>8)&0XFF;
	tbuf[11]=gyroz&0XFF;	
	tbuf[18]=(roll>>8)&0XFF;
	tbuf[19]=roll&0XFF;
	tbuf[20]=(pitch>>8)&0XFF;
	tbuf[21]=pitch&0XFF;
	tbuf[22]=(yaw>>8)&0XFF;
	tbuf[23]=yaw&0XFF;
	MPU_NimingReport(0XAF,tbuf,28);//飞控显示帧,0XAF
} 

// 计步器步数设置包装函数
int dmp_set_pedometer_step_count_wrap(unsigned long count)
{
	return dmp_set_pedometer_step_count(count);
}

// 计步器时间设置包装函数
int dmp_set_pedometer_walk_time_wrap(unsigned long time)
{
	return dmp_set_pedometer_walk_time(time);
}

// 计步器步数获取包装函数
int dmp_get_pedometer_step_count_wrap(unsigned long *count)
{
	return dmp_get_pedometer_step_count(count);
}

// 计步器时间获取包装函数
int dmp_get_pedometer_walk_time_wrap(unsigned long *time)
{
	return dmp_get_pedometer_walk_time(time);
}
/**
 * @brief 将MPU6050设置为睡眠模式以降低功耗
 * @param  无
 * @retval 0: 成功
 *         1: 失败
 */
u8 MPU_Deinit(void)
{
    u8 res;
    
    // 首先检查MPU6050是否在线
    MPU_Read_Byte(MPU_ADDR, MPU_DEVICE_ID_REG, &res);
    if (res != MPU_ADDR) {
        printf("MPU6050 not found, cannot enter sleep mode\n");
        return 1;
    }
    
    // 将MPU6050设置为睡眠模式
    // 写入0x40到电源管理寄存器1(第6位SLEEP位置1)
    MPU_Write_Byte(MPU_ADDR, MPU_PWR_MGMT1_REG, 0x40);
    
    printf("MPU6050 entered sleep mode\n");
    return 0;
}
