/**
 * @file spiflashinstall.cpp
 *
 */
/* Copyright (C) 2021 by Arjan van Vught mailto:info@gd32-dmx.nl
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

#if defined(__clang__)
# pragma GCC diagnostic ignored "-Wunused-private-field"
#endif

#include <cassert>

#include "spiflashinstall.h"

#include "display.h"
#include "spi_flash.h"

#include "debug.h"

SpiFlashInstall *SpiFlashInstall::s_pThis = nullptr;

SpiFlashInstall::SpiFlashInstall() {
	DEBUG_ENTRY

	assert(s_pThis == 0);
	s_pThis = this;

	Display::Get()->Cls();

	m_nFlashSize = spi_flash_get_size();

	printf("%s, sector size %d, %d bytes\n", spi_flash_get_name(), spi_flash_get_sector_size(), m_nFlashSize);
	Display::Get()->Write(1, spi_flash_get_name());

	DEBUG_EXIT
}

SpiFlashInstall::~SpiFlashInstall() {
	DEBUG_ENTRY
	DEBUG_EXIT
}

void SpiFlashInstall::Process(__attribute__((unused)) const char *pFileName, __attribute__((unused)) uint32_t nOffset) {
	DEBUG_ENTRY
	assert(0);
	DEBUG_EXIT
}

bool SpiFlashInstall::Open(__attribute__((unused)) const char* pFileName) {
	DEBUG_ENTRY
	assert(0);
	DEBUG_EXIT
	return false;
}

void SpiFlashInstall::Close() {
	DEBUG_ENTRY
	assert(0);
	DEBUG_EXIT
}

bool SpiFlashInstall::BuffersCompare(__attribute__((unused)) uint32_t nSize) {
	DEBUG_ENTRY
	assert(0);
	DEBUG_EXIT
	return false;
}

bool SpiFlashInstall::Diff(__attribute__((unused)) uint32_t nOffset) {
	DEBUG_ENTRY
	assert(0);
	DEBUG_EXIT
	return false;
}

void SpiFlashInstall::Write(__attribute__((unused)) uint32_t nOffset) {
	DEBUG_ENTRY
	assert(0);
	DEBUG_EXIT
}

