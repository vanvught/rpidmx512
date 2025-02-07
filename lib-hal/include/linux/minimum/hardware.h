/**
 * @file hardware.h
 *
 */
/* Copyright (C) 2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef LINUX_MINIMUM_HARDWARE_H_
#define LINUX_MINIMUM_HARDWARE_H_

#include <cstdint>
#include <cstring>
#include <sys/time.h>
#include <uuid/uuid.h>

namespace hal {
uint32_t get_uptime();
}  // namespace hal

class Hardware {
public:
	static Hardware *Get() {
		static Hardware instance;
		return &instance;
	}

	uint32_t GetUpTime() {
		return ::hal::get_uptime();
	}

	uint32_t Millis() {
		struct timeval tv;
		gettimeofday(&tv, nullptr);

#if defined (__APPLE__)
		return (tv.tv_sec * static_cast<__darwin_time_t>(1000) + (tv.tv_usec / static_cast<__darwin_time_t>(1000)));
#else
		return static_cast<uint32_t>(tv.tv_sec * static_cast<__time_t >(1000) + (tv.tv_usec / static_cast<__suseconds_t >(1000)));
#endif
	}

	/*
	 * Not implemented
	 */

	void Print() {}
	bool Reboot() { return false;}
	void SetMode([[maybe_unused]] const hardware::ledblink::Mode mode) {}
	const char *GetBoardName(uint8_t& nLength) {
		nLength = 0;
		return "";
	}

	bool IsWatchdog() { return false;}
	void WatchdogInit() { }
	void WatchdogFeed() { }
	void WatchdogStop() { }

private:
	Hardware() {}

private:
	uuid_t m_uuid;
 };

#endif /* LINUX_MINIMUM_HARDWARE_H_ */
