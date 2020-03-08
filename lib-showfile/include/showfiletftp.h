/**
 * @file showfiletftp.h
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef SHOWFILETFTP_H_
#define SHOWFILETFTP_H_

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "tftpdaemon.h"

class ShowFileTFTP: public TFTPDaemon {
public:
	ShowFileTFTP(void);
	~ShowFileTFTP(void);

	bool FileOpen(const char *pFileName, TTFTPMode tMode);
	bool FileCreate(const char *pFileName, TTFTPMode tMode);
	bool FileClose(void);
	int FileRead(void *pBuffer, unsigned nCount, unsigned nBlockNumber);
	int FileWrite(const void *pBuffer, unsigned nCount, unsigned nBlockNumber);

	void Exit(void);

private:
	FILE *m_pFile;
};

#endif /* SHOWFILETFTP_H_ */
