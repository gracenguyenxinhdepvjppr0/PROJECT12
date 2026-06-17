/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "W5500_MQTT.h"
#include "MQTTClient.h"
#include "MLX90640_API.h"
#include "MLX90640_I2C_Driver.h"
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MLX90640_ADDR 0x33
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
Network network;
MQTTClient client;
uint8_t sendbuf[6000], readbuf[512];
uint8_t mac_addr[] = {0x00, 0x08, 0xdc, 0x11, 0x22, 0x33};

uint8_t broker_ip[4] = {0, 0, 0, 0};

uint16_t eeMLX90640[832];       // Mảng chứa dữ liệu EEPROM
uint16_t frame[834];            // Mảng chứa dữ liệu thô của 1 khung hình
float MLX90640To[768];          // Mảng chứa nhiệt độ (độ C) của 32x24 pixels
paramsMLX90640 mlx90640Params;	// Chứa tham số calib
char txBuff[6000];

uint8_t is_dns_resolved = 0;
uint8_t is_mqtt_connected = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */
void MLX90640_Setup(void);
void MLX90640_Process_MQTT(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void MLX90640_Setup(void)
{
    int status;
    char buffer[50];

    MLX90640_SetChessMode(MLX90640_ADDR);
    // Thiết lập tốc độ làm tươi (Refresh Rate). Ví dụ: 0x03 = 4Hz, 0x04 = 8Hz
    MLX90640_SetRefreshRate(MLX90640_ADDR, 0x06);

    // Đọc toàn bộ bộ nhớ EEPROM của cảm biến
    status = MLX90640_DumpEE(MLX90640_ADDR, eeMLX90640);
    if (status != 0)
    {
        sprintf(buffer, "Loi doc EEPROM!\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 50);
    }

    // Trích xuất các hệ số hiệu chuẩn vào struct
    status = MLX90640_ExtractParameters(eeMLX90640, &mlx90640Params);
    if (status != 0)
    {
        sprintf(buffer, "Loi trich xuat tham so!\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 50);
    }
}

void MLX90640_Process_MQTT(void)
{
    int subpage;
    float ta, tr, vdd;
    float emissivity = 0.95;

    subpage = MLX90640_GetFrameData(MLX90640_ADDR, frame);

    if (subpage >= 0)
    {
    	vdd = MLX90640_GetVdd(frame, &mlx90640Params);
    	ta = MLX90640_GetTa(frame, &mlx90640Params);
    	tr = ta - 8.0f;
        MLX90640_CalculateTo(frame, &mlx90640Params, emissivity, tr, MLX90640To);

        // Chỉ gửi đi khi đã lấy đủ cả 2 subpage (Khung hình hoàn chỉnh)
        if (subpage == 1 && is_mqtt_connected == 1)
        {
        	int offset = 0;

        	// XÓA SẠCH BỘ ĐỆM CŨ TRƯỚC KHI GHI CHUỖI MỚI (CHỐNG TRÀN BỘ NHỚ)
        	memset(txBuff, 0, sizeof(txBuff));
            // 1. Mở đầu chuỗi JSON
			offset += sprintf(txBuff + offset, "{heatmap:[");

            // 2. Chèn 768 giá trị vào mảng
			for (int i = 0; i < 768; i++)
			{
                // Dùng %.1f để giữ 1 số thập phân (tiết kiệm dung lượng chuỗi)
                offset += sprintf(txBuff + offset, "%d", (int)(MLX90640To[i]*10.0f));
                if (i < 767) {
                    offset += sprintf(txBuff + offset, ","); // Thêm dấu phẩy giữa các số
                }
            }

            // 3. Đóng chuỗi JSON
			offset += sprintf(txBuff + offset, "]}");

            // 4. Gửi qua MQTT
            MQTTMessage pub_msg;
            memset(&pub_msg, 0, sizeof(pub_msg));
            pub_msg.qos = QOS0;
            pub_msg.retained = 0;
            pub_msg.dup = 0;
            pub_msg.payload = txBuff;
            pub_msg.payloadlen = offset; // Kích thước chuỗi lúc này rơi vào khoảng 3800 bytes

            if (MQTTPublish(&client, "v1/devices/me/telemetry", &pub_msg) == MQTT_SUCCESS) {
                HAL_UART_Transmit(&huart1, (uint8_t*)"Gui anh nhiet OK!\r\n", 19, 100);
            } else {
                HAL_UART_Transmit(&huart1, (uint8_t*)"Gui anh nhiet FAILED!\r\n", 23, 100);
            }
        }
    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  MLX90640_Setup();

  char msg[] = "W5500 SPI Initializing...\r\n";
  HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 100);

  uint8_t v = getVERSIONR();
  if (W5500_Hardware_Init() == 1) {
        char msg_spi[] = "W5500 SPI Init OK!\r\n";
        HAL_UART_Transmit(&huart1, (uint8_t*)msg_spi, strlen(msg_spi), 100);
    } else {
        char msg_spi_err[] = "W5500 SPI Init FAILED!\r\n";
        HAL_UART_Transmit(&huart1, (uint8_t*)msg_spi_err, strlen(msg_spi_err), 100);
        sprintf(msg_spi_err, "SPI ERROR! Doc Version W5500 ra %d (Mong doi: 4)\r\n", v);
        HAL_UART_Transmit(&huart1, (uint8_t*)msg_spi_err, strlen(msg_spi_err), 100);
    }

  W5500_DHCP_Init(mac_addr);
  NetworkInit(&network, MQTT_SOCKET);
  MQTTClientInit(&client, &network, 8000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));

  char buf[100];

  uint32_t last_dhcp_log = 0; // tránh spam log DHCP quá nhanh
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  // Duy trì mạng DHCP
	  // Duy trì mạng DHCP
	        if (W5500_DHCP_Run() == 1)
	        {
	            // 1. DỊCH TÊN MIỀN
	            if (is_dns_resolved == 0)
	            {
	                char msg_dns1[] = "Converting ThingsBoard domain...\r\n";
	                HAL_UART_Transmit(&huart1, (uint8_t*)msg_dns1, strlen(msg_dns1), 100);

	                // CHÚ Ý: Đã đổi sang bản EU Cloud (thingsboard.cloud)
	                if (W5500_DNS_Resolve("eu.thingsboard.cloud", broker_ip) == 1)
	                {
	                    sprintf(buf, "DNS Success! IP: %d.%d.%d.%d\r\n", broker_ip[0], broker_ip[1], broker_ip[2], broker_ip[3]);
	                    HAL_UART_Transmit(&huart1, (uint8_t*)buf, strlen(buf), 100);
	                    is_dns_resolved = 1;
	                }
	                else
	                {
	                    char msg_dns_err[] = "DNS Error, retrying in 2s...\r\n";
	                    HAL_UART_Transmit(&huart1, (uint8_t*)msg_dns_err, strlen(msg_dns_err), 100);
	                    HAL_Delay(2000);
	                }
	            }

	            // 2. KẾT NỐI TCP & MQTT (Dùng 'if' thay cho 'else if' để chạy ngay lập tức)
	            if (is_dns_resolved == 1 && is_mqtt_connected == 0)
	            {
	                // Khởi tạo Port động chống kẹt Socket
	                static uint16_t local_port = 30000;
	                local_port++;
	                if (local_port > 60000) local_port = 30000;

	                disconnect(MQTT_SOCKET);
	                close(MQTT_SOCKET);
	                socket(MQTT_SOCKET, Sn_MR_TCP, local_port, 0);

	                if (connect(MQTT_SOCKET, broker_ip, 1883) == SOCK_OK)
	                {
	                    // Kiểm tra TCP Established
	                    uint32_t tcp_timeout = HAL_GetTick();
	                    uint8_t tcp_ok = 0;
	                    while (HAL_GetTick() - tcp_timeout < 5000) {
	                        if (getSn_SR(MQTT_SOCKET) == SOCK_ESTABLISHED) {
	                            tcp_ok = 1;
	                            break;
	                        }
	                        if (getSn_SR(MQTT_SOCKET) == SOCK_CLOSED) break;
	                        HAL_Delay(10);
	                    }

	                    if (tcp_ok == 1)
	                    {
	                        MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	                        data.MQTTVersion = 3;
	                        data.clientID.cstring = "STM32_EU_Cloud"; // Tên thiết bị
	                        data.username.cstring = "e1818hk2mp31yijnw1bs"; // ACCESS TOKEN CỦA BẠN
	                        data.password.cstring = NULL;

	                        if (MQTTConnect(&client, &data) == MQTT_SUCCESS)
	                        {
	                            is_mqtt_connected = 1;
	                            char msg_mqtt_ok[] = "Connected to ThingsBoard successfully!\r\n";
	                            HAL_UART_Transmit(&huart1, (uint8_t*)msg_mqtt_ok, strlen(msg_mqtt_ok), 100);
	                        }
	                        else
	                        {
	                            char msg_fail[] = "MQTT Connect FAILED!\r\n";
	                            HAL_UART_Transmit(&huart1, (uint8_t*)msg_fail, strlen(msg_fail), 100);
	                            close(MQTT_SOCKET);
	                            is_dns_resolved = 0;
	                        }
	                    }
	                    else
	                    {
	                        close(MQTT_SOCKET);
	                        is_dns_resolved = 0;
	                    }
	                }
	                else
	                {
	                    close(MQTT_SOCKET);
	                    is_dns_resolved = 0;
	                    HAL_Delay(10);
	                }
	            }

	            // 3. BẮT ĐẦU PUBLISH
	            if (is_mqtt_connected == 1)
	            {
	                if (MQTTYield(&client, 10) != MQTT_SUCCESS)
	                {
	                    is_mqtt_connected = 0;
	                    NetworkDisconnect(&network);
	                    char msg_drop[] = "MQTT Connection Lost!\r\n";
	                    HAL_UART_Transmit(&huart1, (uint8_t*)msg_drop, strlen(msg_drop), 100);
	                    continue;
	                }

	                // --- DỌN RÁC BỘ NHỚ VÀ ĐÓNG GÓI JSON ---
	                MLX90640_Process_MQTT();
	                HAL_Delay(200);
	            }
	        }
	        else
	        {
	            // Đang xin DHCP hoặc rớt mạng (Bây giờ nó đã nằm đúng vị trí!)
	            is_dns_resolved = 0;
	            is_mqtt_connected = 0;
	            // Báo "nhịp tim" mỗi 1 giây để chống mù thông tin
	            if (HAL_GetTick() - last_dhcp_log > 1000) {
	            	HAL_UART_Transmit(&huart1, (uint8_t*)"Dang xin IP tu Router...\r\n", 26, 100);
	            	last_dhcp_log = HAL_GetTick();
	            }
	        }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = 64;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 34;
  RCC_OscInitStruct.PLL.PLLP = 1;
  RCC_OscInitStruct.PLL.PLLQ = 3;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 3072;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00601A5C;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }

  /** I2C Enable Fast Mode Plus
  */
  HAL_I2CEx_EnableFastModePlus(I2C_FASTMODEPLUS_I2C1);
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 0x0;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  hspi1.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  hspi1.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  hspi1.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi1.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi1.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi1.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi1.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi1.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
  hspi1.Init.IOSwap = SPI_IO_SWAP_DISABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : CS_Pin RST_Pin */
  GPIO_InitStruct.Pin = CS_Pin|RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
