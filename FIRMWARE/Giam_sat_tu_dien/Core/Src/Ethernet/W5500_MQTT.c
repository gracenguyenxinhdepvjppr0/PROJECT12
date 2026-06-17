 /*
 * W5500_MQTT.c
 *
 *  Created on: 22 thg 5, 2026
 *      Author: Admin
 */
#include "Ethernet/W5500_MQTT.h"
#include "main.h"

static uint8_t dns_buffer[1024];
/* =====================================================================
 * GIAO TIẾP SPI VỚI STM32
 * ===================================================================== */
static void W5500_Select(void)
{
    HAL_GPIO_WritePin(W5500_CS_PORT, W5500_CS_PIN, GPIO_PIN_RESET);
}

static void W5500_Unselect(void)
{
    HAL_GPIO_WritePin(W5500_CS_PORT, W5500_CS_PIN, GPIO_PIN_SET);
}

static void W5500_WriteByte(uint8_t byte)
{
	uint8_t dummy_rx; // Biến rác để hứng dữ liệu trả về
	    // Gửi 1 byte đi, đồng thời đọc 1 byte về (nhưng bỏ qua)
	HAL_SPI_TransmitReceive(W5500_SPI_HANDLE, &byte, &dummy_rx, 1, 100);
}

static uint8_t W5500_ReadByte(void)
{
	uint8_t dummy_tx = 0xFF; // Gửi 1 byte rác (0xFF) để tạo xung Clock
	uint8_t rx_data;
	// Vừa gửi byte rác đi, vừa hứng byte dữ liệu thật về
	HAL_SPI_TransmitReceive(W5500_SPI_HANDLE, &dummy_tx, &rx_data, 1, 100);
	return rx_data;}

static void W5500_WriteBurst(uint8_t* pBuf, uint16_t len)
{
    HAL_SPI_Transmit(W5500_SPI_HANDLE, pBuf, len, 100);
}

static void W5500_ReadBurst(uint8_t* pBuf, uint16_t len)
{
    HAL_SPI_Receive(W5500_SPI_HANDLE, pBuf, len, 100);
}

uint8_t W5500_Hardware_Init(void)
{
    // Đăng ký callback
    reg_wizchip_cs_cbfunc(W5500_Select, W5500_Unselect);
    reg_wizchip_spi_cbfunc(W5500_ReadByte, W5500_WriteByte);
    reg_wizchip_spiburst_cbfunc(W5500_ReadBurst, W5500_WriteBurst);

    uint8_t version = getVERSIONR();
    if (version != 0x04)
    {

    	return 0; // Thất bại thực sự
    }
    // Cấu hình buffer 2KB cho 8 Socket
    uint8_t rx_tx_buff_sizes[] = {2, 2, 2, 2, 2, 2, 2, 2};
    if (wizchip_init(rx_tx_buff_sizes, rx_tx_buff_sizes) != 0)
    {
        return 0; // Thất bại
    }
    return 1; // Thành công
}

/* =====================================================================
 * CÁC HÀM PAHO MQTT (Timer & Network)
 * ===================================================================== */
void TimerInit(Timer* timer)
{
	timer->end_time = 0;
}

char TimerIsExpired(Timer* timer)
{
	int32_t left = (int32_t)(timer->end_time - HAL_GetTick());
	return (left <= 0);
}

void TimerCountdownMS(Timer* timer, unsigned int ms)
{
	timer->end_time = HAL_GetTick() + ms;
}

void TimerCountdown(Timer* timer, unsigned int seconds)
{
	timer->end_time = HAL_GetTick() + (seconds * 1000);
}

int TimerLeftMS(Timer* timer)
{
	int32_t left = (int32_t)(timer->end_time - HAL_GetTick());
	return (left < 0) ? 0 : left;
}

static int mqtt_read(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
    Timer timer;
    TimerCountdownMS(&timer, timeout_ms);
    int received_bytes = 0;

    while (received_bytes < len)
    {
        uint16_t rx_len = getSn_RX_RSR(n->my_socket);
        if (rx_len > 0)
        {
            int max_len = (len - received_bytes < rx_len) ? (len - received_bytes) : rx_len;
            int rc = recv(n->my_socket, buffer + received_bytes, max_len);
            if (rc > 0) received_bytes += rc;
            else if (rc < 0) return rc;
        }
        if (TimerIsExpired(&timer)) break;
    }
    return received_bytes;
}

static int mqtt_write(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
    Timer timer;
    TimerCountdownMS(&timer, timeout_ms);
    int sent_bytes = 0;

    while (sent_bytes < len)
    {
        int rc = send(n->my_socket, buffer + sent_bytes, len - sent_bytes);
        if (rc > 0) sent_bytes += rc;
        else if (rc < 0) return rc;
        if (TimerIsExpired(&timer)) break;
    }
    return sent_bytes;
}

void NetworkInit(Network* n, uint8_t socket_num)
{
    n->my_socket = socket_num;
    n->mqttread = mqtt_read;
    n->mqttwrite = mqtt_write;
}

void NetworkDisconnect(Network* n)
{
    disconnect(n->my_socket);
    close(n->my_socket);
}

/* =====================================================================
 * QUẢN LÝ DHCP (Tự động lấy IP)
 * ===================================================================== */
static uint8_t dhcp_buffer[1024];
static uint8_t is_ip_assigned = 0;
static uint8_t user_mac[6];

static void cb_ip_assign(void)
{
    wiz_NetInfo netinfo;
    getIPfromDHCP(netinfo.ip);
    getGWfromDHCP(netinfo.gw);
    getSNfromDHCP(netinfo.sn);
    getDNSfromDHCP(netinfo.dns);
    for(int i=0; i<6; i++) netinfo.mac[i] = user_mac[i];
    netinfo.dhcp = NETINFO_DHCP;
    wizchip_setnetinfo(&netinfo);
    is_ip_assigned = 1;
}

static void cb_ip_conflict(void)
{
	is_ip_assigned = 0;
}

void W5500_DHCP_Init(uint8_t* mac_address)
{
    for(int i=0; i<6; i++) user_mac[i] = mac_address[i];
    setSHAR(user_mac); // Gán MAC cứng
    reg_dhcp_cbfunc(cb_ip_assign, cb_ip_assign, cb_ip_conflict);
    DHCP_init(DHCP_SOCKET, dhcp_buffer);
}

uint8_t W5500_DHCP_Run(void)
{
    static uint32_t timer_1s = 0;

    uint8_t dhcp_state = DHCP_run();
    if (dhcp_state == DHCP_FAILED) is_ip_assigned = 0;

    return is_ip_assigned;
}

uint8_t W5500_DNS_Resolve(char* domain_name, uint8_t* resolved_ip)
{
    wiz_NetInfo netinfo;

    // Lấy thông tin mạng hiện tại
    wizchip_getnetinfo(&netinfo);

    // Khởi tạo bộ đệm cho DNS
    DNS_init(DNS_SOCKET, dns_buffer);

    // Bắt đầu dịch tên miền
    int8_t ret = DNS_run(netinfo.dns, (uint8_t*)domain_name, resolved_ip);
    if (ret == 1)
    {
        return 1; // Thành công
    }
    return 0; // Thất bại (Do sai tên miền hoặc không có mạng Internet)
}
