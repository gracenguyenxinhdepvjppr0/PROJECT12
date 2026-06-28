/*
 * SHT30.h
 *
 *  Created on: 1 thg 6, 2026
 *      Author: Admin
 */

#ifndef INC_SENSORS_SHT30_H_
#define INC_SENSORS_SHT30_H_

#include "stdio.h"
#include "main.h"

extern I2C_HandleTypeDef hi2c3;
#define SHT30_ADDR (0x44 << 1)
#define SHT30_I2C_HANDLE &hi2c3

extern uint8_t sht30_cmd[2];
extern uint8_t sht30_data[6];
extern char bufftemp[10], buffhumid[10], buffer[20];
extern float temperature, humidity;

HAL_StatusTypeDef SHT30_Read(float*, float*);
uint8_t SHT30_CRC(uint8_t *data, int len);
uint8_t SHT30_CheckCRC(uint8_t msb, uint8_t lsb, uint8_t sensor_crc);
float SHT30_CalcDewPoint(float temp, float humi);

#endif /* INC_SENSORS_SHT30_H_ */
