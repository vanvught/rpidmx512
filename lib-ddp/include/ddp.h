/**
 * @file ddp.h
 *
 */

/* Copyright (C) 2021-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef DDP_H_
#define DDP_H_

#include <cstdint>

namespace ddp {
struct Header {
	uint8_t flags1;
	uint8_t flags2;
	uint8_t type;
	uint8_t id;
	uint8_t offset[4];
	uint8_t len[2];
} __attribute__((packed));

static constexpr auto HEADER_LEN = (sizeof(struct Header));
static constexpr auto DATA_LEN = 1440;
static constexpr auto PACKET_LEN = (HEADER_LEN + DATA_LEN);

struct Packet {
	struct Header header;
	uint8_t data[DATA_LEN];
}__attribute__((packed));

namespace flags1 {
static constexpr uint8_t VER_MASK = 0xc0;
static constexpr uint8_t VER1 = 0x40;
static constexpr uint8_t PUSH = 0x01;
static constexpr uint8_t QUERY = 0x02;
static constexpr uint8_t REPLY = 0x04;
static constexpr uint8_t STORAGE = 0x08;
static constexpr uint8_t TIME = 0x10;
}  // namespace flags1

namespace id {
static constexpr uint8_t DISPLAY = 1;
static constexpr uint8_t CONTROL = 246;
static constexpr uint8_t CONFIG = 250;
static constexpr uint8_t STATUS = 251;
static constexpr uint8_t DMXTRANSIT = 254;
static constexpr uint8_t ALLDEVICES = 255;
}  // namespace id
static constexpr uint16_t UDP_PORT = 4048;
}  // namespace ddp

#endif /* DDP_H_ */
