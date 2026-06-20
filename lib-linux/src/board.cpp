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

#if defined(DEBUG_HAL)
#undef NDEBUG
#endif

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/reboot.h>
#include <sys/utsname.h>
#include <uuid/uuid.h>
#include "exec_cmd.h"
#if !defined(DISABLE_RTC)
#include "hwclock.h"
#endif
#include "board.h"
#include "spi.h"
#include "i2c.h"
#include "uuid.h"
#include "configstore.h"
#include "firmware/debug/debug_debug.h"
#if defined(DEBUG_I2C)
#include "firmware/debug/debug_i2cdetect.h"
#endif

static constexpr char kUnknown[] = "Unknown";

static struct utsname s_utsname;
static char s_cpu_name[64];
static char s_soc_name[64];
static char s_board_name[64];

#if !defined(DISABLE_RTC)
static HwClock hw_clock;
#endif

static char* StrFindReplace(char* str, const char* find, const char* replace) {
    assert(strlen(replace) <= strlen(find));

    unsigned i, j, k, n, m;

    i = j = m = n = 0;

    while (str[i] != '\0') {
        if (str[m] == find[n]) {
            m++;
            n++;
            if (find[n] == '\0') {
                for (k = 0; replace[k] != '\0'; k++, j++) {
                    str[j] = replace[k];
                }
                n = 0;
                i = m;
            }
        } else {
            str[j] = str[i];
            j++;
            i++;
            m = i;
            n = 0;
        }
    }

    for (; j < i; j++) {
        str[j] = '\0';
    }

    return str;
}

namespace board {
void Init() {
    DEBUG_ENTRY();

    memset(&s_utsname, 0, sizeof(struct utsname));

    strcpy(s_cpu_name, kUnknown);
    strcpy(s_board_name, kUnknown);
    s_soc_name[0] = '\0';

    uname(&s_utsname);

    { // Board Name
#if defined(__APPLE__)
        constexpr char kCat[] = "sysctl -n hw.model";
        exec_cmd(kCat, s_board_name, sizeof(s_board_name));
#else
        constexpr char kCat[] = "cat /sys/firmware/devicetree/base/model";
        if (!exec_cmd(kCat, s_board_name, sizeof(s_board_name))) {
            constexpr char kCat[] = "cat /sys/class/dmi/id/board_name";
            exec_cmd(kCat, s_board_name, sizeof(s_board_name));
        }
#endif
        StrFindReplace(s_board_name, "Rev ", "V");
    }

    { // SoC Name
        constexpr char kCmd[] = "cat /proc/cpuinfo | grep 'Hardware' | awk '{print $3}'";
        exec_cmd(kCmd, s_soc_name, sizeof(s_soc_name));
    }

    { // CPU Name
#if defined(__APPLE__)
        constexpr char kCmd[] = "sysctl -n machdep.cpu.brand_string";
#else
        constexpr char kCmd[] = "cat /proc/cpuinfo | grep 'model name' | head -n 1 | sed 's/^[^:]*://g' |  sed 's/^[^ ]* //g'";
#endif
        exec_cmd(kCmd, s_cpu_name, sizeof(s_cpu_name));
    }

    i2c::Begin();
    spi::Begin();

#if defined(DEBUG_I2C)
    debug::i2c::Detect();
#endif

#if !defined(DISABLE_RTC)
    hw_clock.RtcProbe();
    hw_clock.Print();
    hw_clock.SysToHc();
#endif

    DEBUG_EXIT();
}

float CoreTemperatureCurrent() {
#if defined(RASPI)
    const char kCmd[] = "vcgencmd measure_temp| egrep \"[0-9.]{4,}\" -o";
    char result[8];

    exec_cmd(kCmd, result, sizeof(result));

    return atof(result);
#elif defined(__APPLE__)
    return -1.0f;
#else
    const char kCmd[] = "sensors | grep 'Core 0' | awk '{print $3}' | cut -c2-3";
    char result[6];

    exec_cmd(kCmd, result, sizeof(result));

    return atof(result);
#endif
}
} // namespace board

bool PowerOff() {
#if defined(__APPLE__)
    return false;
#else
    if (geteuid() == 0) {
        sync();

        if (reboot(RB_POWER_OFF) == 0) {
            return true;
        }

        perror("Call to reboot(RB_POWER_OFF) failed.\n");
        return false;
    }

    printf("Only the superuser may call reboot(RB_POWER_OFF).\n");
    return false;
#endif
}

namespace board {
const char* BoardName(uint8_t& length) {
    length = strlen(s_board_name);
    return s_board_name;
}

const char* SysName(uint8_t& length) {
    length = strlen(s_utsname.sysname);
    return s_utsname.sysname;
}

const char* SocName(uint8_t& length) {
    length = strlen(s_soc_name);
    return s_soc_name;
}

const char* CpuName(uint8_t& length) {
    length = strlen(s_cpu_name);
    return s_cpu_name;
}

bool Reboot() {
    ConfigstoreCommit();
#if defined(__APPLE__)
    return false;
#else
    if (geteuid() == 0) {
        sync();

        if (reboot(RB_AUTOBOOT) == 0) {
            return true;
        }

        perror("Call to reboot(RB_AUTOBOOT) failed.\n");
    }
    printf("Only the superuser may call reboot(RB_AUTOBOOT).\n");
#endif
    return false;
}

const char* Website() {
    return "https://gd32-dmx.org";
}

void Print() {
    static constexpr auto kUuidStringLength = 36;
    char uuid_str[kUuidStringLength + 1];
    uuid_str[kUuidStringLength] = '\0';

    uuid_t out;
    UuidCopy(out);

    uuid_unparse(out, uuid_str);

    printf("CPU  : %s\n", s_cpu_name);
    printf("SoC  : %s\n", s_soc_name);
    printf("Board: %s\n", s_board_name);
    printf("UUID : %s\n", uuid_str);
}
} // namespace board