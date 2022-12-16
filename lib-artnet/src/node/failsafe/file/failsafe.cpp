/**
 * @file failsafe.cpp
 *
 */
/* Copyright (C) 2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifdef NDEBUG
# undef NDEBUG
#endif

#include <cstdint>
#include <cstdio>
#include <cassert>
#include <unistd.h>

#include "artnetnode.h"
#include "artnetnodefailsafe.h"

#include "debug.h"

namespace artnetnode {

static constexpr char FILE_NAME[] = "failsafe.bin";

static FILE *pFile;

void failsafe_write_start() {
	DEBUG_ENTRY

	if ((pFile = fopen(FILE_NAME, "r+")) == nullptr) {
		perror("fopen r+");

		if ((pFile = fopen(FILE_NAME, "w+")) == nullptr) {
			perror("fopen w+");

			DEBUG_EXIT
			return;
		}

		for (uint32_t i = 0; i < failsafe::BYTES_NEEDED; i++) {
			if (fputc(0xFF, pFile) == EOF) {
				perror("fputc(0xFF, file)");	// Same as erasing a flash memory device

				if (fclose(pFile) != 0) {
					perror("flcose");
				}

				pFile = nullptr;
				DEBUG_EXIT
				return;
			}

		}

		if (fflush(pFile) != 0) {
			perror("fflush");
		}
	}

	DEBUG_EXIT
}

void failsafe_write(uint32_t nPortIndex, const uint8_t *pData) {
	DEBUG_ENTRY
	assert(nPortIndex < artnetnode::MAX_PORTS);
	assert(pData != nullptr);

	if (fseek(pFile, static_cast<long int>(nPortIndex * lightset::dmx::UNIVERSE_SIZE), SEEK_SET) != 0) {
		perror("fseek");
		DEBUG_EXIT
		return;
	}

	if (fwrite(pData, 1, lightset::dmx::UNIVERSE_SIZE, pFile) != lightset::dmx::UNIVERSE_SIZE) {
		perror("fwrite");
		DEBUG_EXIT
		return;
	}

	DEBUG_EXIT
}

void failsafe_write_end() {
	DEBUG_ENTRY

	if (pFile != nullptr) {
		if (fclose(pFile) != 0) {
			perror("flcose");
		}
		pFile = nullptr;
	}

	DEBUG_EXIT
}


void failsafe_read_start() {
	DEBUG_ENTRY

	if ((pFile = fopen(FILE_NAME, "r")) == nullptr) {
		perror("fopen r");
		DEBUG_EXIT
	}

	DEBUG_EXIT
}

void failsafe_read(uint32_t nPortIndex, uint8_t *pData) {
	DEBUG_ENTRY
	assert(nPortIndex < artnetnode::MAX_PORTS);
	assert(pData != nullptr);

	if (pFile == nullptr) {
		DEBUG_EXIT
		return;
	}

	if (fseek(pFile, static_cast<long int>(nPortIndex * lightset::dmx::UNIVERSE_SIZE), SEEK_SET) != 0) {
		perror("fseek");
		DEBUG_EXIT
		return;
	}

	if (fread(pData, 1, lightset::dmx::UNIVERSE_SIZE, pFile) != lightset::dmx::UNIVERSE_SIZE) {
		perror("fread");
		DEBUG_EXIT
		return;
	}

	DEBUG_EXIT
}

void failsafe_read_end() {
	DEBUG_ENTRY

	if (pFile != nullptr) {
		if (fclose(pFile) != 0) {
			perror("flcose");
		}
		pFile = nullptr;
	}

	DEBUG_EXIT
}
}  // namespace artnetnode
