/**
 * @file esp8266_cmd.h
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

#ifndef ESP8266_CMD_H_
#define ESP8266_CMD_H_

typedef enum commands {
	CMD_NOP = 0,
	//
	CMD_SYSTEM_FIRMWARE_VERSION = 6,
	CMD_SYSTEM_SDK_VERSION = 8,
	//
	CMD_WIFI_MODE = 9,
	CMD_WIFI_MODE_STA = 10,
	CMD_WIFI_MODE_STA_IP = 13,
	CMD_WIFI_MODE_STA_STATUS = 7,
	CMD_WIFI_MODE_AP = 11,
	//
	CMD_WIFI_MAC_ADDRESS = 1,
	CMD_WIFI_IP_INFO = 2,
	CMD_WIFI_HOST_NAME = 5,
	//
	CMD_WIFI_UDP_BEGIN = 4,
	CMD_WIFI_UDP_RECEIVE = 3,
	CMD_WIFI_UDP_SEND = 12,
	CMD_WIFI_UDP_JOIN_GROUP = 14,
	//
	CMD_ESP_FOTA_START = 15
} _commands;

#endif /* ESP8266_CMD_H_ */
