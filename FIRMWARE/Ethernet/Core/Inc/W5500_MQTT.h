/*
 * W5500_MQTT.h
 *
 *  Created on: 22 thg 5, 2026
 *      Author: Admin
 */

#ifndef INC_W5500_MQTT_H_
#define INC_W5500_MQTT_H_

#include "main.h"
#include "Ethernet/wizchip_conf.h"
#include "Ethernet/socket.h"
#include "Ethernet/dhcp.h"
#include "Ethernet/dns.h"
#include <stdio.h>


/* =====================================================================
 * CẤU HÌNH PHẦN CỨNG
 * ===================================================================== */
extern SPI_HandleTypeDef hspi1;            // Khai báo biến SPI từ main.c
#define W5500_SPI_HANDLE    &hspi1         // Bộ SPI đang dùng
#define W5500_CS_PORT       GPIOA          // Port của chân CS
#define W5500_CS_PIN        GPIO_PIN_3     // Chân CS

/* Cấu hình Mạng & Socket */
#define DHCP_SOCKET         7              // Socket dành riêng cho DHCP
#define MQTT_SOCKET         1              // Socket dành cho MQTT
#define DNS_SOCKET          6              // Dùng Socket 6 cho phân giải tên miền
/* =====================================================================
 * CẤU TRÚC PAHO MQTT (Timer & Network)
 * ===================================================================== */
typedef struct Timer {
    uint32_t end_time;
} Timer;

void TimerInit(Timer* timer);
char TimerIsExpired(Timer* timer);
void TimerCountdownMS(Timer* timer, unsigned int ms);
void TimerCountdown(Timer* timer, unsigned int seconds);
int TimerLeftMS(Timer* timer);

typedef struct Network {
    uint8_t my_socket;
    int (*mqttread)(struct Network*, unsigned char*, int, int);
    int (*mqttwrite)(struct Network*, unsigned char*, int, int);
} Network;

void NetworkInit(Network* n, uint8_t socket_num);
void NetworkDisconnect(Network* n);

/* =====================================================================
 * CÁC HÀM KHỞI TẠO
 * ===================================================================== */
// Khởi tạo giao tiếp SPI với W5500
uint8_t W5500_Hardware_Init(void);

// Khởi tạo DHCP (Truyền vào địa chỉ MAC)
void W5500_DHCP_Init(uint8_t* mac_address);

// Hàm duy trì DHCP (Chạy liên tục trong while 1)
// Trả về 1 nếu đã có IP, 0 nếu đang mất mạng/đang xin IP
uint8_t W5500_DHCP_Run(void);

uint8_t W5500_DNS_Resolve(char* domain_name, uint8_t* resolved_ip);

#endif /* INC_W5500_MQTT_H_ */
