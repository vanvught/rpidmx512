/**
 * @file ubootheader.cpp
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

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <cassert>

#include "ubootheader.h"

#define LZ4F_MAGIC			0x184D2204	/* LZ4 Magic Number		*/
#define IH_MAGIC			0x27051956	/* Image Magic Number	*/
#define IH_NMLEN			32			/* Image Name Length	*/
#define IH_LOAD				0x40000000
#define IH_EP				0x40000000
#define IH_OS_U_BOOT		17
#define IH_ARCH_ARM			2
#define IH_TYPE_STANDALONE	1

struct TImageHeader {
	uint32_t ih_magic;			/* Image Header Magic Number	*/
	uint32_t ih_hcrc;			/* Image Header CRC Checksum	*/
	uint32_t ih_time;			/* Image Creation Timestamp	*/
	uint32_t ih_size;			/* Image Data Size		*/
	uint32_t ih_load;			/* Data	 Load  Address		*/
	uint32_t ih_ep;				/* Entry Point Address		*/
	uint32_t ih_dcrc;			/* Image Data CRC Checksum	*/
	uint8_t ih_os;				/* Operating System		*/
	uint8_t ih_arch;			/* CPU architecture		*/
	uint8_t ih_type;			/* Image Type			*/
	uint8_t ih_comp; 			/* Compression Type		*/
	uint8_t ih_name[IH_NMLEN];	/* Image Name		*/
};

enum TImageHeaderCompression {
	IH_COMP_NONE = 0, 	/*  No	 Compression Used	*/
	IH_COMP_GZIP		/* gzip	 Compression Used	*/
};

UBootHeader::UBootHeader(uint8_t *pHeader): m_pHeader(pHeader), m_bIsValid(false) {
	assert(pHeader != nullptr);

	TImageHeader *pImageHeader = reinterpret_cast<TImageHeader*>(pHeader);

	m_bIsValid =  (pImageHeader->ih_magic == __builtin_bswap32(IH_MAGIC));
	m_bIsValid &= (pImageHeader->ih_load == __builtin_bswap32(IH_LOAD));
	m_bIsValid &= (pImageHeader->ih_ep == __builtin_bswap32(IH_EP));
	m_bIsValid &= (pImageHeader->ih_os == IH_OS_U_BOOT);
	m_bIsValid &= (pImageHeader->ih_arch == IH_ARCH_ARM);
	m_bIsValid &= (pImageHeader->ih_type == IH_TYPE_STANDALONE);
	m_bIsValid &= (strncmp(reinterpret_cast<const char*>(pImageHeader->ih_name), "http://www.orangepi-dmx.org", IH_NMLEN) == 0);

	m_bIsCompressed = (pImageHeader->ih_comp == IH_COMP_GZIP);
}

UBootHeader::~UBootHeader() {
	m_bIsValid = false;
}

void UBootHeader::Dump() {
#ifndef NDEBUG
	if (!m_bIsValid) {
		printf("* Not a valid header! *\n");
	}

	TImageHeader *pImageHeader = reinterpret_cast<TImageHeader*>(m_pHeader);
	const time_t rawtime = __builtin_bswap32(pImageHeader->ih_time);
	struct tm *info = localtime(&rawtime);

	printf("Magic Number        : %.8x\n", __builtin_bswap32(pImageHeader->ih_magic));
	printf("CRC Checksum        : %.8x\n", __builtin_bswap32(pImageHeader->ih_hcrc));
	printf("Creation Timestamp  : %.8x - %s", __builtin_bswap32(pImageHeader->ih_time), asctime(info));
	printf("Data Size           : %.8x - %d kBytes\n", __builtin_bswap32(pImageHeader->ih_size), __builtin_bswap32(pImageHeader->ih_size) / 1024);
	printf("Data Load Address   : %.8x\n", __builtin_bswap32(pImageHeader->ih_load));
	printf("Entry Point Address : %.8x\n", __builtin_bswap32(pImageHeader->ih_ep));
	printf("Operating System    : %d - %s\n", pImageHeader->ih_os, pImageHeader->ih_os == IH_OS_U_BOOT ? "Firmware" : "Not supported");
	printf("CPU architecture    : %d - %s\n", pImageHeader->ih_arch, pImageHeader->ih_arch == IH_ARCH_ARM ? "Arm" : "Not supported");
	printf("Image type          : %d - %s\n", pImageHeader->ih_type, pImageHeader->ih_type == IH_TYPE_STANDALONE ? "Standalone Program" : "Not supported");
	printf("Compression         : %d - %s\n", pImageHeader->ih_comp, pImageHeader->ih_comp == IH_COMP_NONE ? "none" : (pImageHeader->ih_comp == IH_COMP_GZIP ? "gzip" : "Not supported"));
	printf("Image Name          : %s\n", pImageHeader->ih_name);
#endif
}
