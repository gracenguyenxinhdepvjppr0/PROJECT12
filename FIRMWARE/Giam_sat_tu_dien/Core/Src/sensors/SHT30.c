/*
 * SHT30.c
 *
 *  Created on: 1 thg 6, 2026
 *      Author: Admin
 */
#include "sensors/SHT30.h"
#include "math.h"

uint8_t sht30_cmd[2] = {0x24, 0x00};
uint8_t sht30_data[6];
char bufftemp[10], buffhumid[10], buffer[20];
float temperature, humidity;

HAL_StatusTypeDef SHT30_Read(float *temp, float *humi)
{
   // Gửi lệnh đo
   if (HAL_I2C_Master_Transmit(SHT30_I2C_HANDLE, SHT30_ADDR, sht30_cmd, 2, 100) != HAL_OK)
       return HAL_ERROR;
   HAL_Delay(20); // chờ đo
   // Đọc 6 byte
   if (HAL_I2C_Master_Receive(SHT30_I2C_HANDLE, SHT30_ADDR, sht30_data, 6, 100) != HAL_OK)
       return HAL_ERROR;

   if (SHT30_CheckCRC(sht30_data[0], sht30_data[1], sht30_data[2]) && SHT30_CheckCRC(sht30_data[3], sht30_data[4], sht30_data[5]))
   {
	   // Ghép dữ liệu
	   uint16_t rawTemp = (sht30_data[0] << 8) | sht30_data[1];
	   uint16_t rawHumi = (sht30_data[3] << 8) | sht30_data[4];

	   // Công thức chuyển đổi (theo datasheet)
	   *temp = -45.0f + 175.0f * ((float)rawTemp / 65535.0f);
	   *humi = 100.0f * ((float)rawHumi / 65535.0f);
	   return HAL_OK;
   }

   return HAL_ERROR;
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

uint8_t SHT30_CheckCRC(uint8_t msb, uint8_t lsb, uint8_t sensor_crc)
{
    uint8_t data[2] = {msb, lsb};
    uint8_t calculated_crc = SHT30_CRC(data, 2);

    return (calculated_crc == sensor_crc);
}

float SHT30_CalcDewPoint(float temp, float humi)
{
    // Hằng số Magnus
    const float b = 17.62f;
    const float c = 243.12f;

    // Tránh lỗi toán học (log của 0 hoặc số âm không xác định)
    if (humi <= 0.0f)
    {
        humi = 0.001f;
    }

    // Tính hệ số alpha
    float alpha = logf(humi / 100.0f) + ((b * temp) / (c + temp));

    // Tính Dew Point
    float dewPoint = (c * alpha) / (b - alpha);

    return dewPoint;
}
