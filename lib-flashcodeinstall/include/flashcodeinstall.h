/**
 * @file flashcodeinstall.h
 *
 */
/* Copyright (C) 2018-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#if defined (H3)
// nuc-i5:~/uboot-spi/u-boot$ grep CONFIG_BOOTCOMMAND include/configs/sunxi-common.h
// #define CONFIG_BOOTCOMMAND "sf probe; sf read 48000000 180000 22000; bootm 48000000"
# define FIRMWARE_MAX_SIZE	0x22000			// 136K
# define OFFSET_UBOOT_SPI	0x000000
# define OFFSET_UIMAGE		0x180000
#elif defined (GD32)
# if defined (BOARD_GD32F107RC)
#  define OFFSET_UIMAGE		0x007000		// 28K
#  define FIRMWARE_MAX_SIZE (74 * 1024)		// 74K
# elif defined (BOARD_GD32F207RG)
#  define OFFSET_UIMAGE		0x007000		// 28K
#  define FIRMWARE_MAX_SIZE (234 * 1024)	// 234K
# elif defined (BOARD_GD32F207VC)
#  define OFFSET_UIMAGE		0x007000		// 28K
#  define FIRMWARE_MAX_SIZE (106 * 1024)	// 106K
# elif defined (BOARD_GD32F207C_EVAL)
#  define OFFSET_UIMAGE		0x007000		// 28K
#  define FIRMWARE_MAX_SIZE (106 * 1024)	// 106K
# elif defined (BOARD_GD32F407RE)
#  define OFFSET_UIMAGE		0x008000		// 32K
#  define FIRMWARE_MAX_SIZE (106 * 1024)	// 106K
# elif defined (BOARD_BW_OPIDMX4)
#  define OFFSET_UIMAGE		0x008000		// 32K
#  define FIRMWARE_MAX_SIZE (106 * 1024)	// 106K
# elif defined (BOARD_DMX4)
#  define OFFSET_UIMAGE		0x008000		// 32K
#  define FIRMWARE_MAX_SIZE (106 * 1024)	// 106K
# elif defined (BOARD_GD32F450VE)
#  define OFFSET_UIMAGE		0x008000		// 32K
#  define FIRMWARE_MAX_SIZE (168 * 1024)	// 168K
# elif defined (BOARD_GD32F450VI)
#  define OFFSET_UIMAGE		0x008000		// 32K
#  define FIRMWARE_MAX_SIZE (224 * 1024)	// 224K
# endif
#else
# define OFFSET_UIMAGE		0x0
#endif

#ifdef __cplusplus

#include "flashcode.h"

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
#endif

#endif /* FLASHCODEINSTALL_H_ */
