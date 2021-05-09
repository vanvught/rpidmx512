/**
 * @file bw.h
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

#ifndef BW_H_
#define BW_H_

#include <cstdint>
#include <string.h>
#include <algorithm>

#include "hal_spi.h"

/*
 * http://www.bitwizard.nl/wiki/index.php/Default_addresses
 */

namespace bw {
namespace id_string {
static constexpr size_t length = 0;
}  // namespace id_string

namespace spi {
namespace speed {
static constexpr uint32_t max_hz = 50000;		///< 50 kHz
static constexpr uint32_t default_hz = 50000;	///< 50 kHz
}  // namespace speed
}  // namespace spi

namespace lcd {
static constexpr uint8_t address = 0x82;
static constexpr char id_string[] = "spi_lcd";
static constexpr uint8_t max_characters = 16;
static constexpr uint8_t max_lines = 2;
namespace spi {
static constexpr uint32_t write_delay_us = 12;
}  // namespace spi
}  // namespace lcd

namespace dio {
static constexpr uint8_t address = 0x84;
static constexpr char id_string[] = "spi_dio";
}  // namespace dio

namespace fets {
static constexpr uint8_t address = 0x88;
static constexpr char id_string[] = "spi_7fets";
}  // namespace fets

namespace relay {
static constexpr uint8_t address = 0x8E;
static constexpr char id_string[] = "spi_relay";
}  // namespace relay

namespace dimmer {
static constexpr uint8_t address = 0x9E;
static constexpr char id_string[] = "spi_dimmer";
}  // namespace dimmer

namespace port {
namespace read {
static constexpr uint8_t id_string = 0x01;
}  // namespace read
namespace write {
static constexpr uint8_t set_all_outputs = 0x10;
static constexpr uint8_t io_direction = 0x30;

static constexpr uint8_t display_data = 0x00;
static constexpr uint8_t clear_screen = 0x10;	///< any data clears the screen
static constexpr uint8_t move_cursor = 0x11;
static constexpr uint8_t reinit_lcd = 0x14;

}  // namespace write
}  // namespace port

}  // namespace bw

class BwSpi: public HAL_SPI {
public:
	BwSpi(uint8_t nChipSelect, uint8_t nAddress, const char *pIdString) : HAL_SPI(nChipSelect, bw::spi::speed::default_hz), m_nAddress(nAddress) {
		char spiBuffer[bw::id_string::length + 2];

		spiBuffer[0] = static_cast<char>(m_nAddress | 1);
		spiBuffer[1] = bw::port::read::id_string;

		HAL_SPI::WriteRead(spiBuffer, sizeof(spiBuffer));

		if (pIdString != nullptr) {
			const auto length = std::min(bw::id_string::length, strlen(pIdString));
			m_IsConnected = (strncmp(&spiBuffer[2], pIdString, length) == 0);
		}
	}

protected:
	uint8_t m_nAddress;
	bool m_IsConnected = false;
};

#endif /* BW_H_ */
