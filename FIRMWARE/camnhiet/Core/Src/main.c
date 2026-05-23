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
#include "MLX90640_API.h"
#include "MLX90640_I2C_Driver.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
extern I2C_HandleTypeDef *mlx_i2c;
extern UART_HandleTypeDef *mlx_usart;
extern void send_frame_uart(float *frame);
/* USER CODE BEGIN PV */
paramsMLX90640 mlx90640;  // Thêm biến này
volatile  uint16_t eeDT[832];       // Sửa 768 thành 832
volatile  uint16_t frame[834];      // Thêm mảng đọc frame
volatile float image[768];         // Sửa uint16_t thành float
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void uart_print(const char *s) {
    HAL_UART_Transmit(&huart1, (uint8_t*)s, strlen(s), 125);
}
//
//int MLX90640_init(uint16_t slaveAddr, uint16_t *eeDT) {
//
//	int status;
//
//	status = MLX90640_DumpEE(slaveAddr, eeDT);
//
//	if (status != 0) return status;
//
//	status = MLX90640_ExtractParameters(eeDT, &hi2c1);
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
int MLX90640_init(uint16_t slaveAddr, uint16_t *eeDT) {
	int status;
	status = MLX90640_DumpEE(slaveAddr, eeDT);
	if (status != 0) return status;

	// Sửa dòng này: đổi &hi2c1 thành &mlx90640
	status = MLX90640_ExtractParameters(eeDT, &mlx90640);

	if(status != 0) return status;
	status = MLX90640_SetChessMode(slaveAddr);
	if(status != 0) return status;
	status = MLX90640_SetResolution(slaveAddr, MLX90640_RES_16BIT);
	if(status != 0) return status;
	status = MLX90640_SetRefreshRate(slaveAddr, MLX90640_REFRESH_RATE_4HZ);
	if(status != 0) return status;

	return 0;
}
int MLX90640_read(uint16_t slaveAddr, uint16_t *frame, float *image, float emissivity)
{


    int status = 0;
    float tr = 0;
	float ta;
    //float ta = 10;
    status = MLX90640_SynchFrame(slaveAddr);
    if (status < 0) {
    	char m[64];
    	snprintf(m,64,"Synch1 err=%d\r\n", status);
    	uart_print(m);
    	return status;
    }
    status = MLX90640_GetFrameData(slaveAddr, frame);
    if (status < 0) {
    	char m[64];
    	snprintf(m,64,"Synch2 err=%d\r\n", status);
    	uart_print(m);
    	return status;
    }

    int sp0 = MLX90640_GetSubPageNumber(frame);

    char m0[32];
    snprintf(m0,sizeof(m0),"sp0=%d\r\n", sp0);
    uart_print(m0);
    ta = MLX90640_GetTa(frame, &mlx90640);
	tr = ta - TA_SHIFT;

	MLX90640_CalculateTo(frame, &mlx90640, emissivity, tr, image);

	//HAL_Delay(250);

    status = MLX90640_GetFrameData(slaveAddr, frame);
    if (status < 0) {
    	char m[64];
    	snprintf(m,64,"Synch3 err=%d\r\n", status);
    	uart_print(m);
    	return status;
    }
    int sp1 = MLX90640_GetSubPageNumber(frame);

    char m1[32]; snprintf(m1,sizeof(m1),"sp1=%d\r\n", sp1); uart_print(m1);

    if (sp1 != sp0) {
    	ta = MLX90640_GetTa(frame, &mlx90640);
    	tr = ta - TA_SHIFT;

    	MLX90640_CalculateTo(frame, &mlx90640, emissivity, tr, image);
    }
    else {
    	char m3[64];
    	snprintf(m3,64,"Error subpage\r\n");
    	uart_print(m3);
    	return -5;
    }

  //  HAL_Delay(250);


    return 0;
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
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  mlx_i2c = &hi2c1;
  mlx_usart = &huart1;

  // Bỏ dấu & ở biến eeDT đi vì nó đã là mảng
  MLX90640_init(0x33, eeDT);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  MLX90640_read(0x33, frame, image, 0.95);
	            // Gửi dữ liệu qua UART sau khi đọc thành công cả 2 subpage
	  send_frame_uart(image);

	 HAL_Delay(2000);
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 100;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

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
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

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
