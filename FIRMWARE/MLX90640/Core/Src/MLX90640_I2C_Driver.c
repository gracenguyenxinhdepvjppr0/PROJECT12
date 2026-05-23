/*
 * MLX90640_I2C_Driver.c
 *
 *  Created on: 7 thg 4, 2026
 *      Author: Admin
 */
/**
 * @file    MLX90640_I2C_Driver.c
 * @brief   I2C Driver for Melexis MLX90640 targeting STM32F411CEU6 using HAL
 */

#include "MLX90640_I2C_Driver.h"
#include "stm32f4xx_hal.h"

extern I2C_HandleTypeDef hi2c1;

void MLX90640_I2CInit(void)
{
   //Đã được khởi tạo qua CubeMX
}

int MLX90640_I2CGeneralReset(void)
{
    uint8_t cmd = 0x06;

    // MLX90640 sử dụng General Call Address là 0x00 cho lệnh Reset
    if (HAL_I2C_Master_Transmit(&hi2c1, 0x00, &cmd, 1, HAL_MAX_DELAY) != HAL_OK) {
        return -1; // Lỗi
    }
    return 0;      // Thành công
}

int MLX90640_I2CRead(uint8_t slaveAddr, uint16_t startAddress, uint16_t nMemAddressRead, uint16_t *data)
{
    // Cấp phát buffer để chứa dữ liệu thô (mỗi thanh ghi 16-bit = 2 bytes)
    // Lưu ý: Nếu dùng RTOS có stack nhỏ, hãy cân nhắc dùng 'static uint8_t buffer[1664]'
    // hoặc cấp phát động (malloc) vì nMemAddressRead có thể lên tới 832 words (1664 bytes).
    static uint8_t buffer[1664];

    // Đọc liên tiếp các byte từ I2C
    // Chú ý: slaveAddr phải dịch trái 1 bit cho thư viện HAL
    if (HAL_I2C_Mem_Read(&hi2c1, (slaveAddr << 1), startAddress, I2C_MEMADD_SIZE_16BIT, buffer, nMemAddressRead * 2, HAL_MAX_DELAY) != HAL_OK) {
        return -1;
    }

    // Ghép 2 bytes thành 1 word 16-bit (Chuyển đổi Big-Endian từ MLX sang Little-Endian của STM32)
    for (uint16_t cnt = 0; cnt < nMemAddressRead; cnt++) {
        uint16_t i = cnt << 1; // i = cnt * 2
        data[cnt] = ((uint16_t)buffer[i] << 8) | buffer[i + 1];
    }

    return 0;
}

int MLX90640_I2CWrite(uint8_t slaveAddr, uint16_t writeAddress, uint16_t data)
{
    uint8_t buffer[2];

    // Tách 1 word 16-bit thành 2 bytes (Chuyển Little-Endian thành Big-Endian)
    buffer[0] = (uint8_t)(data >> 8);   // MSB
    buffer[1] = (uint8_t)(data & 0xFF); // LSB

    // Ghi dữ liệu xuống MLX90640
    if (HAL_I2C_Mem_Write(&hi2c1, (slaveAddr << 1), writeAddress, I2C_MEMADD_SIZE_16BIT, buffer, 2, HAL_MAX_DELAY) != HAL_OK) {
        return -1;
    }

    return 0;
}

void MLX90640_I2CFreqSet(int freq)
{
    //Tần số I2C đã được fix cứng khi config trên CubeMX
    //nên ko thay đổi đc 
}

