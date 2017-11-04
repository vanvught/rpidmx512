/**
 * @file wifi_network_params.c
 *
 */
/* Copyright (C) 2016-2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdint.h>
#include <stdbool.h>

#include "read_config_file.h"
#include "sscan.h"
#include "util.h"

static const char PARAMS_FILE_NAME[] ALIGNED = "network.txt";			///< Parameters file name
static const char PARAMS_USE_DHCP[] ALIGNED = "use_dhcp";				///<
static const char PARAMS_IP_ADDRESS[] ALIGNED = "ip_address";			///<
static const char PARAMS_NET_MASK[] ALIGNED = "net_mask";				///<
static const char PARAMS_DEFAULT_GATEWAY[] ALIGNED = "default_gateway";	///<
static const char PARAMS_NAME_SERVER[] ALIGNED = "name_server";			///<
static const char PARAMS_SSID[] ALIGNED = "ssid";						///<
static const char PARAMS_PASSWORD[] ALIGNED = "password";				///<

static bool network_params_use_dhcp = true;								///<
static uint32_t network_params_ip_address = (uint32_t) 0;				///<
static uint32_t network_params_net_mask = (uint32_t) 0;					///<
static uint32_t network_params_default_gateway = (uint32_t) 0;			///<
static uint32_t network_params_name_server = (uint32_t) 0;				///<
static char network_params_ssid[34] ALIGNED;							///<
static char network_params_password[34] ALIGNED;						///<

/**
 *
 * @return
 */
const bool network_params_is_use_dhcp(void) {
	return network_params_use_dhcp;
}

/**
 *
 * @return
 */
const uint32_t network_params_get_ip_address(void) {
	return network_params_ip_address;
}

/**
 *
 * @return
 */
const uint32_t network_params_get_net_mask(void) {
	return network_params_net_mask;
}

/**
 *
 * @return
 */
const uint32_t network_params_get_default_gateway(void) {
	return network_params_default_gateway;
}

/**
 *
 * @return
 */
const uint32_t network_params_get_name_server(void) {
	return network_params_name_server;
}

/**
 *
 * @return
 */
const char *network_params_get_ssid(void) {
	return network_params_ssid;
}

/**
 *
 * @return
 */
const char *network_params_get_password(void) {
	return network_params_password;
}


/**
 *
 * @param line
 */
static void process_line_read(const char *line) {
	uint8_t len = 32;
	uint8_t value8;
	uint32_t value32;

	if (sscan_char_p(line, PARAMS_SSID, network_params_ssid, &len) == 2) {
		return;
	}

	len = 32;
	if (sscan_char_p(line, PARAMS_PASSWORD, network_params_password, &len) == 2) {
		return;
	}

	if (sscan_uint8_t(line, PARAMS_USE_DHCP, &value8) == 2) {
		if (value8 == 0) {
			network_params_use_dhcp = false;
		}
		return;
	}

	if (sscan_ip_address(line, PARAMS_IP_ADDRESS, &value32) == 1) {
		network_params_ip_address = value32;
	} else if (sscan_ip_address(line, PARAMS_NET_MASK, &value32) == 1) {
		network_params_net_mask = value32;
	} else if (sscan_ip_address(line, PARAMS_DEFAULT_GATEWAY, &value32) == 1) {
		network_params_default_gateway = value32;
	} else if (sscan_ip_address(line, PARAMS_NAME_SERVER, &value32) == 1) {
		network_params_name_server = value32;
	}
}

/**
 *
 */
const bool network_params_init(void) {
	uint32_t i;

	for (i = 0; i < sizeof(network_params_ssid) / sizeof(char); i++) {
		network_params_ssid[i] = (char) 0;
		network_params_password[i] = (char) 0;
	}

	return read_config_file(PARAMS_FILE_NAME, &process_line_read);
}
