/**
 * @file oscsimplesend.h
 *
 */
/* Copyright (C) 2020-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef OSCSIMPLESEND_H_
#define OSCSIMPLESEND_H_

#include <cstdint>

class OscSimpleSend
{
   public:
    static constexpr uint32_t kBufferSize = 512U;

    // Support for path only
    OscSimpleSend(int32_t handle, uint32_t ip_address, uint16_t port, const char* path, const char* type);
    // Support for 's'
    OscSimpleSend(int32_t handle, uint32_t ip_address, uint16_t port, const char* path, const char* type, const char* string);
    // Support for type 'i'
    OscSimpleSend(int32_t handle, uint32_t ip_address, uint16_t port, const char* path, const char* type, int value);
    // Support for type 'f'
    OscSimpleSend(int32_t handle, uint32_t ip_address, uint16_t port, const char* path, const char* type, float value);

   private:
    void UpdateMessage(const char* path, uint32_t path_length, char type);
    void Send(uint32_t message_length, int32_t handle, uint32_t ip_address, uint16_t port);

   private:
    inline static uint8_t s_message[kBufferSize];
};

#endif  // OSCSIMPLESEND_H_
