/**
 * @file wifi.h
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

#ifndef WIFI_H_
#define WIFI_H_

#include <stdint.h>
#include <stdbool.h>

#include "ip_address.h"

typedef enum wifi_mode {
	WIFI_OFF = 0,
	WIFI_STA = 1,
	WIFI_AP = 2,
	WIFI_AP_STA = 3
} _wifi_mode;

typedef enum wifi_station_status {
	WIFI_STATION_IDLE = 0,        /**< ESP8266 station idle */
	WIFI_STATION_CONNECTING,      /**< ESP8266 station is connecting to AP*/
	WIFI_STATION_WRONG_PASSWORD,  /**< the password is wrong*/
	WIFI_STATION_NO_AP_FOUND,     /**< ESP8266 station can not find the target AP*/
	WIFI_STATION_CONNECT_FAIL,    /**< ESP8266 station fail to connect to AP*/
	WIFI_STATION_GOT_IP           /**< ESP8266 station got IP address from AP*/
} _wifi_station_status;

typedef enum wifiphy_phy_mode {
	WIFI_PHY_MODE_11B = 1,
	WIFI_PHY_MODE_11G = 2,
	WIFI_PHY_MODE_11N = 3
} _wifiphy_phy_mode;

#define STATION_IF	0x00
#define SOFTAP_IF	0x01

#define HOST_NAME_MAX			255
#define SDK_VERSION_MAX			255
#define FIRMWARE_VERSION_MAX	255

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ESP8266 SDK functions 
 */

extern /*@shared@*/const char *system_get_sdk_version(void);

/*
 * Wifi functions
 */

extern const bool wifi(const struct ip_info *);

extern const bool wifi_get_macaddr(/*@out@*/const uint8_t *);
extern const bool wifi_get_ip_info(/*@out@*/const struct ip_info *);
extern _wifi_mode wifi_get_opmode(void);
extern /*@shared@*/const char *wifi_get_hostname(void);
extern const bool wifi_detect(void);
extern /*@shared@*/const char *wifi_get_firmware_version(void);

/*
 * Wifi AP functions
 */

extern void wifi_ap_init(const char *);
extern const bool wifi_ap_is_open(void);

/*
 * Wifi Station functions
 */

extern void wifi_station(const char *, const char *);
extern void wifi_station_ip(const char *, const char *, const struct ip_info *);
extern /*@observer@*/const char *wifi_station_status(_wifi_station_status);
extern const bool wifi_station_is_dhcp_used(void);
extern const _wifi_station_status wifi_station_get_connect_status(void);
extern /*@observer@*//*@null@*/const char *wifi_get_ssid(void);

#ifdef __cplusplus
}
#endif

#endif /* WIFI_H_ */
