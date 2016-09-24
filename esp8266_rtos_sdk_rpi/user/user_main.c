/**
 * @file user_main.c
 *
 */
/* Copyright (C) 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdbool.h>
#include <stdint.h>

#include <esp_common.h>
#include <freertos/task.h>
#include <lwip/sockets.h>
#include <upgrade.h>

#include "esp8266_cmd.h"
#include "esp8266_peri.h" //TODO replace by SDK defines
#include "esp8266_rpi.h"

//#define DEBUG

#if !defined(DEBUG)
#define printf(...) (void *)0
#endif

/************************************************************************
*
*
************************************************************************/
#define WIFI_AP_PASSWORD	""	//TODO
#define OTA_UPDATE_PORT		8888

typedef enum conn_state {
	WIFI_NOT_CONNECTED,
	WIFI_CONNECTING,
	WIFI_CONNECTING_ERROR,
	WIFI_CONNECTED
} _conn_state;

/************************************************************************
*
*
************************************************************************/
LOCAL const uint8_t gc_zero = (uint8_t) 0;
LOCAL char *g_sdk_version;
LOCAL uint32_t g_sdk_version_length;
LOCAL uint8_t g_cpu_freq;
LOCAL uint8_t g_hwmac[6];
LOCAL struct ip_info g_ipconfig;
LOCAL char *g_host_name;
LOCAL uint32_t g_host_name_length;
LOCAL int32 g_sock_fd;
LOCAL int16_t g_port_udp_begin;
LOCAL xTaskHandle g_task_udp_handle = NULL;
LOCAL xTaskHandle g_task_rpi_handle = NULL;
LOCAL uint8_t g_head = 0;
LOCAL uint8_t g_tail = 0;
LOCAL char *g_update_file = NULL;
LOCAL const char *g_user1_bin = "user1.bin";
LOCAL const char *g_user2_bin = "user2.bin";

#define DATA_BUFFER_INDEX_ENTRIES     (1 << 3)
#define DATA_BUFFER_INDEX_MASK        (DATA_BUFFER_INDEX_ENTRIES - 1)

#define UDP_BUFFER_SIZE	800

LOCAL int8_t g_sendto_buffer[UDP_BUFFER_SIZE];

struct _udp_data {
  uint16_t len;
  uint32_t ip_address;
  uint16_t port;
  char data[UDP_BUFFER_SIZE];
} LOCAL g_udp_receive[DATA_BUFFER_INDEX_ENTRIES];


uint32 ICACHE_FLASH_ATTR user_rf_cal_sector_set(void)
{
	flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 8;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}

/**
 *
 */
static inline void data_gpio_fsel_input(void) {
  GPEC = (uint32_t)(0x0F << 12);
}

/**
 *
 * @return
 */
inline static uint8_t _read_byte(void) {
	while (!(GPI & (uint32_t) (1 << ESP8266_RPI_CTRL_IN))) {
	}

	GPOS = (uint32_t) (1 << ESP8266_RPI_CTRL_OUT);

	uint8_t data = (GPI >> 12) & 0x0F;

	while (GPI & (uint32_t) (1 << ESP8266_RPI_CTRL_IN)) {
	}

	GPOC = (uint32_t) (1 << ESP8266_RPI_CTRL_OUT);

	while (!(GPI & (uint32_t) (1 << ESP8266_RPI_CTRL_IN))) {
	}

	GPOS = (uint32_t) (1 << ESP8266_RPI_CTRL_OUT);
	data |= ((GPI >> 12) & 0x0F) << 4;

	while (GPI & (uint32_t) (1 << ESP8266_RPI_CTRL_IN)) {
	}

	GPOC = (uint32_t) (1 << ESP8266_RPI_CTRL_OUT);

	return data;
}

/**
 *
 * @return
 */
uint8_t IRAM_ATTR rpi_read_4bits(void) {
	data_gpio_fsel_input();

	while (!(GPI & (uint32_t) (1 << ESP8266_RPI_CTRL_IN))) {
	}

	GPOS = (uint32_t) (1 << ESP8266_RPI_CTRL_OUT);

	uint8_t data = (GPI >> 12) & 0x0F;

	while (GPI & (uint32_t) (1 << ESP8266_RPI_CTRL_IN)) {
	}

	GPOC = (uint32_t) (1 << ESP8266_RPI_CTRL_OUT);

	return data;
}

/**
 *
 * @return
 */
/*static IRAM_ATTR uint8_t IRAM_ATTR rpi_read_byte(void) {
	data_gpio_fsel_input();
	return _read_byte();
}*/

/**
 *
 * @param p
 * @param nLength
 */
static void IRAM_ATTR rpi_read_bytes(uint8_t *p, uint16_t nLength) {
	uint16_t i;
	uint8_t *_p = p;

	data_gpio_fsel_input();

	for (i = 0; i < nLength; i++) {
		*_p = _read_byte();
		_p++;
	}
}

/**
 *
 * @param
 */
static uint16_t IRAM_ATTR rpi_read_halfword(void) {
	uint16_t data;

	data_gpio_fsel_input();

	data = _read_byte();
	data |= _read_byte() << 8;

	return data;
}

/**
 *
 * @param
 */
static uint32_t IRAM_ATTR rpi_read_word(void) {
	uint32_t data;

	data_gpio_fsel_input();

	data = _read_byte();
	data |= _read_byte() << 8;
	data |= _read_byte() << 16;
	data |= _read_byte() << 24;

	return data;
}

/**
 *
 */
static inline void data_gpio_fsel_output(void) {
  GPES = (uint32_t)(0x0F << 12);
}

/**
 *
 * @param p
 * @param nLength
 */
static void IRAM_ATTR rpi_write_bytes(uint8_t *p, uint16_t nLength) {
	uint16_t i;
	uint8_t *_p = p;
	uint32_t out_gpio;

	data_gpio_fsel_output();

	for (i = 0; i < nLength; i++) {
		// bit 0,1,2,3
		while (!(GPI & (uint32_t) (1 << ESP8266_RPI_CTRL_IN))) {
		}
		uint8_t data = *_p;
		out_gpio = (data & 0x0F) << 12;
		GPOS = out_gpio;
		GPOC = out_gpio ^ (0x0F << 12);
		GPOS = (uint32_t) (1 << ESP8266_RPI_CTRL_OUT);
		while (GPI & (uint32_t) (1 << ESP8266_RPI_CTRL_IN)) {
		}
		GPOC = (uint32_t) (1 << ESP8266_RPI_CTRL_OUT);
		// bit 4,5,6,7
		while (!(GPI & (uint32_t) (1 << ESP8266_RPI_CTRL_IN))) {
		}
		out_gpio = ((data >> 4) & 0x0F) << 12;
		GPOS = out_gpio;
		GPOC = out_gpio ^ (0x0F << 12);
		GPOS = (uint32_t) (1 << ESP8266_RPI_CTRL_OUT);
		while (GPI & (uint32_t) (1 << ESP8266_RPI_CTRL_IN)) {
		}
		GPOC = (uint32_t) (1 << ESP8266_RPI_CTRL_OUT);
		_p++;
	}
}

/**
 *
 * @param pvParameters
 */
void IRAM_ATTR task_udp(void *pvParameters) {
	int32 ret;
	struct sockaddr_in server_addr, address_remote;
	int slen = sizeof(address_remote);

	printf("udp-interface, port : %d\n", g_port_udp_begin);

    do {
		g_sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (g_sock_fd == -1) {
			printf("ERROR: Failed to create sock!\n");
			vTaskDelay(1000 / portTICK_RATE_MS);
		}
	} while (g_sock_fd == -1);

    memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(g_port_udp_begin);
	server_addr.sin_len = sizeof(server_addr);

    do {
		ret = bind(g_sock_fd, (struct sockaddr * )&server_addr, sizeof(server_addr));
		if (ret != 0) {
			printf("ERROR: Failed to bind sock! - %d\n", ret);
			vTaskDelay(1000 / portTICK_RATE_MS);
		}
	} while (ret != 0);

	while (1) {
		size_t len;
		if ((len = recvfrom(g_sock_fd, g_udp_receive[g_head].data, UDP_BUFFER_SIZE, 0, (struct sockaddr * )&address_remote, &slen)) == -1) {
			printf("ERROR: recvfrom!\n");
		} else {
			//printf("%d:%d\n", g_head, len); //TODO DEBUG
			g_udp_receive[g_head].len = len;
			g_udp_receive[g_head].ip_address = address_remote.sin_addr.s_addr;
			g_udp_receive[g_head].port = address_remote.sin_port;
			g_head = (g_head + 1) & DATA_BUFFER_INDEX_MASK;
		}
	}

	g_task_udp_handle = NULL;
	vTaskDelete(NULL);
}

/**
 *
 * @param lhs
 * @param rhs
 * @return
 */
static bool softap_config_equal(const struct softap_config *lhs, const struct softap_config *rhs) {
    if(strcmp(lhs->ssid, rhs->ssid) != 0) {
        return false;
    }

    if(strcmp(lhs->password, rhs->password) != 0) {
        return false;
    }

    if(lhs->channel != rhs->channel) {
        return false;
    }

    if(lhs->ssid_hidden != rhs->ssid_hidden) {
        return false;
    }
    return true;
}

/**
 *
 * @param evt
 */
void wifi_event_cb(System_Event_t *evt) {
	printf("WiFi event : %d\n", evt->event_id);

	switch (evt->event_id) {
	case EVENT_STAMODE_CONNECTED:
		printf("ESP8266 station connected to AP\n");
		if(wifi_get_opmode() != STATION_MODE) {
			wifi_station_disconnect();
			wifi_station_dhcpc_stop();
			wifi_set_opmode(SOFTAP_MODE);
		}
		break;
	case EVENT_STAMODE_DISCONNECTED:
		printf("ESP8266 station disconnected from AP\n");
		break;
	case EVENT_STAMODE_GOT_IP:
		printf("ESP8266 station got IP from connected AP\n");
		break;
	case EVENT_SOFTAPMODE_PROBEREQRECVED:
		printf("Receive probe request packet in soft-AP interface\n");
		break;
	default:
		break;
	}
}

/**
 *
 */
void setup_wifi_ap_mode(void) {
	struct softap_config config_new;
    struct softap_config config_current;
    struct ip_info ip;

    wifi_set_opmode(STATIONAP_MODE);

	memset(config_new.ssid, 0, sizeof(config_new.ssid));			/* SSID of ESP8266 soft-AP */
	memset(config_new.password, 0, sizeof(config_new.password));	/* Password of ESP8266 soft-AP */

	config_new.ssid_hidden = 0;				/* Broadcast the SSID */
	config_new.channel = 7;					/* Channel of ESP8266 soft-AP */
	config_new.max_connection = 2;			/* Max number of stations allowed to connect in, max 4 */
	config_new.ssid_len = sprintf(config_new.ssid, "AvV-ArtNet-%X", system_get_chip_id());

	if (strlen(WIFI_AP_PASSWORD) == 0) {
		config_new.authmode = AUTH_OPEN;	/* authenticate mode : open */
		*config_new.password = 0;
	} else {
		config_new.authmode = AUTH_WPA_WPA2_PSK;
		strcpy(config_new.password, WIFI_AP_PASSWORD);
	}

    wifi_softap_get_config(&config_current);

	if (!softap_config_equal(&config_new, &config_current)) {
		if (!wifi_softap_set_config_current(&config_new)) {
			printf("set_config failed!\n");
		}
	} else {
		printf("softap config unchanged\n");
	}


    if (wifi_softap_dhcps_status() != DHCP_STARTED) {
		printf("DHCP not started, starting...\n");
		if (!wifi_softap_dhcps_start()) {
			printf("wifi_softap_dhcps_start failed!\n");
		}
	}

	if (wifi_get_ip_info(SOFTAP_IF, &ip)) {
		if (ip.ip.addr == 0x00000000) {
			printf("IP config Invalid resetting...\n");
			IP4_ADDR(&ip.ip, 192, 168, 4, 1);
			IP4_ADDR(&ip.gw, 192, 168, 4, 1);
			IP4_ADDR(&ip.netmask, 255, 255, 255, 0);

			if (!wifi_softap_dhcps_stop()) {
				printf("wifi_softap_dhcps_stop failed!\n");
			}

			if (!wifi_set_ip_info(SOFTAP_IF, &ip)) {
				printf("wifi_set_ip_info failed!\n");
			}

			if (!wifi_softap_dhcps_start()) {
				printf("wifi_softap_dhcps_start failed!\n");
			}

		}
	} else {
		printf("wifi_get_ip_info failed!\n");
	}

	 wifi_set_event_handler_cb(wifi_event_cb);
}

/**
 *
 * @param ssid
 * @param password
 */
void setup_wifi_st_mode(const char *ssid, const char *password, struct ip_info *ip_config) {

	struct station_config stconfig;

	wifi_set_opmode_current(STATION_MODE);
	wifi_station_disconnect();
	wifi_station_dhcpc_stop();
	wifi_softap_dhcps_stop();

	if (wifi_station_get_config(&stconfig)) {
		memset(stconfig.ssid, 0, sizeof(stconfig.ssid));
		memset(stconfig.password, 0, sizeof(stconfig.password));

		sprintf(stconfig.ssid, "%s", ssid);
		sprintf(stconfig.password, "%s", password);

		if (!wifi_station_set_config(&stconfig)) {
			printf("ESP8266 not set station config!\n");
		}
	}

	if (ip_config->ip.addr != 0) {
		wifi_set_ip_info(STATION_IF, ip_config);
	} else {
		wifi_station_dhcpc_start();
	}

	wifi_station_connect();
	wifi_station_set_auto_connect(1);

	printf("ESP8266 in STA mode configured.\n");
}

/**
 *
 */
void reply_with_wifi_mode(void) {
	printf("reply_with_wifi_mode :");
	const uint8_t mode = wifi_get_opmode();
	printf("%d\n", mode);
	rpi_write_bytes((uint8_t *) &mode, 1);
}

/**
 *
 */
void reply_with_mac_address(void) {
	printf("reply_with_mac_address\n");
	rpi_write_bytes(g_hwmac, 6);
}

/**
 *
 */
void reply_with_ip_config(void) {
	printf("reply_with_ip_config\n");
	rpi_write_bytes((uint8_t *) &g_ipconfig, 12);
}

/**
 *
 */
void reply_with_hostname(void) {
  printf("reply_with_hostname : %s[%d]\n	", g_host_name, g_host_name_length);
  rpi_write_bytes((uint8_t *)g_host_name, g_host_name_length);
  rpi_write_bytes((uint8_t *)&gc_zero, 1);
}

/**
 *
 */
void reply_with_sdk_version(void) {
  printf("reply_with_sdk_version : %s[%d]\n", g_sdk_version, g_sdk_version_length);
  rpi_write_bytes((uint8_t *)g_sdk_version, g_sdk_version_length);
  rpi_write_bytes((uint8_t *)&gc_zero, 1);
}

/**
 *
 */
void reply_with_cpu_freq(void) {
  printf("reply_with_cpufreq\n");
  g_cpu_freq = system_get_cpu_freq();
  rpi_write_bytes((uint8_t *)&g_cpu_freq, 1);
}

/**
 *
 */
void handle_udp_begin(void) {
	printf("handle_udp_begin - ");

	g_port_udp_begin = rpi_read_halfword();

	printf("xTaskCreate (%d)\n"), g_port_udp_begin;

	if (g_task_udp_handle != NULL) {
		close(g_sock_fd);
		vTaskDelete(g_task_udp_handle);
		g_task_udp_handle = NULL;
	}

	xTaskCreate(task_udp, "udp-interface", 256, NULL, 2, &g_task_udp_handle);
}

void wifi_mode_sta_connect(const char *ssid, const char *password, struct ip_info *ip_config) {
	struct station_config sta_config;

	_conn_state conn_state = WIFI_NOT_CONNECTED;

	if (wifi_station_get_config(&sta_config)) {
		if (strncmp(sta_config.ssid, ssid, 32) == 0) {
			if (wifi_station_get_connect_status() == STATION_GOT_IP) {
				conn_state = WIFI_CONNECTED;
				printf("WiFi already connected\n");
			}
		}
	}

	if (conn_state == WIFI_NOT_CONNECTED) {
		setup_wifi_st_mode(ssid, password, ip_config);
		conn_state = WIFI_CONNECTING;
	}

	while (conn_state == WIFI_CONNECTING) {
		switch (wifi_station_get_connect_status()) {
		case STATION_GOT_IP:
			wifi_get_ip_info(STATION_IF, ip_config);
			if (ip_config->ip.addr != 0) {
				conn_state = WIFI_CONNECTED;
				printf("WiFi connected\n");
				continue;
			}
			break;
		case STATION_WRONG_PASSWORD:
			conn_state = WIFI_CONNECTING_ERROR;
			printf("WiFi connecting error, wrong password\n");
			break;
		case STATION_NO_AP_FOUND:
			conn_state = WIFI_CONNECTING_ERROR;
			printf("WiFi connecting error, ap not found\n");
			break;
		case STATION_CONNECT_FAIL:
			conn_state = WIFI_CONNECTING_ERROR;
			printf("WiFi connecting fail\n");
			break;
		default:
			conn_state = WIFI_CONNECTING;
			printf("WiFi connecting...\n");
		}

		vTaskDelay(1000 / portTICK_RATE_MS);
	}

	wifi_get_macaddr(STATION_IF, (uint8_t *) g_hwmac);
	wifi_get_ip_info(STATION_IF, &g_ipconfig);
}

void rpi_read_ssid_password(char *ssid, char *password) {
	uint8_t d;
	unsigned i = 0;

	data_gpio_fsel_input();

	while (((d = _read_byte()) != 0) && (i < 31)) {
		ssid[i++] = d;
	}
	ssid[i] = '\0';

	i = 0;

	while (((d = _read_byte()) != 0) && (i < 31)) {
		password[i++] = d;
	}
	password[i] = '\0';

	printf("ssid : %s\n", ssid);
	printf("pwd : %s\n", password);
}

void handle_wifi_mode_sta_static_ip(void) {
	printf("handle_wifi_mode_sta_static_ip\n");
	char ssid[32];
	char password[32];
	struct ip_info ip_config;

	rpi_read_ssid_password(ssid, password);
	rpi_read_bytes((uint8_t *) &ip_config, 12);
	wifi_mode_sta_connect(ssid, password, &ip_config);
}

/**
 *
 */
void handle_wifi_mode_sta_dhcp(void) {
	printf("handle_wifi_mode_sta_dhcp\n");

	char ssid[32];
	char password[32];
	struct ip_info ip_config;

	// DHCP
	ip_config.ip.addr = 0;
	ip_config.gw.addr = 0;
	ip_config.netmask.addr = 0;

	rpi_read_ssid_password(ssid, password);
	wifi_mode_sta_connect(ssid, password, &ip_config);
}

/**
 *
 */
void IRAM_ATTR reply_with_udp_packet(void) {
	if (g_head == g_tail) {
		uint16_t data = 0;
		rpi_write_bytes((uint8_t *) &data, 2);
	} else {
		uint16_t len = g_udp_receive[g_tail].len;
		rpi_write_bytes((uint8_t *) &len, 2);
		uint32_t ip = g_udp_receive[g_tail].ip_address;
		rpi_write_bytes((uint8_t *) &ip, 4);
		uint16_t port = g_udp_receive[g_tail].port;
		rpi_write_bytes((uint8_t *) &port, 2);
		rpi_write_bytes((uint8_t *) g_udp_receive[g_tail].data, len);
		g_tail = (g_tail + 1) & DATA_BUFFER_INDEX_MASK;
	}
}

/**
 *
 */
void IRAM_ATTR handle_udp_packet(void) {
	//printf("handle_udp_packet\n");	//TODO DEBUG

	struct sockaddr_in address_remote;
	int slen = sizeof(address_remote);

	const uint16_t len = rpi_read_halfword();
	const uint32_t ip_address = rpi_read_word();
	const uint16_t port = rpi_read_halfword();
	rpi_read_bytes((uint8_t *) g_sendto_buffer, len);

	address_remote.sin_family = AF_INET;
	address_remote.sin_addr.s_addr = ip_address;
	address_remote.sin_port = htons(port);

	if ((sendto(g_sock_fd, g_sendto_buffer, len, 0, (const struct sockaddr * )&address_remote, slen)) == -1) {
		printf("ERROR sendto\n");
	}
}


void ICACHE_FLASH_ATTR ota_finished_callback(void *arg) {
	struct upgrade_server_info *update = arg;

	if (update->upgrade_flag == true) {
		printf("OTA : Success, rebooting\n");
		system_upgrade_reboot();
	} else {
		printf("OTA : Failed\n");
	}

	free(update->url);
	free(update);
}

/**
 *
 */
void ICACHE_FLASH_ATTR handle_ota_start(void) {
	printf("handle_ota_start\n");

	struct sockaddr_in remote_ip;

	const uint32 server_ip = rpi_read_word();

	if (g_task_udp_handle != NULL) {
		close(g_sock_fd);
		vTaskDelete(g_task_udp_handle);
		g_task_udp_handle = NULL;
	}

	const uint8_t user_bin = system_upgrade_userbin_check();

	switch (user_bin) {
	case UPGRADE_FW_BIN1:
		g_update_file = (char *)g_user2_bin;
		break;
	case UPGRADE_FW_BIN2:
		g_update_file = (char *)g_user1_bin;
		break;
	default:
		printf("Invalid userbin number : %d\n", user_bin);
		for(;;);
	}

	bzero(&remote_ip, sizeof(struct sockaddr_in));
	remote_ip.sin_family = AF_INET;
	remote_ip.sin_addr.s_addr = server_ip;
	remote_ip.sin_port = htons(OTA_UPDATE_PORT);

	struct upgrade_server_info* update = (struct upgrade_server_info *) zalloc(sizeof(struct upgrade_server_info));
	update->url = (uint8 *) zalloc(128);		/* the url of upgrading server */
	update->check_cb = ota_finished_callback;	/* callback of upgrading */
	update->check_times = 10000;				/* time out of upgrading, unit : ms */
	update->sockaddrin = remote_ip;				/* socket of upgrading */

	sprintf((char *) update->url, "GET /%s HTTP/1.1\r\nHost: "IPSTR":%d\r\nConnection: close\r\n\r\n\r", g_update_file, IP2STR(server_ip), OTA_UPDATE_PORT);

	printf("url : %s\n", (char *) update->url);

/*
	if (system_upgrade_start(update) == false) {
		printf("OTA : Could not start upgrade\n");
		free(update->url);
		free(update);
	} else {
		printf("OTA : Upgrading...\n");
	}
*/
}

/**
 *
 * @param pvParameters
 */
void IRAM_ATTR task_rpi(void *pvParameters) {
	printf("rpi-interface\n");

	setup_wifi_ap_mode();

	g_host_name = (char *) wifi_station_get_hostname();
	g_host_name_length = strlen(g_host_name);

	wifi_get_macaddr(SOFTAP_IF, (uint8_t *) g_hwmac);
	wifi_get_ip_info(SOFTAP_IF, &g_ipconfig);

	if (GPI & (uint32_t) (1 << ESP8266_RPI_CTRL_IN)) {
		printf("Ctrl pin is high\n");
	}

	while (GPI & (uint32_t) (1 << ESP8266_RPI_CTRL_IN)) {
	}

	if (!(GPI & (uint32_t) (1 << ESP8266_RPI_CTRL_IN))) {
		printf("Ctrl pin is low\n");
	}

	_commands state = WAIT_FOR_CMD;

	while (1) {
		switch (state) {
		case WAIT_FOR_CMD:
			state = rpi_read_4bits();
			break;
		case CMD_SDK_VERSION:
			reply_with_sdk_version();
			state = WAIT_FOR_CMD;
			break;
		case CMD_CPU_FREQ:
			reply_with_cpu_freq();
			state = WAIT_FOR_CMD;
			break;
		case CMD_HOST_NAME:
			reply_with_hostname();
			state = WAIT_FOR_CMD;
			break;
		case CMD_WIFI_MODE:
			reply_with_wifi_mode();
			state = WAIT_FOR_CMD;
			break;
		case CMD_MAC_ADDRESS:
			reply_with_mac_address();
			state = WAIT_FOR_CMD;
			break;
		case CMD_IP_CONFIG:
			reply_with_ip_config();
			state = WAIT_FOR_CMD;
			break;
		case CMD_UDP_BEGIN:
			handle_udp_begin();
			state = WAIT_FOR_CMD;
			break;
		case CMD_WIFI_MODE_STA:
			handle_wifi_mode_sta_dhcp();
			state = WAIT_FOR_CMD;
			break;
		case CMD_WIFI_MODE_STA_IP:
			handle_wifi_mode_sta_static_ip();
			state = WAIT_FOR_CMD;
			break;
		case CMD_UDP_RECEIVE:
			reply_with_udp_packet();
			state = WAIT_FOR_CMD;
			break;
		case CMD_UPD_SEND:
			handle_udp_packet();
			state = WAIT_FOR_CMD;
			break;
		case CMD_OTA_START:
			handle_ota_start();
			state = WAIT_FOR_CMD;
			break;
		default:
			state = WAIT_FOR_CMD;
			break;
		}
	}

	vTaskDelete(NULL);
}

/**
 *
 */
void user_init(void) {
	printf("Compiled on %s at %s\n", __DATE__, __TIME__);

	system_update_cpu_freq(SYS_CPU_160MHZ);

	g_sdk_version = (char *) system_get_sdk_version();
	g_sdk_version_length = strlen(g_sdk_version);

	printf("CPU frequency : %d MHz\n", system_get_cpu_freq());
	printf("SDK version : %s\n", g_sdk_version);
	system_print_meminfo();

	GPF(ESP8266_RPI_CTRL_IN) = GPFFS(GPFFS_GPIO(ESP8266_RPI_CTRL_IN));
	GPC(ESP8266_RPI_CTRL_IN) = (GPC(ESP8266_RPI_CTRL_IN) & (0xF << GPCI));

	GPF(ESP8266_RPI_CTRL_OUT) = GPFFS(GPFFS_GPIO(ESP8266_RPI_CTRL_OUT));
	GPC(ESP8266_RPI_CTRL_OUT) = (GPC(ESP8266_RPI_CTRL_OUT) & (0xF << GPCI));

	GPF(12) = GPFFS(GPFFS_GPIO(12));
	GPC(12) = (GPC(12) & (0xF << GPCI));
	GPF(13) = GPFFS(GPFFS_GPIO(13));
	GPC(13) = (GPC(13) & (0xF << GPCI));
	GPF(14) = GPFFS(GPFFS_GPIO(14));
	GPC(14) = (GPC(14) & (0xF << GPCI));
	GPF(15) = GPFFS(GPFFS_GPIO(15));
	GPC(15) = (GPC(15) & (0xF << GPCI));

	GPEC = (uint32_t) (1 << ESP8266_RPI_CTRL_IN);
	GPES = (uint32_t) (1 << ESP8266_RPI_CTRL_OUT);
	GPOC = (uint32_t) (1 << ESP8266_RPI_CTRL_OUT);

	xTaskCreate(task_rpi, "rpi-interface", 256, NULL, 2, &g_task_rpi_handle);
}

