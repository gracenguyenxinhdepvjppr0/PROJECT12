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
#include "sensors/SHT30.h"
#include "sensors/MLX90640/MLX90640_API.h"
#include "sensors/MLX90640/MLX90640_I2C_Driver.h"
#include "OLED/fonts.h"
#include "OLED/SH1106.h"
#include "Ethernet/W5500_MQTT.h"
#include "MQTTClient.h"
#include "arm_math.h"
#include "stdlib.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MLX90640_ADDR 0x33
#define BUFFER_SIZE 1024
#define FFT_SAMPLES 512 // Bằng đúng một nửa BUFFER_SIZE (do dùng Half-Transfer)

#define SHT30_SOCKET 		0
#define MLX90640_SOCKET   	1
#define SHT30_TOKEN   		"SHT30"
#define MLX90640_TOKEN   	"MLX90640"
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;
I2C_HandleTypeDef hi2c3;

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
uint8_t dp = 0;

uint16_t eeMLX90640[832];       // Mảng chứa dữ liệu EEPROM
uint16_t frame[834];            // Mảng chứa dữ liệu thô của 1 khung hình
float MLX90640To[768];          // Mảng chứa nhiệt độ (độ C) của 32x24 pixels
paramsMLX90640 mlx90640Params;	// Chứa tham số calib
char txBuff[6000];

__attribute__((aligned(32))) uint16_t adc_buffer[BUFFER_SIZE];
float dc_offset = 2048.0f;
uint32_t spark_count = 0;

// ----- CÁC BIẾN CHO FFT -----
arm_rfft_fast_instance_f32 fft_handler; // Biến cấu hình bộ FFT
float32_t fft_input[FFT_SAMPLES];       // Mảng chứa tín hiệu âm thanh đầu vào (số thực)
float32_t fft_output[FFT_SAMPLES];      // Mảng chứa đầu ra của FFT (dạng số phức: thực/ảo xen kẽ)
float32_t fft_mag[FFT_SAMPLES / 2];     // Mảng chứa năng lượng (Magnitude) của 256 dải tần

volatile float32_t current_spark_energy = 0.0f; // Biến toàn cục để xem trên Live Expressions

Network network;
MQTTClient client;
uint8_t sendbuf[6000], readbuf[512];
uint8_t mac_addr[] = {0x00, 0x08, 0xdc, 0x11, 0x22, 0x33};

uint8_t broker_ip[4] = {0, 0, 0, 0};

Network net_sht30, net_mlx;
MQTTClient client_sht30, client_mlx;
// Buffer cho SHT30
unsigned char sendbuf_sht30[256], readbuf_sht30[256];
// Buffer cho MLX90640
unsigned char sendbuf_mlx[6500], readbuf_mlx[256];

uint8_t is_dns_resolved = 0;
uint8_t sht30_connected = 0;
uint8_t mlx_connected = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C2_Init(void);
static void MX_I2C3_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */
void MLX90640_Setup(void);
void MLX90640_Process(void);
void SHT30_Process(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t Connect_ThingsBoard(uint8_t sock_num, Network* net, MQTTClient* client, char* token, char* client_id)
{
    static uint16_t local_port = 30000;
    local_port++;
    if (local_port > 60000) local_port = 30000;

    disconnect(sock_num);
    close(sock_num);
    socket(sock_num, Sn_MR_TCP, local_port, 0);

    if (connect(sock_num, broker_ip, 1883) == SOCK_OK)
    {
        uint32_t tcp_timeout = HAL_GetTick();
        uint8_t tcp_ok = 0;
        // Đợi TCP Established
        while (HAL_GetTick() - tcp_timeout < 500)
        {
            if (getSn_SR(sock_num) == SOCK_ESTABLISHED) { tcp_ok = 1; break; }
            if (getSn_SR(sock_num) == SOCK_CLOSED) break;
            HAL_Delay(5);
        }

        if (tcp_ok == 1)
        {
            MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
            data.MQTTVersion = 3;
            data.clientID.cstring = client_id;
            data.username.cstring = token;
            data.password.cstring = NULL;

            if (MQTTConnect(client, &data) == MQTT_SUCCESS)
            {
                return 1; // Kết nối thành công
            }
        }
    }
    close(sock_num);
    return 0; // Kết nối thất bại
}

void SHT30_Process(void)
{
	SH1106_GotoXY(10,5);
	SH1106_Puts("Moi truong:", &Font_11x18,1);

	SHT30_Read(&temperature, &humidity);
	SH1106_GotoXY(10, 25);
	SH1106_Puts("Nhiet do: ", &Font_7x10, 1);
	SH1106_GotoXY(77, 25);
	sprintf(bufftemp, "%.2f", temperature);
	SH1106_Puts(bufftemp, &Font_7x10, 1);
	SH1106_GotoXY(10, 35);
	SH1106_Puts("Do am: ", &Font_7x10, 1);
	sprintf(buffhumid, "%.2f", humidity);
	SH1106_GotoXY(59, 35);
	SH1106_Puts(buffhumid, &Font_7x10, 1);
	float dew_point = SHT30_CalcDewPoint(temperature, humidity);
	float safety_margin = 3.0f; // Ngưỡng an toàn 3 độ C

	// Nếu nhiệt độ vỏ tủ tiến quá gần đến mức tạo sương
	if (temperature <= (dew_point + safety_margin))
	{
		dp = 1;
		SH1106_GotoXY(10, 30);
		SH1106_Puts("Canh bao:", &Font_7x10, 1);
		SH1106_GotoXY(10, 45);
		SH1106_Puts("Gan dat nguong diem suong", &Font_7x10, 1);
		HAL_Delay(15000);
	}

	SH1106_UpdateScreen();

	if (sht30_connected == 1)
	{
		char json_payload[100];
		sprintf(json_payload, "{\"temperature\":%.2f, \"humidity\":%.2f, \"dp\":%d}", temperature, humidity, dp);

		MQTTMessage pub_msg;
		memset(&pub_msg, 0, sizeof(pub_msg));
		pub_msg.qos = QOS0;
		pub_msg.payload = json_payload;
		pub_msg.payloadlen = strlen(json_payload);

		MQTTPublish(&client_sht30, "v1/devices/me/telemetry", &pub_msg);
	}
}

void MLX90640_Setup(void)
{
    int status;
    char buffer[80];

    MLX90640_SetChessMode(MLX90640_ADDR);
    // Thiết lập tốc độ làm tươi (Refresh Rate). Ví dụ: 0x03 = 4Hz, 0x04 = 8Hz
    MLX90640_SetRefreshRate(MLX90640_ADDR, 0x04);

    // Đọc toàn bộ bộ nhớ EEPROM của cảm biến.
    // Thử lại vài lần: nếu lỗi -3..-6 lặp lại y hệt ở mọi lần đọc -> rất có thể
    // là dữ liệu hiệu chuẩn gốc của cảm biến, không phải do nhiễu I2C.
    int retry;
    for (retry = 0; retry < 3; retry++)
    {
        status = MLX90640_DumpEE(MLX90640_ADDR, eeMLX90640);
        if (status == 0) break;

        sprintf(buffer, "Loi doc EEPROM (lan %d/3): %d\r\n", retry + 1, status);
        HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 50);
        HAL_Delay(10);
    }

    char debugMsg[100];
    sprintf(debugMsg, "EEPROM[0]=0x%04X | EEPROM[1]=0x%04X\r\n", eeMLX90640[0], eeMLX90640[1]);
    HAL_UART_Transmit(&huart1, (uint8_t*)debugMsg, strlen(debugMsg), 100);

    if (status != 0)
    {
        // -1/-2: lỗi I2C thật sự (NACK/timeout) -> không có dữ liệu hợp lệ, dừng hẳn
        // thay vì chạy tiếp với mảng eeMLX90640 rỗng/rác.
        sprintf(buffer, "Loi doc EEPROM sau 3 lan, dung lai!\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 50);
        while (1) { HAL_Delay(500); }
    }

    // Trích xuất các hệ số hiệu chuẩn vào struct
    status = MLX90640_ExtractParameters(eeMLX90640, &mlx90640Params);
    if (status != 0)
    {
        // -3 (>4 broken pixel), -4 (>4 outlier), -5 (tong > 4), -6 (2 pixel loi ke nhau):
        // đây chỉ là CẢNH BÁO về vài pixel trong EEPROM, các hệ số hiệu chuẩn khác
        // (Vdd, Ta, Gain, Offset, Alpha...) vẫn được tính đầy đủ phía trên hàm này.
        // KHÔNG dừng chương trình vì lỗi này - nếu dừng, sẽ không bao giờ có ảnh.
        sprintf(buffer, "Canh bao trich xuat (khong chan chuong trinh): %d\r\n", status);
        HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 50);
    }
}

// Đọc khung hình và gửi qua UART
void MLX90640_Process(void)
{
    int subpage;
    float ta, tr, vdd;
    float emissivity = 0.95; // Hệ số phát xạ

    // Đọc dữ liệu thô từ cảm biến
    subpage = MLX90640_GetFrameData(MLX90640_ADDR, frame);
    // DEBUG TẠM THỜI: in subpage ở MỌI lần gọi (kể cả khi hợp lệ) để biết
    // vòng lặp có đang chạy hay bị "treo" bên trong MLX90640_GetFrameData().
    // Xóa khối này sau khi xác định được nguyên nhân.

        char dbg[32];
        int n = sprintf(dbg, "[Frame] subpage=%d\r\n", subpage);
        HAL_UART_Transmit(&huart1, (uint8_t*)dbg, n, 50);


    if (subpage < 0) {
        char errBuf[50];
        sprintf(errBuf, "Loi Frame: %d\r\n", subpage);
        HAL_UART_Transmit(&huart1, (uint8_t*)errBuf, strlen(errBuf), 100);
    }
    if (subpage >= 0)
    { // Đọc thành công
    	vdd = MLX90640_GetVdd(frame, &mlx90640Params);
    	ta = MLX90640_GetTa(frame, &mlx90640Params);
    	tr = ta - 8.0f;
        // Tính toán ra nhiệt độ tuyệt đối (To)
        MLX90640_CalculateTo(frame, &mlx90640Params, emissivity, tr, MLX90640To);

        if (subpage == 1)
        {
        	int offset = 0;
        	// ThingsBoard yêu cầu chuẩn JSON. Bạn nên bọc mảng của bạn lại như sau:
        	offset += sprintf(txBuff + offset, "{\"thermal_pixels\": \"\n");

        	for (int i = 0; i < 768; i++)
        	{
        		offset += sprintf(txBuff + offset, "%5.2f ", MLX90640To[i]);
        		if ((i + 1) % 32 == 0)
        		{
        			offset += sprintf(txBuff + offset, "\n");
        		}
        	}
        	offset += sprintf(txBuff + offset, "\"}"); // Đóng ngoặc JSON

        	HAL_UART_Transmit(&huart1, (uint8_t*)txBuff, offset, HAL_MAX_DELAY);

        	// Đẩy dữ liệu ảnh nhiệt lên mạng
        	if (mlx_connected == 1)
        	{
        		MQTTMessage pub_msg;
        		memset(&pub_msg, 0, sizeof(pub_msg));
        		pub_msg.qos = QOS0;
        		pub_msg.payload = txBuff;
        		pub_msg.payloadlen = offset;

        		MQTTPublish(&client_mlx, "v1/devices/me/telemetry", &pub_msg);
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
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_I2C3_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_Delay(10);
  SH1106_Init();
  SH1106_Clear();
  SH1106_UpdateScreen();

  uint32_t last_sht30_time = 0;
  uint32_t last_mlx_time = 0;
  uint32_t last_network_time = 0;
  uint32_t last_mqtt_ping_time = 0;

  W5500_Hardware_Init();

  uint32_t last_dhcp_log = 0; // tránh spam log DHCP quá nhanh

  W5500_DHCP_Init(mac_addr);
  NetworkInit(&net_sht30, SHT30_SOCKET);
  NetworkInit(&net_mlx, MLX90640_SOCKET);
  MQTTClientInit(&client_sht30, &net_sht30, 8000, sendbuf_sht30, sizeof(sendbuf_sht30), readbuf_sht30, sizeof(readbuf_sht30));
//  MLX90640_Setup();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  uint8_t has_ip = W5500_DHCP_Run();

	  if (HAL_GetTick() - last_sht30_time >= 1000)
	  {
		  last_sht30_time = HAL_GetTick();
		  SHT30_Process();
	  }

//	  if (HAL_GetTick() - last_mlx_time >= 10)
//	  {
//		  last_mlx_time = HAL_GetTick();
//		  MLX90640_Process();
//	  }

	  if (HAL_GetTick() - last_network_time >= 2000)
	  {
		  last_network_time = HAL_GetTick();

		  if (has_ip == 1) // Đã lấy được IP từ DHCP
		  {
			  if (is_dns_resolved == 0)
			  {
				  if (W5500_DNS_Resolve("eu.thingsboard.cloud", broker_ip) == 1)
				  {
					  is_dns_resolved = 1;
				  }
			  }

			  if (is_dns_resolved == 1 && sht30_connected == 0)
			  {
				  sht30_connected = Connect_ThingsBoard(SHT30_SOCKET, &net_sht30, &client_sht30, SHT30_TOKEN, "SHT30_Sensor");
			  }
		  }
		  else
		  {
			  is_dns_resolved = 0;
			  sht30_connected = 0;
		  }
	  }

	  if (sht30_connected == 1)
	  {
		  if (HAL_GetTick() - last_mqtt_ping_time >= 100)
		  {
			  last_mqtt_ping_time = HAL_GetTick();

			  // MQTTYield cần được gọi liên tục để duy trì mạng và nhận dữ liệu
			  if (MQTTYield(&client_sht30, 10) != MQTT_SUCCESS)
			  {
				  sht30_connected = 0; // Đánh dấu rớt mạng
			  }
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
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 60;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 5;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc1.Init.Resolution = ADC_RESOLUTION_16B;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_CIRCULAR;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
  hadc1.Init.OversamplingMode = DISABLE;
  hadc1.Init.Oversampling.Ratio = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  sConfig.OffsetSignedSaturation = DISABLE;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

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
  hi2c1.Init.Timing = 0x00B03FDB;
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
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x307075B1;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief I2C3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C3_Init(void)
{

  /* USER CODE BEGIN I2C3_Init 0 */

  /* USER CODE END I2C3_Init 0 */

  /* USER CODE BEGIN I2C3_Init 1 */

  /* USER CODE END I2C3_Init 1 */
  hi2c3.Instance = I2C3;
  hi2c3.Init.Timing = 0x307075B1;
  hi2c3.Init.OwnAddress1 = 0;
  hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c3.Init.OwnAddress2 = 0;
  hi2c3.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c3) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c3, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c3, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C3_Init 2 */

  /* USER CODE END I2C3_Init 2 */

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
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
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
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);

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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PE15 */
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

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
