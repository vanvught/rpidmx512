/**
 * @file tftpfileserver.h
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

#ifndef TFTPFILESERVER_H_
#define TFTPFILESERVER_H_

#include <stdint.h>

#include "tftpdaemon.h"

class TFTPFileServer: public TFTPDaemon {
public:
	TFTPFileServer (uint8_t *pBuffer, uint32_t nSize);
	~TFTPFileServer (void);

	bool FileOpen (const char *pFileName, TTFTPMode tMode);
	bool FileCreate (const char *pFileName, TTFTPMode tMode);
	bool FileClose (void);
	int FileRead (void *pBuffer, unsigned nCount, unsigned nBlockNumber);
	int FileWrite (const void *pBuffer, unsigned nCount, unsigned nBlockNumber);

	uint32_t GetFileSize(void) {
		return m_nFileSize;
	}

	bool isDone(void)  {
		return m_bDone;
	}

private:
	uint8_t* m_pBuffer;
	uint32_t m_nSize;
	uint32_t m_nFileSize;
	bool m_bDone;
};

#endif /* TFTPFILESERVER_H_ */
