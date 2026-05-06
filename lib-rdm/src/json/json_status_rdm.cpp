#if defined(RDM_CONTROLLER)
/**
 * @file json_status_rdm.cpp
 */
/* Copyright (C) 2025-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>
#include <cstdio>

#include "rdm_discovery.h"

namespace json::status {
static char ToChar(uint32_t port_index, uint8_t data) {
	const auto kIsSet = (data & (1U << port_index)) == (1U << port_index);
	return kIsSet ? 'y' : 'n';
}
static char ToType(uint32_t port_index, uint8_t data) {
	const auto kIsSet = (data & (1U << port_index)) == (1U << port_index);
	return kIsSet ? 'f' : 'i';
}
static uint32_t PortStatus(uint8_t data[5], char* out_buffer, uint32_t out_buffer_size, uint32_t port_index) {
    auto length = static_cast<uint32_t>(snprintf(out_buffer, out_buffer_size, 
		"{\"port\":\"%c\",\"enabled\":\"%c\",\"waiting\":\"%c\",\"type\":\"%c\",\"bg\":\"%c\",\"running\":\"%c\"},", 
		static_cast<char>('A' + port_index),
		ToChar(port_index, data[0]),
		ToChar(port_index, data[1]),
		ToType(port_index, data[2]),
		ToChar(port_index, data[3]),
		ToChar(port_index, data[4])));

    return length;
}

uint32_t Rdm(char* out_buffer, uint32_t out_buffer_size) {
    out_buffer[0] = '[';
    uint32_t length = 1;
	
	auto& discovery = rdm::Discovery::Instance();
	uint8_t data[5];
	discovery.GetStatus(data);

    for (uint32_t port_index = 0; port_index < rdm::Discovery::kPorts; port_index++) {
        length += PortStatus(data, &out_buffer[length], out_buffer_size - length, port_index);
    }

    if (length == 1) {
        length++;
    }

    out_buffer[length - 1] = ']';

    return length;
}

uint32_t RdmQueue(char* out_buffer, uint32_t out_buffer_size) {
    const auto kBufferSize = out_buffer_size - 2U;
    auto length = static_cast<uint32_t>(snprintf(out_buffer, kBufferSize, "{\"uid\":["));

    auto& discovery = rdm::Discovery::Instance();

    length += discovery.CopyWorkingQueue(&out_buffer[length], kBufferSize - length);

    out_buffer[length++] = ']';
    out_buffer[length++] = '}';

    assert(length <= out_buffer_size);
    return length;
}

static uint32_t CopyTod(uint32_t port_index, char* out_buffer, uint32_t out_buffer_size) {
    assert(port_index < rdm::Discovery::kPorts);

    const auto kSize = static_cast<int32_t>(out_buffer_size);
    int32_t length = 0;

    auto& discovery = rdm::Discovery::Instance();

    for (uint32_t count = 0; count < discovery.TodUidCount(port_index); count++) {
        uint8_t uid[rdm::kUidSize];

        discovery.TodCopyUidEntry(port_index, count, uid);

        length += snprintf(&out_buffer[length], static_cast<size_t>(kSize - length), 
		"\"%.2x%.2x:%.2x%.2x%.2x%.2x\",", uid[0], uid[1], uid[2], uid[3], uid[4], uid[5]);
    }

    if (length == 0) {
        return 0;
    }

    out_buffer[length - 1] = '\0';

    return static_cast<uint32_t>(length - 1);
}

uint32_t RdmTod(char* out_buffer, uint32_t out_buffer_size, uint32_t port_index) {
    if (port_index < rdm::Discovery::kPorts) {
        const auto kBufferSize = out_buffer_size - 2U;
        auto length = static_cast<uint32_t>(snprintf(out_buffer, kBufferSize, 
			"{\"port\":\"%c\",\"tod\":[", static_cast<char>(port_index + 'A')));

        length += CopyTod(port_index, &out_buffer[length], kBufferSize - length);

        out_buffer[length++] = ']';
        out_buffer[length++] = '}';

        assert(length <= out_buffer_size);
        return length;
    }

    return 0;
}
} // namespace json::status
#endif
