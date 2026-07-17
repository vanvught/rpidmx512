/**
 * @file json_status_dmx.cpp
 */
 /* Copyright (C) 2025-206 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "dmx.h"

namespace json::status
{
uint32_t Dmx(char* out_buffer, uint32_t out_buffer_size, uint32_t port_index) {
    if (port_index < ::dmx::config::max::kPorts)
    {
        auto& statistics = Dmx::Get()->GetTotalStatistics(port_index);
        auto length = static_cast<uint32_t>(snprintf(out_buffer, out_buffer_size,
         "{\"port\":\"%c\","
         "\"dmx\":{\"sent\":\"%u\",\"received\":\"%u\"},"
         "\"rdm\":{\"sent\":{\"class\":\"%u\",\"discovery\":\"%u\"},\"received\":{\"good\":\"%u\",\"bad\":\"%u\",\"discovery\":\"%u\"}}}",
         static_cast<char>('A' + port_index), static_cast<unsigned int>(statistics.dmx.sent), static_cast<unsigned int>(statistics.dmx.received),
         static_cast<unsigned int>(statistics.rdm.sent.classes), static_cast<unsigned int>(statistics.rdm.sent.discovery_response), static_cast<unsigned int>(statistics.rdm.received.good),
         static_cast<unsigned int>(statistics.rdm.received.bad), static_cast<unsigned int>(statistics.rdm.received.discovery_response)));

        return length;
    }

    return 0;	
}

uint32_t Dmx(char* out_buffer, uint32_t out_buffer_size) {
    out_buffer[0] = '[';
    uint32_t length = 1;

    for (uint32_t port_index = 0; port_index < ::dmx::config::max::kPorts; port_index++)
    {
        length += Dmx(&out_buffer[length], out_buffer_size - length, port_index);
		out_buffer[length++] = ',';
    }

    out_buffer[length - 1] = ']';

    return length;	
}
}  // namespace json::status
