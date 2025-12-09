#ifndef __MPU6050_HARDWARE_I2C_H
#define __MPU6050_HARDWARE_I2C_H
 
#include "debug.h"
#include "eMPL/inv_mpu.h"

// MPU6050 Register Definitions
//#define MPU_ACCEL_OFFS_REG        0X06    // accel_offs register
//#define MPU_PROD_ID_REG           0X0C    // prod id register
#define MPU_SELF_TESTX_REG        0X0D    // Self-test register X
#define MPU_SELF_TESTY_REG        0X0E    // Self-test register Y
#define MPU_SELF_TESTZ_REG        0X0F    // Self-test register Z
#define MPU_SELF_TESTA_REG        0X10    // Self-test register A
#define MPU_SAMPLE_RATE_REG       0X19    // Sample rate divider
#define MPU_CFG_REG               0X1A    // Configuration register
#define MPU_GYRO_CFG_REG          0X1B    // Gyroscope configuration register
#define MPU_ACCEL_CFG_REG         0X1C    // Accelerometer configuration register
#define MPU_MOTION_DET_REG        0X1F    // Motion detection threshold register
#define MPU_FIFO_EN_REG           0X23    // FIFO enable register
#define MPU_I2CMST_CTRL_REG       0X24    // I2C master control register
#define MPU_I2CSLV0_ADDR_REG      0X25    // I2C slave 0 device address register
#define MPU_I2CSLV0_REG           0X26    // I2C slave 0 data address register
#define MPU_I2CSLV0_CTRL_REG      0X27    // I2C slave 0 control register
#define MPU_I2CSLV1_ADDR_REG      0X28    // I2C slave 1 device address register
#define MPU_I2CSLV1_REG           0X29    // I2C slave 1 data address register
#define MPU_I2CSLV1_CTRL_REG      0X2A    // I2C slave 1 control register
#define MPU_I2CSLV2_ADDR_REG      0X2B    // I2C slave 2 device address register
#define MPU_I2CSLV2_REG           0X2C    // I2C slave 2 data address register
#define MPU_I2CSLV2_CTRL_REG      0X2D    // I2C slave 2 control register
#define MPU_I2CSLV3_ADDR_REG      0X2E    // I2C slave 3 device address register
#define MPU_I2CSLV3_REG           0X2F    // I2C slave 3 data address register
#define MPU_I2CSLV3_CTRL_REG      0X30    // I2C slave 3 control register
#define MPU_I2CSLV4_ADDR_REG      0X31    // I2C slave 4 device address register
#define MPU_I2CSLV4_REG           0X32    // I2C slave 4 data address register
#define MPU_I2CSLV4_DO_REG        0X33    // I2C slave 4 write data register
#define MPU_I2CSLV4_CTRL_REG      0X34    // I2C slave 4 control register
#define MPU_I2CSLV4_DI_REG        0X35    // I2C slave 4 read data register

#define MPU_I2CMST_STA_REG        0X36    // I2C master status register
#define MPU_INTBP_CFG_REG         0X37    // Interrupt/bypass configuration register
#define MPU_INT_EN_REG            0X38    // Interrupt enable register
#define MPU_INT_STA_REG           0X3A    // Interrupt status register

#define MPU_ACCEL_XOUTH_REG       0X3B    // Accelerometer X high byte
#define MPU_ACCEL_XOUTL_REG       0X3C    // Accelerometer X low byte
#define MPU_ACCEL_YOUTH_REG       0X3D    // Accelerometer Y high byte
#define MPU_ACCEL_YOUTL_REG       0X3E    // Accelerometer Y low byte
#define MPU_ACCEL_ZOUTH_REG       0X3F    // Accelerometer Z high byte
#define MPU_ACCEL_ZOUTL_REG       0X40    // Accelerometer Z low byte

#define MPU_TEMP_OUTH_REG         0X41    // Temperature high byte
#define MPU_TEMP_OUTL_REG         0X42    // Temperature low byte

#define MPU_GYRO_XOUTH_REG        0X43    // Gyroscope X high byte
#define MPU_GYRO_XOUTL_REG        0X44    // Gyroscope X low byte
#define MPU_GYRO_YOUTH_REG        0X45    // Gyroscope Y high byte
#define MPU_GYRO_YOUTL_REG        0X46    // Gyroscope Y low byte
#define MPU_GYRO_ZOUTH_REG        0X47    // Gyroscope Z high byte
#define MPU_GYRO_ZOUTL_REG        0X48    // Gyroscope Z low byte

#define MPU_I2CSLV0_DO_REG        0X63    // I2C slave 0 data register
#define MPU_I2CSLV1_DO_REG        0X64    // I2C slave 1 data register
#define MPU_I2CSLV2_DO_REG        0X65    // I2C slave 2 data register
#define MPU_I2CSLV3_DO_REG        0X66    // I2C slave 3 data register

#define MPU_I2CMST_DELAY_REG      0X67    // I2C master delay register
#define MPU_SIGPATH_RST_REG       0X68    // Signal path reset register
#define MPU_MDETECT_CTRL_REG      0X69    // Motion detection control register
#define MPU_USER_CTRL_REG         0X6A    // User control register
#define MPU_PWR_MGMT1_REG         0X6B    // Power management register 1
#define MPU_PWR_MGMT2_REG         0X6C    // Power management register 2
#define MPU_FIFO_CNTH_REG         0X72    // FIFO count high byte
#define MPU_FIFO_CNTL_REG         0X73    // FIFO count low byte
#define MPU_FIFO_RW_REG           0X74    // FIFO read/write register
#define MPU_DEVICE_ID_REG         0X75    // Device ID register
 
// If AD0 pin (pin 9) is grounded, I2C address is 0X68
// If connected to 3.3V, I2C address is 0X69
#define MPU_ADDR                  0X68

/************************************ Hardware I2C Implementation ************************************** */
#include "hardware_i2c.h"
#include "debug.h"
#include "Delay.h"
#define log_i printf	// Print information
#define log_e printf	// Print error
#define delay_ms Delay_ms
#define MPU6050_IIC_Init() Hardware_I2C_Init()
#define MPU_Write_Byte(dev_addr, reg_addr, data) MPU6050_WriteReg(reg_addr, data)
#define MPU_Write_Bytes(dev_addr, reg_addr, len, pdata) Hardware_I2C_Write_Bytes(dev_addr, reg_addr, len, pdata)
#define MPU_Read_Byte(dev_addr, reg_addr, pdata) *pdata = MPU6050_ReadReg(reg_addr)
#define MPU_Read_Bytes(dev_addr, reg_addr, len, pdata) Hardware_I2C_Read_Bytes_From_Reg(dev_addr, reg_addr, len, pdata)
#define MUP_uart_send_bytes(buf, len) Usart1_send_bytes(buf, len)
/****************************************end********************************************** */

// Function prototypes
u8 MPU_Init(void);                             // Initialize MPU6050
u8 MPU_Set_Gyro_Fsr(u8 fsr);
u8 MPU_Set_Accel_Fsr(u8 fsr);
u8 MPU_Set_LPF(u16 lpf);
u8 MPU_Set_Rate(u16 rate);
u8 MPU_Set_Fifo(u8 sens);

short MPU_Get_Temperature(void);
u8 MPU_Get_Gyroscope(short *gx,short *gy,short *gz);
u8 MPU_Get_Accelerometer(short *ax,short *ay,short *az);
void MPU_ReportImu(short aacx,short aacy,short aacz,short gyrox,short gyroy,short gyroz,short roll,short pitch,short yaw);

// DMP pedometer functions
// mpu_dmp_init() - Initialize DMP with attitude calculation (Euler angles) and motion detection
// mpu_dmp_init_pedometer() - Initialize DMP specifically for pedometer functionality
int dmp_set_pedometer_step_count_wrap(unsigned long count);
int dmp_set_pedometer_walk_time_wrap(unsigned long time);
int dmp_get_pedometer_step_count_wrap(unsigned long *count);
int dmp_get_pedometer_walk_time_wrap(unsigned long *time);
#endif
