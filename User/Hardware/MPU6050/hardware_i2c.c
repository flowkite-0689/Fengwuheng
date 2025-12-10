#include "hardware_i2c.h"
#include "Delay.h"
#include "debug.h"

/**
 * @brief Initialize I2C1 hardware for MPU6050 (PB6=SCL, PB7=SDA)
 */
void MPU6050_I2C_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    I2C_InitTypeDef I2C_InitStructure;

    // 1. Enable GPIOB and I2C1 clocks (I2C1 is on APB2)
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    // 2. Configure PB6(SCL) and PB7(SDA) as alternate function open drain
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD; // Alternate function open drain
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 3. Configure I2C1 parameters
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0x00; // Master address, any unused value
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = 400000; // 400kHz standard mode
    I2C_Init(I2C1, &I2C_InitStructure);

    // 4. Enable I2C1
    I2C_Cmd(I2C1, ENABLE);
}
/**
 * @brief 反初始化I2C硬件，关闭时钟以降低功耗
 * @param  无
 * @retval 无
 */
void MPU6050_I2C_Deinit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 1. 禁用I2C1外设
    I2C_Cmd(I2C1, DISABLE);
    
    // 2. 将GPIO引脚设置为模拟输入（最低功耗模式）
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;  // PB6(SCL), PB7(SDA)
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;           // 模拟输入模式
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // 3. 禁用I2C1时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, DISABLE);
    
    // 4. 禁用GPIOB和AFIO时钟（如果确定没有其他外设使用）
    // 注意：如果系统中还有其他设备使用这些资源，不要禁用
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, DISABLE);
    
    printf("I2C hardware deinitialized\n");
}
/**
 * @brief Wait for I2C event with timeout
 * @param I2Cx: I2C peripheral
 * @param I2C_EVENT: Event to wait for
 * @return 0: Success, 1: Timeout
 */
uint8_t MPU6050_WaitEvent(I2C_TypeDef* I2Cx, uint32_t I2C_EVENT) {
    uint32_t Timeout = 100000; // Timeout counter, adjust based on clock frequency
    while(Timeout--) {
        if(I2C_CheckEvent(I2Cx, I2C_EVENT) == SUCCESS) {
            return 0; // Success
        }
    }
    return 1; // Timeout failure
}

/**
 * @brief Write a single byte to MPU6050 register
 * @param RegAddress: Register address to write to
 * @param Data: Data to write
 * @return 0: Success, 1: Error
 */
uint8_t MPU6050_WriteReg(uint8_t RegAddress, uint8_t Data) {
    // Wait for bus to be free
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
    
    // Generate START condition
    I2C_GenerateSTART(I2C1, ENABLE);
    if(MPU6050_WaitEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT)) goto error; // EV5

    // Send device address (write mode)
    I2C_Send7bitAddress(I2C1, MPU6050_ADDRESS << 1, I2C_Direction_Transmitter);
    if(MPU6050_WaitEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) goto error; // EV6

    // Send register address
    I2C_SendData(I2C1, RegAddress);
    if(MPU6050_WaitEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTING)) goto error; // EV8_1

    // Send data
    I2C_SendData(I2C1, Data);
    if(MPU6050_WaitEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) goto error; // EV8_2

    // Generate STOP condition
    I2C_GenerateSTOP(I2C1, ENABLE);
    Delay_us(10); // Short delay to ensure STOP condition completes
    return 0; // Success

error:
    // Generate STOP condition on error
    I2C_GenerateSTOP(I2C1, ENABLE);
    return 1; // Error
}

/**
 * @brief Read a single byte from MPU6050 register
 * @param RegAddress: Register address to read from
 * @return Read data, 0xFF if error
 */
uint8_t MPU6050_ReadReg(uint8_t RegAddress) {
    uint8_t Data = 0;

    // Wait for bus to be free
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));

    // Phase 1: Send register address (write operation)
    I2C_GenerateSTART(I2C1, ENABLE);
    if(MPU6050_WaitEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT)) goto error;

    I2C_Send7bitAddress(I2C1, MPU6050_ADDRESS << 1, I2C_Direction_Transmitter);
    if(MPU6050_WaitEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) goto error;

    I2C_SendData(I2C1, RegAddress);
    if(MPU6050_WaitEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) goto error;

    // Phase 2: Restart and enter receive mode
    I2C_GenerateSTART(I2C1, ENABLE); // Repeated START condition
    if(MPU6050_WaitEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT)) goto error;

    I2C_Send7bitAddress(I2C1, MPU6050_ADDRESS << 1, I2C_Direction_Receiver);
    if(MPU6050_WaitEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)) goto error; // EV6

    // Prepare to receive last byte (NACK + STOP)
    I2C_AcknowledgeConfig(I2C1, DISABLE); // Disable ACK before reading last byte
    I2C_GenerateSTOP(I2C1, ENABLE);       // Prepare STOP condition

    // Wait for data reception complete
    if(MPU6050_WaitEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED)) goto error; // EV7
    Data = I2C_ReceiveData(I2C1); // Read data

    I2C_AcknowledgeConfig(I2C1, ENABLE); // Re-enable ACK for next communication
    return Data;

error:
    I2C_GenerateSTOP(I2C1, ENABLE);
    I2C_AcknowledgeConfig(I2C1, ENABLE);
    return 0xFF; // Return error value
}

/**
 * @brief Read multiple bytes from MPU6050 registers
 * @param RegAddress: Starting register address
 * @param len: Number of bytes to read
 * @param pData: Pointer to data buffer
 * @return 0: Success, 1: Error
 */
uint8_t MPU6050_ReadBytes(uint8_t RegAddress, uint8_t len, uint8_t *pData) {
    uint8_t i;

    // Wait for bus to be free
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));

    // Phase 1: Send register address (write operation)
    I2C_GenerateSTART(I2C1, ENABLE);
    if(MPU6050_WaitEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT)) goto error;

    I2C_Send7bitAddress(I2C1, MPU6050_ADDRESS << 1, I2C_Direction_Transmitter);
    if(MPU6050_WaitEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) goto error;

    I2C_SendData(I2C1, RegAddress);
    if(MPU6050_WaitEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) goto error;

    // Phase 2: Restart and enter receive mode
    I2C_GenerateSTART(I2C1, ENABLE); // Repeated START condition
    if(MPU6050_WaitEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT)) goto error;

    I2C_Send7bitAddress(I2C1, MPU6050_ADDRESS << 1, I2C_Direction_Receiver);
    if(MPU6050_WaitEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)) goto error;

    // Receive multiple bytes
    for(i = 0; i < len; i++) {
        if(i == len - 1) {
            // Last byte: disable ACK and generate STOP
            I2C_AcknowledgeConfig(I2C1, DISABLE);
            I2C_GenerateSTOP(I2C1, ENABLE);
        }
        
        if(MPU6050_WaitEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED)) goto error;
        pData[i] = I2C_ReceiveData(I2C1);
    }

    I2C_AcknowledgeConfig(I2C1, ENABLE); // Re-enable ACK
    return 0; // Success

error:
    I2C_GenerateSTOP(I2C1, ENABLE);
    I2C_AcknowledgeConfig(I2C1, ENABLE);
    return 1; // Error
}

// Compatibility functions for eMPL library
/**
 * @brief Hardware I2C function for eMPL library - write multiple bytes
 * @param dev_addr: Device address (ignored, uses MPU6050_ADDRESS)
 * @param reg_addr: Register address
 * @param len: Number of bytes to write
 * @param pdata: Pointer to data buffer
 * @return 0: Success, 1: Error
 */
uint8_t Hardware_I2C_Write_Bytes(uint8_t dev_addr, uint8_t reg_addr, uint32_t len, uint8_t *pdata) {
    uint32_t i;
    uint8_t result = 0;
    
    // Write bytes one by one
    for(i = 0; i < len; i++) {
        result |= MPU6050_WriteReg(reg_addr + i, pdata[i]);
    }
    
    return result;
}

/**
 * @brief Hardware I2C function for eMPL library - read multiple bytes
 * @param dev_addr: Device address (ignored, uses MPU6050_ADDRESS)
 * @param reg_addr: Register address
 * @param len: Number of bytes to read
 * @param pdata: Pointer to data buffer
 * @return 0: Success, 1: Error
 */
uint8_t Hardware_I2C_Read_Bytes_From_Reg(uint8_t dev_addr, uint8_t reg_addr, uint32_t len, uint8_t *pdata) {
    // For simplicity, cast to uint8_t (MPU6050 typically reads small amounts of data)
    if(len > 255) return 1; // Error if length too large
    return MPU6050_ReadBytes(reg_addr, (uint8_t)len, pdata);
}

/**
 * @brief Hardware I2C function for eMPL library - initialize I2C
 */
void Hardware_I2C_Init(void) {
    MPU6050_I2C_Init();
}
