/**
 * @file ntpclient.cpp
 *
 */
/* Copyright (C) 2021-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "core/protocol/ntp.h"
#include "display.h"

namespace network::apps::ntpclient
{
void DisplayStatus(::ntp::Status status)
{
    switch (status)
    {
        case ::ntp::Status::kStopped:
            Display::Get()->TextStatus("No NTP Client");
            break;
        case ::ntp::Status::kIdle:
            Display::Get()->TextStatus("NTP Client");
            break;
        case ::ntp::Status::kLocked:
            Display::Get()->TextStatus("NTP Client LOCKED");
            break;
        case ::ntp::Status::kFailed:
            Display::Get()->TextStatus("Error: NTP");
            break;
        default:
            break;
    }
}
} // namespace network::apps::ntpclient
