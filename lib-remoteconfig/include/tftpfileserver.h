/**
 * @file tftpfileserver.h
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <stdint.h>

#include "tftpdaemon.h"

class TFTPFileServer final: public TFTPDaemon {
public:
	TFTPFileServer (uint8_t *pBuffer, uint32_t nSize);
	~TFTPFileServer () override;

	bool FileOpen (const char *pFileName, TFTPMode tMode) override;
	bool FileCreate (const char *pFileName, TFTPMode tMode) override;
	bool FileClose () override;
	size_t FileRead (void *pBuffer, size_t nCount, unsigned nBlockNumber) override;
	size_t FileWrite (const void *pBuffer, size_t nCount, unsigned nBlockNumber) override;
	void Exit() override;

	uint32_t GetFileSize() {
		return m_nFileSize;
	}

	bool isDone()  {
		return m_bDone;
	}

private:
	uint8_t* m_pBuffer;
	uint32_t m_nSize;
	uint32_t m_nFileSize;
	bool m_bIsCompressedSupported;
	bool m_bDone;
};

#endif /* TFTPFILESERVER_H_ */
