/**
 * @file spiflashinstall.cpp
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

#if __GNUC__ > 8
 #pragma GCC diagnostic ignored "-Wunused-private-field"
#endif

#include "spiflashinstall.h"

#include "debug.h"

SpiFlashInstall *SpiFlashInstall::s_pThis = 0;

SpiFlashInstall::SpiFlashInstall(void):
	m_bHaveFlashChip(false),
	m_nEraseSize(0),
	m_nFlashSize(0),
	m_pFileBuffer(0),
	m_pFlashBuffer(0),
	m_pFile(0)
{
	DEBUG_ENTRY
	s_pThis = this;
	DEBUG_EXIT
}

SpiFlashInstall::~SpiFlashInstall(void) {
	DEBUG_ENTRY
	DEBUG_EXIT
}

void SpiFlashInstall::Process(const char *pFileName, uint32_t nOffset) {
	DEBUG_ENTRY
	DEBUG_EXIT
}

bool SpiFlashInstall::Open(const char* pFileName) {
	DEBUG_ENTRY
	DEBUG_EXIT
	return false;
}

void SpiFlashInstall::Close(void) {
	DEBUG_ENTRY
	DEBUG_EXIT
}

bool SpiFlashInstall::BuffersCompare(uint32_t nSize) {
	DEBUG1_ENTRY
	DEBUG1_EXIT
	return false;
}

bool SpiFlashInstall::Diff(uint32_t nOffset) {
	DEBUG_ENTRY
	DEBUG_EXIT
	return false;
}

void SpiFlashInstall::Write(uint32_t nOffset) {
	DEBUG_ENTRY
	DEBUG_EXIT
}

bool SpiFlashInstall::WriteFirmware(const uint8_t* pBuffer, uint32_t nSize) {
	DEBUG_ENTRY
	DEBUG_EXIT
	return false;
}
