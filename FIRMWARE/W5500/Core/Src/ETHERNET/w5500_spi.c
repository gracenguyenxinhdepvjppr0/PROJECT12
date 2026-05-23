/*
 * w5500_spi.c
 *
 *  Created on: 27 thg 12, 2025
 *      Author: Admin
 */
#include "stm32f4xx_hal.h"
#include "wizchip_conf.h"
#include "stdio.h"
#include "main.h"

extern SPI_HandleTypeDef hspi1;

uint8_t SPIReadWrite(uint8_t data)
{
	while ((hspi1.Instance -> SR & SPI_FLAG_TXE) != SPI_FLAG_TXE);
	*(__IO uint8_t*) & hspi1.Instance -> DR = data;

	while ((hspi1.Instance -> SR & SPI_FLAG_RXNE) != SPI_FLAG_RXNE);
	return (*(__IO uint8_t*) & hspi1.Instance -> DR);
}

void wizchip_select(void)
{
	HAL_GPIO_WritePin(GPIOA, CS_Pin, GPIO_PIN_RESET);
}

void wizchip_deselect(void)
{
	HAL_GPIO_WritePin(GPIOA, CS_Pin, GPIO_PIN_SET);
}

uint8_t wizchip_read()
{
	uint8_t readByte = SPIReadWrite(0x00);
	return readByte;
}

void wizchip_write(uint8_t writeByte)
{
	SPIReadWrite(writeByte);
}

void wizchip_readburst(uint8_t* pBuff, uint16_t len)
{
	for (uint16_t i = 0; i < len; i++)
	{
		*pBuff = SPIReadWrite(0x00);
		pBuff ++;
	}
}

void wizchip_writeburst(uint8_t* pBuff, uint16_t len)
{
	for (uint16_t i = 0; i < len; i++)
		{
			SPIReadWrite(*pBuff);
			pBuff ++;
		}
}


void W5500_Init()
{
	uint8_t tmp;
	uint8_t memsize[2][8] = {{2, 2, 2, 2, 2, 2, 2, 2}, {2, 2, 2, 2, 2, 2, 2, 2}};

	MX_GPIO_Init();

	HAL_GPIO_WritePin(GPIOA, CS_Pin, GPIO_PIN_SET);

	HAL_GPIO_WritePin(GPIOA, RST_Pin, GPIO_PIN_RESET);
	tmp = 0xFF;
	while (tmp --)
		HAL_GPIO_WritePin(GPIOA, RST_Pin, GPIO_PIN_SET);

	reg_wizchip_cs_cbfunc(wizchip_select, wizchip_deselect);
	reg_wizchip_spi_cbfunc(wizchip_read, wizchip_write);
	reg_wizchip_spiburst_cbfunc(wizchip_readburst, wizchip_writeburst);

	if (ctlwizchip(CW_INIT_WIZCHIP, (void*) memsize == -1))
	{
		printf("WIZCHIP Initialization failed\r\n");
		while(1);
	}
	printf("WIZCHIP Initialization successfully\r\n");
}
