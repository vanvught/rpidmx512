/**
 * @file networkw5x00.h
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

#ifndef NETWORKW5X00_H_
#define NETWORKW5X00_H_

#include <stdint.h>

#include "network.h"

#ifndef HOST_NAME_MAX
 #define HOST_NAME_MAX 255
#endif

enum {
	NETWORK5X00_SPI_CS_DEFAULT = 8,
	NETWORK5X00_SPI_SPEED_HZ_DEFAULT = 4000000
};

class NetworkW5x00: public Network {
public:
	NetworkW5x00(void);
	~NetworkW5x00(void);

	int Init(void);

	void Begin(uint16_t nPort);
	void End(void);

	void MacAddressCopyTo(uint8_t *pMacAddress);
	void SetMacAddress(const char *pMacAddress);

	const char* GetHostName(void);

	uint16_t RecvFrom(uint8_t *packet, uint16_t size, uint32_t *from_ip, uint16_t *from_port);
	void SendTo(const uint8_t *packet, uint16_t size, uint32_t to_ip, uint16_t remote_port);

	void SetIp(uint32_t ip);
	void JoinGroup(uint32_t ip);

	// W5X00 device specific

	void SetSpiChipSelect(uint8_t nSpiChipSelect=NETWORK5X00_SPI_CS_DEFAULT);
	inline uint8_t GetSpiChipSelect(void) { return m_nSpiChipSelect; }

	void SetSpiSpeedHz(uint32_t nSpiSpeedHz=NETWORK5X00_SPI_SPEED_HZ_DEFAULT);
	inline uint32_t GetSpiSpeedHz(void) { return m_nSpiSpeedHz; }

	const char *GetChipName(void);

protected:
	uint8_t m_nSpiChipSelect;
	uint32_t m_nSpiSpeedHz;
	char m_aHostname[HOST_NAME_MAX + 1];
	bool m_IsSetMacAddress;
	bool m_IsMulticast;
	uint16_t m_nPort;
};

#endif /* NETWORKW5X00_H_ */
