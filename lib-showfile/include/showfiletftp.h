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

#include <stdio.h>

#include "tftpdaemon.h"

class ShowFileTFTP final: public TFTPDaemon {
public:
	ShowFileTFTP();

	bool FileOpen(const char *pFileName, TFTPMode tMode) override;
	bool FileCreate(const char *pFileName, TFTPMode tMode) override;
	bool FileClose() override;
	size_t FileRead(void *pBuffer, size_t nCount, unsigned nBlockNumber) override;
	size_t FileWrite(const void *pBuffer, size_t nCount, unsigned nBlockNumber) override;

	void Exit() override;

private:
	FILE *m_pFile {nullptr};
};

#endif /* SHOWFILETFTP_H_ */
