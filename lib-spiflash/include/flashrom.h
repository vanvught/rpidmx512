/**
 * @file flashrom.h
 *
 */
/* Copyright (C) 2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef FLASHROM_H_
#define FLASHROM_H_

#include <cstdint>

namespace flashrom {
enum class result {
	OK,
	ERROR
};
}  // namespace flashrom

class FlashRom {
public:
	FlashRom();

	bool IsDetected() const {
		return m_IsDetected;
	}

	const char* GetName();
	uint32_t GetSize();
	uint32_t GetSectorSize();

	bool Read(uint32_t nOffset, uint32_t nLength, uint8_t *pBuffer, flashrom::result& nResult);
	bool Erase(uint32_t nOffset, uint32_t nLength, flashrom::result& nResult);
	bool Write(uint32_t nOffset, uint32_t nLength, const uint8_t *pBuffer, flashrom::result& nResult);

	static FlashRom *Get() {
		return s_pThis;
	}

private:
	bool m_IsDetected { false };
	static FlashRom *s_pThis;
};

#endif /* INCLUDE_FLASHROM_H_ */
