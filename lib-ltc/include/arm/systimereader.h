/**
 * @file systimereader.h
 *
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef ARM_SYSTIMEREADER_H_
#define ARM_SYSTIMEREADER_H_

#include <cstdint>
#include <time.h>

#include "ltc.h"

class SystimeReader
{
   public:
    SystimeReader(uint8_t fps, int32_t utc_offset);

    void Start(bool start = false);
    void Run();

    void HandleRequest(char* buffer = nullptr, uint16_t buffer_length = 0);

    void ActionStart();
    void ActionStop();
    void ActionSetRate(const char* rate);

    void Input(const uint8_t* buffer, uint32_t size, uint32_t from_ip, uint16_t from_port);

    static SystimeReader* Get() { return s_this; }

   private:
    void SetFps(uint8_t fps);
    void static StaticCallbackFunction(const uint8_t* buffer, uint32_t size, uint32_t from_ip, uint16_t from_port)
    {
        s_this->Input(buffer, size, from_ip, from_port);
    }

   private:
    uint8_t fps_;
    int32_t utc_offset_{0};
    int32_t handle_{-1};
    uint32_t bytes_received_{0};
    char* udp_buffer_{nullptr};
    time_t time_previous_{0};
    bool started_{false};

    static inline SystimeReader* s_this;
};

#endif  // ARM_SYSTIMEREADER_H_
