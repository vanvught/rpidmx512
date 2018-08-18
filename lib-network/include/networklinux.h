/**
 * @file networklinux.h
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef NETWORKLINUX_H_
#define NETWORKLINUX_H_

#include <stdint.h>
#include <net/if.h>
#include <limits.h>

#include "network.h"

#ifndef HOST_NAME_MAX
 #define HOST_NAME_MAX 255
#endif

class NetworkLinux: public Network {
public:
	NetworkLinux(void);
	~NetworkLinux(void);

	int Init(const char *s);

	void Begin(uint16_t nPort);
	void End(void);

	void MacAddressCopyTo(uint8_t *pMacAddress);
	const char* GetHostName(void);

	void SetIp(uint32_t ip);

	void JoinGroup(uint32_t ip);
	uint16_t RecvFrom(uint8_t *packet, uint16_t size, uint32_t *from_ip, uint16_t *from_port);
	void SendTo(const uint8_t *packet, uint16_t size, uint32_t to_ip, uint16_t remote_port);

private:
	bool is_dhclient(const char *if_name);
	int if_get_by_address(const char *ip, char *name, size_t len);
	int if_details(const char *iface);

private:
	int _socket;
	char _if_name[IFNAMSIZ];
	char _hostname[HOST_NAME_MAX + 1];
};

#endif /* NETWORKLINUX_H_ */
