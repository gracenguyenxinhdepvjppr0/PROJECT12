#include "MLX90640_I2C_Driver.h"

I2C_HandleTypeDef *mlx_i2c = NULL;
paramsMLX90640 *mlx = NULL;
UART_HandleTypeDef *mlx_usart = NULL;
//void uart_print(const char *s) {
//    HAL_UART_Transmit(mlx_usart, (uint8_t*)s, strlen(s), 125);
//}


int MLX90640_I2CGeneralReset(void) {
    //uint8_t *p = (uint8_t*)(0x06);
	uint8_t cmd = 0x06;
    if (HAL_I2C_Master_Transmit(mlx_i2c, 0x00, &cmd, 1, 100) != HAL_OK) {
        return -1;
    }
    return 0;
}

void send_frame_uart(float *frame) {

    const char header[] = "FRAME_START\n";
    HAL_UART_Transmit(mlx_usart, (uint8_t*)header, strlen(header), 100);


    char buffer[32];
    for (int i = 0; i < 768; i++) {
        int n = snprintf(buffer, sizeof(buffer), "%.2f,", frame[i]);
        HAL_UART_Transmit(mlx_usart, (uint8_t*)buffer, n, 100);
    }


    const char footer[] = "\nFRAME_END\n";
    HAL_UART_Transmit(mlx_usart, (uint8_t*)footer, strlen(footer), 100);
}

//int MLX90640_init(uint16_t slaveAddr, uint16_t *eeDT) {
//
//	int status;
//
//	status = MLX90640_DumpEE(slaveAddr, eeDT);
//
//	if (status != 0) return status;
//
//	status = MLX90640_ExtractParameters(eeDT, mlx);
//
//	if(status != 0) return status;
//
//	status = MLX90640_SetChessMode(slaveAddr);
//
//	if(status != 0) return status;
//
//	status = MLX90640_SetResolution(slaveAddr, MLX90640_RES_16BIT);
//
//	if(status != 0) return status;
//
//	status = MLX90640_SetRefreshRate(slaveAddr, MLX90640_REFRESH_RATE_4HZ);
//
//	if(status != 0) return status;
//
//
//	return 0;
//}

int MLX90640_I2CRead(uint8_t slaveAddr, uint16_t startAddress, uint16_t nMemAddressRead, uint16_t *data) {
	int status = 0;
 uint8_t buffer[2 * nMemAddressRead];
	//static uint8_t buffer[1668];
	status = HAL_I2C_Mem_Read(mlx_i2c, (uint16_t)(slaveAddr << 1), startAddress, I2C_MEMADD_SIZE_16BIT, buffer, sizeof(buffer), 500);
	if (status != HAL_OK) {
		return -1;
	}

    for (uint16_t i = 0; i < nMemAddressRead; i++) {
        data[i] = ((uint16_t)buffer[2*i] << 8) | buffer[2*i + 1];
    }
	return 0;
}

int MLX90640_I2CWrite(uint8_t slaveAddr,uint16_t writeAddress, uint16_t data) {
	int status = 0;
	uint8_t cmd[2];

    cmd[0] = (uint8_t)(data >> 8);
    cmd[1] = (uint8_t)(data & 0xFF);

	status = HAL_I2C_Mem_Write(mlx_i2c, (uint16_t)(slaveAddr << 1), writeAddress, I2C_MEMADD_SIZE_16BIT, cmd, sizeof(cmd), 500);

	if (status != HAL_OK) {
		return -1;
	}
	return 0;
}

extern void MLX90640_I2CFreqSet(int freq) {

}



//int MLX90640_read(uint16_t slaveAddr, uint16_t *frame, float *image, float emissivity)
//{
//
//
//    int status = 0;
//    float tr = 0;
//	float ta;
//    //float ta = 10;
//    status = MLX90640_SynchFrame(slaveAddr);
//    if (status < 0) {
//    	char m[64];
//    	snprintf(m,64,"Synch1 err=%d\r\n", status);
//    	uart_print(m);
//    	return status;
//    }
//    status = MLX90640_GetFrameData(slaveAddr, frame);
//    if (status < 0) {
//    	char m[64];
//    	snprintf(m,64,"Synch2 err=%d\r\n", status);
//    	uart_print(m);
//    	return status;
//    }
//
//    int sp0 = MLX90640_GetSubPageNumber(frame);
//
//    char m0[32];
//    snprintf(m0,sizeof(m0),"sp0=%d\r\n", sp0);
//    uart_print(m0);
//    ta = MLX90640_GetTa(frame, mlx);
//	tr = ta - TA_SHIFT;
//
//	MLX90640_CalculateTo(frame, mlx, emissivity, tr, image);
//
//	HAL_Delay(250);
//
//    status = MLX90640_GetFrameData(slaveAddr, frame);
//    if (status < 0) {
//    	char m[64];
//    	snprintf(m,64,"Synch3 err=%d\r\n", status);
//    	uart_print(m);
//    	return status;
//    }
//    int sp1 = MLX90640_GetSubPageNumber(frame);
//
//    char m1[32]; snprintf(m1,sizeof(m1),"sp1=%d\r\n", sp1); uart_print(m1);
//
//    if (sp1 != sp0) {
//    	ta = MLX90640_GetTa(frame, mlx);
//    	tr = ta - TA_SHIFT;
//
//    	MLX90640_CalculateTo(frame, mlx, emissivity, tr, image);
//    }
//    else {
//    	char m3[64];
//    	snprintf(m3,64,"Error subpage\r\n");
//    	uart_print(m3);
//    	return -5;
//    }
//
//    HAL_Delay(250);
//
//
//    return 0;
//}

//static void uart_print(const char *s) {
//    HAL_USART_Transmit(&husart2, (uint8_t*)s, strlen(s), 125);
//}
//
//
//extern int MLX90640_I2CGeneralReset(void) {
//    //uint8_t *p = (uint8_t*)(0x06);
//	uint8_t cmd = 0x06;
//    if (HAL_I2C_Master_Transmit(&hi2c2, 0x00, &cmd, 1, 100) != HAL_OK) {
//        return -1;
//    }
//    return 0;
//}
//
//void send_frame_uart(float *frame) {
//
//    const char header[] = "FRAME_START\n";
//    HAL_USART_Transmit(&husart2, (uint8_t*)header, strlen(header), 100);
//
//
//    char buffer[32];
//    for (int i = 0; i < 768; i++) {
//        int n = snprintf(buffer, sizeof(buffer), "%.2f,", frame[i]);
//        HAL_USART_Transmit(&husart2, (uint8_t*)buffer, n, 100);
//    }
//
//
//    const char footer[] = "\nFRAME_END\n";
//    HAL_USART_Transmit(&husart2, (uint8_t*)footer, strlen(footer), 100);
//}
//
//int MLX90640_init(uint16_t slaveAddr, uint16_t *eeDT) {
//
//	int status;
//
//	status = MLX90640_DumpEE(slaveAddr, eeDT);
//
//	if (status != 0) return status;
//
//	status = MLX90640_ExtractParameters(eeDT, &mlx90640);
//
//	if(status != 0) return status;
//
//	status = MLX90640_SetChessMode(slaveAddr);
//
//	if(status != 0) return status;
//
//	status = MLX90640_SetResolution(slaveAddr, MLX90640_RES_16BIT);
//
//	if(status != 0) return status;
//
//	status = MLX90640_SetRefreshRate(slaveAddr, MLX90640_REFRESH_RATE_4HZ);
//
//	if(status != 0) return status;
//
//
//	return 0;
//}
//
//extern int MLX90640_I2CRead(uint8_t slaveAddr, uint16_t startAddress, uint16_t nMemAddressRead, uint16_t *data) {
//	int status = 0;
//	uint8_t buffer[2 * nMemAddressRead];
//
//	status = HAL_I2C_Mem_Read(&hi2c2, (uint16_t)(slaveAddr << 1), startAddress, I2C_MEMADD_SIZE_16BIT, buffer, sizeof(buffer), 500);
//	if (status != HAL_OK) {
//		return -1;
//	}
//
//    for (uint16_t i = 0; i < nMemAddressRead; i++) {
//        data[i] = ((uint16_t)buffer[2*i] << 8) | buffer[2*i + 1];
//    }
//	return 0;
//}
//
//extern int MLX90640_I2CWrite(uint8_t slaveAddr,uint16_t writeAddress, uint16_t data) {
//	int status = 0;
//	uint8_t cmd[2];
//
//    cmd[0] = (uint8_t)(data >> 8);
//    cmd[1] = (uint8_t)(data & 0xFF);
//
//	status = HAL_I2C_Mem_Write(&hi2c2, (uint16_t)(slaveAddr << 1), writeAddress, I2C_MEMADD_SIZE_16BIT, cmd, sizeof(cmd), 500);
//
//	if (status != HAL_OK) {
//		return -1;
//	}
//	return 0;
//}
//
//extern void MLX90640_I2CFreqSet(int freq) {
//
//}
//
//
//
//int MLX90640_read(uint16_t slaveAddr, uint16_t *frame, float *image, float emissivity)
//{
//
//
//    int status = 0;
//    float tr = 0;
//	float ta;
//    //float ta = 10;
//    status = MLX90640_SynchFrame(slaveAddr);
//    if (status < 0) {
//    	char m[64];
//    	snprintf(m,64,"Synch1 err=%d\r\n", status);
//    	uart_print(m);
//    	return status;
//    }
//    status = MLX90640_GetFrameData(slaveAddr, frame);
//    if (status < 0) {
//    	char m[64];
//    	snprintf(m,64,"Synch2 err=%d\r\n", status);
//    	uart_print(m);
//    	return status;
//    }
//
//    int sp0 = MLX90640_GetSubPageNumber(frame);
//
//    char m0[32];
//    snprintf(m0,sizeof(m0),"sp0=%d\r\n", sp0);
//    uart_print(m0);
//    ta = MLX90640_GetTa(frame, &mlx90640);
//	tr = ta - TA_SHIFT;
//
//	MLX90640_CalculateTo(frame, &mlx90640, emissivity, tr, image);
//
//	HAL_Delay(250);
//
//    status = MLX90640_GetFrameData(slaveAddr, frame);
//    if (status < 0) {
//    	char m[64];
//    	snprintf(m,64,"Synch3 err=%d\r\n", status);
//    	uart_print(m);
//    	return status;
//    }
//    int sp1 = MLX90640_GetSubPageNumber(frame);
//
//    char m1[32]; snprintf(m1,sizeof(m1),"sp1=%d\r\n", sp1); uart_print(m1);
//
//    if (sp1 != sp0) {
//    	ta = MLX90640_GetTa(frame, &mlx90640);
//    	tr = ta - TA_SHIFT;
//
//    	MLX90640_CalculateTo(frame, &mlx90640, emissivity, tr, image);
//    }
//    else {
//    	char m3[64];
//    	snprintf(m3,64,"Error subpage\r\n");
//    	uart_print(m3);
//    	return -5;
//    }
//
//    HAL_Delay(250);
//
//
//    return 0;
//}

