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

	bool WriteFirmware(const uint8_t *buffer, uint32_t size);

	static FlashCodeInstall* Get() {
		return s_this;
	}

private:
	bool Open(const char *file_name);
	void Close();
	bool BuffersCompare(uint32_t size);
	bool Diff(uint32_t offset);
	void Write(uint32_t offset);
	void Process(const char *file_name, uint32_t offset);

private:
 uint32_t erase_size_{0};
 uint32_t flash_size_{0};
 uint8_t* file_buffer_{nullptr};
 uint8_t* flash_buffer_{nullptr};
 FILE* file_{nullptr};

 bool have_flash_{false};

 inline static FlashCodeInstall* s_this;
};

#endif  // FLASHCODEINSTALL_H_
