/**
 * @file ubootheader.cpp
 *
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <time.h>
#include <zlib.h>
#include <cassert>

#include "ubootheader.h"
#include "firmware.h"

UBootHeader::UBootHeader(const uint8_t *pHeader): header_(pHeader) {
	assert(pHeader != nullptr);

	auto *pImageHeader = reinterpret_cast<const TImageHeader *>(pHeader);

	is_valid_ =  (pImageHeader->ih_magic == __builtin_bswap32(IH_MAGIC));
	is_valid_ &= (pImageHeader->ih_load == __builtin_bswap32(IH_LOAD));
	is_valid_ &= (pImageHeader->ih_ep == __builtin_bswap32(IH_EP));
	is_valid_ &= (pImageHeader->ih_os == IH_OS_U_BOOT);
	is_valid_ &= (pImageHeader->ih_arch == IH_ARCH_ARM);
	is_valid_ &= (pImageHeader->ih_type == IH_TYPE_STANDALONE);
	is_valid_ &= (strncmp(reinterpret_cast<const char *>(pImageHeader->ih_name), "http://www.orangepi-dmx.org", IH_NMLEN) == 0);

	is_compressed_ = (pImageHeader->ih_comp == IH_COMP_GZIP);

#if !defined (CONFIG_UBOOT_COMPRESSED)
	is_valid_ &= !is_compressed_;
#endif

	TImageHeader ih;
	memcpy(&ih, pImageHeader, sizeof(struct TImageHeader));
	ih.ih_hcrc = 0;
	const auto hcrc = crc32(0, reinterpret_cast<const uint8_t *>(&ih), sizeof(struct TImageHeader));
	is_valid_ &= (__builtin_bswap32(pImageHeader->ih_hcrc) == hcrc);
}

void UBootHeader::Dump() {
	if (!is_valid_) {
		printf("* Not a valid header! *\n");
	}

	auto *pImageHeader = reinterpret_cast<const TImageHeader *>(header_);
	const auto rawtime = static_cast<time_t>(__builtin_bswap32(pImageHeader->ih_time));
	auto *info = localtime(&rawtime);

	printf("Magic Number        : %.8x\n", __builtin_bswap32(pImageHeader->ih_magic));
	printf("Header CRC Checksum : %.8x\n", __builtin_bswap32(pImageHeader->ih_hcrc));
	printf("Creation Timestamp  : %.8x - %s", __builtin_bswap32(pImageHeader->ih_time), asctime(info));
	printf("Data Size           : %.8x - %d kBytes\n", __builtin_bswap32(pImageHeader->ih_size), __builtin_bswap32(pImageHeader->ih_size) / 1024);
	printf("Data Load Address   : %.8x\n", __builtin_bswap32(pImageHeader->ih_load));
	printf("Entry Point Address : %.8x\n", __builtin_bswap32(pImageHeader->ih_ep));
	printf("Image CRC Checksum  : %.8x\n", __builtin_bswap32(pImageHeader->ih_dcrc));
	printf("Operating System    : %d - %s\n", pImageHeader->ih_os, pImageHeader->ih_os == IH_OS_U_BOOT ? "Firmware" : "Not supported");
	printf("CPU architecture    : %d - %s\n", pImageHeader->ih_arch, pImageHeader->ih_arch == IH_ARCH_ARM ? "Arm" : "Not supported");
	printf("Image type          : %d - %s\n", pImageHeader->ih_type, pImageHeader->ih_type == IH_TYPE_STANDALONE ? "Standalone Program" : "Not supported");
	printf("Compression         : %d - %s\n", pImageHeader->ih_comp, pImageHeader->ih_comp == IH_COMP_NONE ? "none" : (pImageHeader->ih_comp == IH_COMP_GZIP ? "gzip" : "Not supported"));
	printf("Image Name          : %s\n", pImageHeader->ih_name);
}
