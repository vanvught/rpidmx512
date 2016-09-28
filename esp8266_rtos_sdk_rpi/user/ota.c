/**
 * @file ota.c
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

#include "debug.h"

#define OTA_UPDATE_PORT		8888

LOCAL char *g_update_file = NULL;
LOCAL const char *g_user1_bin = "user1.bin";
LOCAL const char *g_user2_bin = "user2.bin";

extern const uint32_t ota_rpi_read_word(void);
extern void ota_rpi_write_bytes(const uint8_t *, uint16_t);
extern uint8_t ota_rpi_read_4bits(void);

static void ICACHE_FLASH_ATTR ota_status(const char *s) {
	const uint8_t zero = (uint8_t) 0;
	uint16_t len = strlen(s);

	(void)ota_rpi_read_4bits();

	ota_rpi_write_bytes(s, len);
	ota_rpi_write_bytes((uint8_t *)&zero, 1);
}

/**
 *
 */
void ICACHE_FLASH_ATTR ota_finished_callback(void *arg) {
	struct upgrade_server_info *update = arg;

	if (update->upgrade_flag == true) {
		printf("Success, rebooting\n");
		ota_status("Success, rebooting");
		ota_status("\0");
		system_upgrade_reboot();
	} else {
		printf("Failed\n");
		ota_status("Failed");
	}

	free(update->url);
	free(update);

	ota_status("\0");
}

/**
 *
 */
void ICACHE_FLASH_ATTR handle_ota_start(void) {
	printf("handle_ota_start\n");

	struct sockaddr_in remote_ip;
	const uint32 server_ip = ota_rpi_read_word();
	const uint8_t user_bin = system_upgrade_userbin_check();

	ota_status("status :");

	switch (user_bin) {
	case UPGRADE_FW_BIN1:
		g_update_file = (char *)g_user2_bin;
		break;
	case UPGRADE_FW_BIN2:
		g_update_file = (char *)g_user1_bin;
		break;
	default:
		printf("Invalid userbin number : %d\n", user_bin);
		ota_status("Invalid userbin number");
		for(;;);
	}

	ota_status(g_update_file);

	bzero(&remote_ip, sizeof(struct sockaddr_in));
	remote_ip.sin_family = AF_INET;
	remote_ip.sin_addr.s_addr = server_ip;
	remote_ip.sin_port = htons(OTA_UPDATE_PORT);

	struct upgrade_server_info* update = (struct upgrade_server_info *) zalloc(sizeof(struct upgrade_server_info));
	update->url = (uint8 *) zalloc(128);		/* the url of upgrading server */
	update->check_cb = ota_finished_callback;	/* callback of upgrading */
	update->check_times = 10000;				/* time out of upgrading, unit : ms */
	update->sockaddrin = remote_ip;				/* socket of upgrading */

	sprintf((char *) update->url, "GET /%s HTTP/1.1\r\nHost: %d.%d.%d.%d:%d\r\nConnection: close\r\n\r\n\r",
			g_update_file, (server_ip & 0xff), (server_ip & 0xff00) >> 8, (server_ip & 0xff0000) >> 16, (server_ip >> 24) & 0xff, OTA_UPDATE_PORT);

	printf("url : %s\n", (char *) update->url);
	ota_status((char *) update->url);

	if (system_upgrade_start(update) == false) {
		printf("Could not start upgrade\n");
		ota_status("Could not start upgrade");
		free(update->url);
		free(update);
	} else {
		printf("Upgrading...\n");
		ota_status("Upgrading...");
		for (;;) {
			vTaskDelay(1000000 / portTICK_RATE_MS);
		}
	}

}
