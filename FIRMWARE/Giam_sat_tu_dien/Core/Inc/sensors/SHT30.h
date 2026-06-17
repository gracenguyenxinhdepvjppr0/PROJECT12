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

uint8_t sht30_cmd[2] = {0x24, 0x00};
uint8_t sht30_data[6];
char bufftemp[10], buffhumid[10], buffer[20];
float temperature, humidity;

HAL_StatusTypeDef SHT30_Read(float*, float*);
uint8_t SHT30_CRC(uint8_t *data, int len);

#endif /* INC_SENSORS_SHT30_H_ */
