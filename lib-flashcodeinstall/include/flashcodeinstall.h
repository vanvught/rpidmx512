/**
 * @file flashcodeinstall.h
 *
 */
/* Copyright (C) 2018-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef FLASHCODEINSTALL_H_
#define FLASHCODEINSTALL_H_

#include <cstdint>
#include <cstdio>

#include "flashcode.h"

#include "firmware.h" //TODO Remove

class FlashCodeInstall: FlashCode {
public:
	FlashCodeInstall();
	~FlashCodeInstall();

	bool WriteFirmware(const uint8_t *pBuffer, uint32_t nSize);

	static FlashCodeInstall* Get() {
		return s_pThis;
	}

private:
	bool Open(const char *pFileName);
	void Close();
	bool BuffersCompare(uint32_t nSize);
	bool Diff(uint32_t nOffset);
	void Write(uint32_t nOffset);
	void Process(const char *pFileName, uint32_t nOffset);

private:
	uint32_t m_nEraseSize { 0 };
	uint32_t m_nFlashSize { 0 };
	uint8_t *m_pFileBuffer { nullptr };
	uint8_t *m_pFlashBuffer { nullptr };
	FILE *m_pFile { nullptr };

	bool m_bHaveFlashChip { false };

	static FlashCodeInstall *s_pThis;
};

#endif /* FLASHCODEINSTALL_H_ */
