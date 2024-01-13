/**
 * @file oscsimplesend.h
 *
 */
/* Copyright (C) 2020-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef OSCSIMPLESEND_H_
#define OSCSIMPLESEND_H_

#include <cstdint>

namespace osc {
namespace simple {
namespace send {
static constexpr auto BUFFER_SIZE = 512U;
}  // namespace send
}  // namespace simple
}  // namespace osc

class OscSimpleSend {
public:
	// Support for path only
	OscSimpleSend(int32_t nHandle, uint32_t nIpAddress , uint16_t nPort, const char *pPath, const char *pType);
	// Support for 's'
	OscSimpleSend(int32_t nHandle, uint32_t nIpAddress , uint16_t nPort, const char *pPath, const char *pType, const char *pString);
	// Support for type 'i'
	OscSimpleSend(int32_t nHandle, uint32_t nIpAddress , uint16_t nPort, const char *pPath, const char *pType, int nValue);
	// Support for type 'f'
	OscSimpleSend(int32_t nHandle, uint32_t nIpAddress , uint16_t nPort, const char *pPath, const char *pType, float fValue);

private:
	void UpdateMessage(const char *pPath, uint32_t nPathLength, char cType);
	void Send(uint32_t nMessageLength, int32_t nHandle, uint32_t nIpAddress, uint16_t nPort);

private:
	static char s_Message[osc::simple::send::BUFFER_SIZE];
};

#endif /* OSCSIMPLESEND_H_ */
