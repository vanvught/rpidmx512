/**
 * @file networkparams.h
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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


#ifndef NETWORKPARAMS_H_
#define NETWORKPARAMS_H_

#include <stdbool.h>
#include <stdint.h>

class NetworkParams {
public:
	NetworkParams(void);
	~NetworkParams(void);

	bool Load(void);

	bool isDhcpUsed(void) const;
	uint32_t GetIpAddress(void) const;
	uint32_t GetNetMask(void) const;
	uint32_t GetDefaultGateway(void) const;
	uint32_t GetNameServer(void) const;

	void Dump(void);

private:
	bool isMaskSet(uint16_t) const;

public:
    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *s);

private:
    uint32_t m_bSetList;

    bool		m_isDhcpUsed;
    uint32_t	m_IpAddress;
    uint32_t	m_NetMask;
    uint32_t	m_DefaultGateway;
    uint32_t	m_NameServer;

private:
};

#endif /* NETWORKPARAMS_H_ */
