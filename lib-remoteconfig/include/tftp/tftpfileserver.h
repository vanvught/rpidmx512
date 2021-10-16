/**
 * @file tftpfileserver.h
 *
 */
/* Copyright (C) 2019-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef TFTPFILESERVER_H_
#define TFTPFILESERVER_H_

#include <cstddef>
#include <cstdint>

#include "tftpdaemon.h"

namespace tftpfileserver {
	bool is_valid(const void *pBuffer);
#if defined (BARE_METAL)
# if defined (H3)
#  if defined(ORANGE_PI)
	static constexpr char FILE_NAME[] = "orangepi_zero.uImage";
#  else
	static constexpr char FILE_NAME[] = "orangepi_one.uImage";
#  endif
# elif defined (GD32)
	static constexpr char FILE_NAME[] = "gd32.bin";
# endif
#endif
}  // namespace tftpfileserver

class TFTPFileServer final: public TFTPDaemon {
public:
	TFTPFileServer (uint8_t *pBuffer, uint32_t nSize);
	~TFTPFileServer () override {}

	bool FileOpen (const char *pFileName, TFTPMode tMode) override;
	bool FileCreate (const char *pFileName, TFTPMode tMode) override;
	bool FileClose () override;
	size_t FileRead (void *pBuffer, size_t nCount, unsigned nBlockNumber) override;
	size_t FileWrite (const void *pBuffer, size_t nCount, unsigned nBlockNumber) override;
	void Exit() override;

	uint32_t GetFileSize() const {
		return m_nFileSize;
	}

	bool isDone() const {
		return m_bDone;
	}

private:
	uint8_t *m_pBuffer;
	uint32_t m_nSize;
	uint32_t m_nFileSize { 0 };
	bool m_bDone { false };
};

#endif /* TFTPFILESERVER_H_ */
