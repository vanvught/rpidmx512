/**
 * @file nextion.h
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef NEXTION_H_
#define NEXTION_H_

#include <sc16is740.h>
#include <stdint.h>
#include <stdbool.h>


class Nextion: public SC16IS740 {
public:
	Nextion(void);
	~Nextion(void);

	void SetBaud(uint32_t nBaud);
	uint32_t GetBaud(void) {
		return m_nBaud;
	}

	bool Start(void);
	void Run(void);

	void Print(void);

private:
	//
	void InitInterrupt(void);
	bool IsInterrupt(void);
	//
	void SendCommand(const char *pCommand);
	bool ReceiveCommandResponse(void);

	void SetText(const char *pObjectName, const char *pValue);
	bool GetText(const char *pObjectName, char *pValue, uint32_t &nLength);
	bool ReceiveReturnedText(char *pValue, uint32_t &nLength);

	void SetValue(const char *pObjectName, uint32_t nValue);
	bool GetValue(const char *pObjectName, uint32_t &nValue);
	bool ReceiveReturnedValue(uint32_t &nValue);

	void SetIp(const char *pObjectName, uint32_t nIp);
	uint32_t GetIp(const char *pObjectName);

	bool Listen(void);

	void HandleTouchEvent(void);
	void HandleReady(void);
	void HandleRawData(void);

	// Main page
	void HandleMain(void);

	// Get
	void HandleGet(uint32_t nIndex);
	void HandleRconfigGet(void);
	void HandleDisplayGet(void);
	void HandleNetworkGet(void);
	void HandleArtNetGet(void);
	void HandleDevicesGet(void);

	// Save
	void HandleSave(uint32_t nIndex);
	void HandleRconfigSave(void);
	void HandleDisplaySave(void);
	void HandleNetworkSave(void);
	void HandleArtNetSave(void);
	void HandleDevicesSave(void);

private:
	uint32_t m_nBaud;
	uint32_t m_nCount;
	uint8_t m_aCommandReturned[64];
};

#endif /* NEXTION_H_ */
