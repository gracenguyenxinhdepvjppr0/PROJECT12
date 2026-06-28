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

#include "sensors/MLX90640/MLX90640_I2C_Driver.h"
#include "stm32h7xx_hal.h"

void MLX90640_I2CInit(void)
{
   //Đã được khởi tạo qua CubeMX
}

int MLX90640_I2CGeneralReset(void)
{
    uint8_t cmd = 0x06;

    // MLX90640 sử dụng General Call Address là 0x00 cho lệnh Reset
    if (HAL_I2C_Master_Transmit(MLX90640_I2C_HANDLE, 0x00, &cmd, 1, HAL_MAX_DELAY) != HAL_OK)
    {
        return -1; // Lỗi
    }
    return 0;      // Thành công
}

int MLX90640_I2CRead(uint8_t slaveAddr, uint16_t startAddress, uint16_t nMemAddressRead, uint16_t *data)
{
	uint16_t chunkSize = 128; // Tốc độ cao: Đọc 128 words (256 bytes) mỗi nhịp
	uint16_t wordsRead = 0;
	static uint8_t buffer[1664]; // Mảng tĩnh để hứng toàn bộ dữ liệu (tránh tràn RAM)

	while (wordsRead < nMemAddressRead)
	{
		uint16_t wordsToRead = (nMemAddressRead - wordsRead) > chunkSize ? chunkSize : (nMemAddressRead - wordsRead);

		if (HAL_I2C_Mem_Read(MLX90640_I2C_HANDLE, (slaveAddr << 1), startAddress + wordsRead, I2C_MEMADD_SIZE_16BIT, &buffer[wordsRead * 2], wordsToRead * 2, 100) != HAL_OK)
		{
			return -1;
		}
		wordsRead += wordsToRead;
	}

	// Đảo Byte cho toàn bộ mảng một lần duy nhất sau khi đọc xong
	for (uint16_t cnt = 0; cnt < nMemAddressRead; cnt++)
	{
		uint16_t i = cnt << 1;
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
    if (HAL_I2C_Mem_Write(MLX90640_I2C_HANDLE, (slaveAddr << 1), writeAddress, I2C_MEMADD_SIZE_16BIT, buffer, 2, HAL_MAX_DELAY) != HAL_OK)
    {
        return -1;
    }

    return 0;
}

void MLX90640_I2CFreqSet(int freq)
{
    //Tần số I2C đã được fix cứng khi config trên CubeMX nên ko thay đổi đc
}

