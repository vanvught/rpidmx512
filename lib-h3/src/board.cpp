/**
 * @file board.cpp
 *
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

#include "board.h"
#include "h3_board.h"

namespace board {
static constexpr float kCoreTemperatureMin = -40.0;
static constexpr float kCoreTemperatureMax = +90.0;

const char* Website() {
    return "www.orangepi-dmx.org";
}

const char* BoardName(uint8_t& length) {
    length = sizeof(H3_BOARD_NAME) - 1;
    return H3_BOARD_NAME;
}

const char* SysName(uint8_t& length) {
    static constexpr const char kSysName[] = "Baremetal";
    static constexpr auto kSysNameLength = sizeof(kSysName) - 1;
    length = kSysNameLength;
    return kSysName;
}

const char* SocName(uint8_t& length) {
#if defined(ORANGE_PI)
    static constexpr const char kSocName[] = "H2+";
    static constexpr auto kSocNameLength = sizeof(kSocName) - 1;
#elif defined(ORANGE_PI_ONE)
    static constexpr char kSocName[] = "H3";
    static constexpr auto kSocNameLength = sizeof(kSocName) - 1;
#endif
    length = kSocNameLength;
    return kSocName;
}

const char* CpuName(uint8_t& length) {
    static constexpr const char kCpuName[] = "Cortex-A7";
    static constexpr auto kCpuNameLength = sizeof(kCpuName) - 1;
    length = kCpuNameLength;
    return kCpuName;
}

const char* MachineName(uint8_t& length) {
    static constexpr const char kMachineName[] = "arm";
    static constexpr auto kMachineNameLength = sizeof(kMachineName) - 1;
    length = kMachineNameLength;
    return kMachineName;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"

// https://github.com/linux-sunxi/sunxi-tools/blob/master/uart0-helloworld-sdboot.c#L598
BootDevice GetBootDevice() {
    auto* spl_signature = reinterpret_cast<volatile uint32_t*>(0x4);

    /* Check the eGON.BT0 magic in the SPL header */
    if (spl_signature[0] != 0x4E4F4765 || spl_signature[1] != 0x3054422E) {
        return BootDevice::kFel;
    }

    const uint32_t kBootDev = spl_signature[9] & 0xFF; /* offset into SPL = 0x28 */

    if (kBootDev == 0) {
        return BootDevice::kMmc0;
    }

    if (kBootDev == 3) {
        return BootDevice::kSpi;
    }

    return BootDevice::kUnkown;
}

float CoreTemperatureMin() {
    return kCoreTemperatureMin;
}
float CoreTemperatureMax() {
    return kCoreTemperatureMax;
}
} // namespace board
