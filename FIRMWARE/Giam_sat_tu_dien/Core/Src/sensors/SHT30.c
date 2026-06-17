/*
 * SHT30.c
 *
 *  Created on: 1 thg 6, 2026
 *      Author: Admin
 */
#include "sensors/SHT30.h"

HAL_StatusTypeDef SHT30_Read(float *temp, float *humi)
{
   // Gửi lệnh đo
   if (HAL_I2C_Master_Transmit(SHT30_I2C_HANDLE, SHT30_ADDR, sht30_cmd, 2, 100) != HAL_OK)
       return HAL_ERROR;
   HAL_Delay(20); // chờ đo
   // Đọc 6 byte
   if (HAL_I2C_Master_Receive(SHT30_I2C_HANDLE, SHT30_ADDR, sht30_data, 6, 100) != HAL_OK)
       return HAL_ERROR;
   // Ghép dữ liệu
   uint16_t rawTemp = (sht30_data[0] << 8) | sht30_data[1];
   uint16_t rawHumi = (sht30_data[3] << 8) | sht30_data[4];
   // Công thức chuyển đổi (theo datasheet)
   *temp = -45 + 175 * ((float)rawTemp / 65535.0);
   *humi = 100 * ((float)rawHumi / 65535.0);
   return HAL_OK;
}

// Check CRC
uint8_t SHT30_CRC(uint8_t *data, int len)
{
    uint8_t crc = 0xFF;

    for (int i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (int j = 0; j < 8; j++)
        {
            if (crc & 0x80)
                crc = (crc << 1) ^ 0x31;
            else
                crc <<= 1;
        }
    }
    return crc;
}


