#if !defined(CONFIG_HAL_USE_MINIMUM)
/**
 * @file hal_init.cpp
 *
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "hal_i2c.h"
#include "hal_spi.h"

#if defined(DEBUG_I2C)
#include "i2cdetect.h"
#endif

#include "hal.h"

 #include "firmware/debug/debug_debug.h"

static constexpr char UNKNOWN[] = "Unknown";

#if defined(__linux__)
static constexpr char RASPBIAN_LED_INIT[] = "echo gpio | sudo tee /sys/class/leds/led0/trigger";
static constexpr char RASPBIAN_LED_OFF[] = "echo 0 | sudo tee /sys/class/leds/led0/brightness";
static constexpr char RASPBIAN_LED_ON[] = "echo 1 | sudo tee /sys/class/leds/led0/brightness";
static constexpr char RASPBIAN_LED_HB[] = "echo heartbeat | sudo tee /sys/class/leds/led0/trigger";
static constexpr char RASPBIAN_LED_FLASH[] = "echo timer | sudo tee /sys/class/leds/led0/trigger";
#endif

static Board m_boardType;
static struct utsname m_TOsInfo;
static char m_aCpuName[64];
static char m_aSocName[64];
static char m_aBoardName[64];
static uint32_t m_nBoardId;

#if !defined(__APPLE__)
extern "C"
{
    int __attribute__((weak)) bcm2835_init(void)
    {
        return 0;
    }
}
#endif

#if !defined(DISABLE_RTC)
static HwClock hwClock;
#endif

namespace hal
{
void UuidCopy(uuid_t out);
} // namespace hal

static char* str_find_replace(char* str, const char* find, const char* replace)
{
    assert(strlen(replace) <= strlen(find));

    unsigned i, j, k, n, m;

    i = j = m = n = 0;

    while (str[i] != '\0')
    {
        if (str[m] == find[n])
        {
            m++;
            n++;
            if (find[n] == '\0')
            {
                for (k = 0; replace[k] != '\0'; k++, j++)
                {
                    str[j] = replace[k];
                }
                n = 0;
                i = m;
            }
        }
        else
        {
            str[j] = str[i];
            j++;
            i++;
            m = i;
            n = 0;
        }
    }

    for (; j < i; j++)
    {
        str[j] = '\0';
    }

    return str;
}

namespace hal
{
void Init()
{
    DEBUG_ENTRY();

    memset(&m_TOsInfo, 0, sizeof(struct utsname));

    strcpy(m_aCpuName, UNKNOWN);
    strcpy(m_aBoardName, UNKNOWN);
    m_aSocName[0] = '\0';

#if defined(__linux__)
    m_boardType = Board::TYPE_LINUX;
#elif defined(__APPLE__)
    m_boardType = Board::TYPE_OSX;
#else
    m_boardType = Board::TYPE_UNKNOWN;
#endif
#if defined(__linux__)
    constexpr char cmd[] = "which vcgencmd";
    char buf[16];

    FILE* fp = popen(cmd, "r");

    if (fgets(buf, sizeof(buf) - 1, fp) != 0)
    {
        m_boardType = Board::TYPE_RASPBIAN;
        if (system(RASPBIAN_LED_INIT) == 0)
        {
            // Just make the compile happy
        }
    }

    if (fp != nullptr)
    {
        pclose(fp);
    }
#endif

    if (m_boardType != Board::TYPE_UNKNOWN)
    {
        uname(&m_TOsInfo);
    }

#ifndef NDEBUG
    printf("m_boardType=%d\n", static_cast<int>(m_boardType));
#endif

#if !defined(__APPLE__)
    if (m_boardType == Board::TYPE_RASPBIAN)
    {
        if (getuid() != 0)
        {
            fprintf(stderr, "Program is not started as \'root\' (sudo)\n");
            exit(-1);
        }

        if (bcm2835_init() == 0)
        {
            fprintf(stderr, "Function bcm2835_init() failed\n");
            exit(-1);
        }
    }
#endif

    { // Board Name
#if defined(__APPLE__)
        constexpr char cat[] = "sysctl -n hw.model";
        exec_cmd(cat, m_aBoardName, sizeof(m_aBoardName));
#elif defined(__linux__)
        constexpr char cat[] = "cat /sys/firmware/devicetree/base/model";
        if (!exec_cmd(cat, m_aBoardName, sizeof(m_aBoardName)))
        {
            constexpr char cat[] = "cat /sys/class/dmi/id/board_name";
            exec_cmd(cat, m_aBoardName, sizeof(m_aBoardName));
        }
#endif
        str_find_replace(m_aBoardName, "Rev ", "V");
    }

    { // SoC Name
        constexpr char cmd[] = "cat /proc/cpuinfo | grep 'Hardware' | awk '{print $3}'";
        exec_cmd(cmd, m_aSocName, sizeof(m_aSocName));
    }

    { // CPU Name
#if defined(__APPLE__)
        constexpr char cmd[] = "sysctl -n machdep.cpu.brand_string";
#else
        constexpr char cmd[] = "cat /proc/cpuinfo | grep 'model name' | head -n 1 | sed 's/^[^:]*://g' |  sed 's/^[^ ]* //g'";
#endif
        exec_cmd(cmd, m_aCpuName, sizeof(m_aCpuName));
    }

    if (m_boardType == Board::TYPE_RASPBIAN)
    {
        char aResult[16];
        constexpr char cmd[] = "cat /proc/cpuinfo | grep 'Revision' | awk '{print $3}'";
        exec_cmd(cmd, aResult, sizeof(aResult));
        m_nBoardId = static_cast<uint32_t>(strtol(aResult, nullptr, 16));
    }

    FUNC_PREFIX(I2cBegin());
    FUNC_PREFIX(SpiBegin());

#if defined(DEBUG_I2C)
    I2cDetect();
#endif

#if !defined(DISABLE_RTC)
    hwClock.RtcProbe();
    hwClock.Print();
    hwClock.SysToHc();
#endif

    DEBUG_EXIT();
}

const char* BoardName(uint8_t& length)
{
    length = strlen(m_aBoardName);
    return m_aBoardName;
}

const char* SocName(uint8_t& length)
{
    length = strlen(m_aSocName);
    return m_aSocName;
}

const char* CpuName(uint8_t& length)
{
    length = strlen(m_aCpuName);
    return m_aCpuName;
}

const char* MachineName(uint8_t& length)
{
    length = strlen(m_TOsInfo.machine);
    return m_TOsInfo.machine;
}

const char* SysName(uint8_t& length)
{
    length = strlen(m_TOsInfo.sysname);
    return m_TOsInfo.sysname;
}

bool Reboot()
{
#if defined(__APPLE__)
    return false;
#else
    if (geteuid() == 0)
    {
        sync();

        if (reboot(RB_AUTOBOOT) == 0)
        {
            return true;
        }

        perror("Call to reboot(RB_AUTOBOOT) failed.\n");
    }
    printf("Only the superuser may call reboot(RB_AUTOBOOT).\n");
#endif
    return false;
}

float CoreTemperatureCurrent()
{
#if defined(__linux__)
    if (m_boardType == Board::TYPE_RASPBIAN)
    {
        const char cmd[] = "vcgencmd measure_temp| egrep \"[0-9.]{4,}\" -o";
        char aResult[8];

        exec_cmd(cmd, aResult, sizeof(aResult));

        return atof(aResult);
    }
    else
    {
        const char cmd[] = "sensors | grep 'Core 0' | awk '{print $3}' | cut -c2-3";
        char aResult[6];

        exec_cmd(cmd, aResult, sizeof(aResult));

        return atof(aResult);
    }
#endif
    return -1.0f;
}
} // namespace hal

Board linux_board_type()
{
    return m_boardType;
}

void linux_print()
{
    static constexpr auto UUID_STRING_LENGTH = 36;
    char uuid_str[UUID_STRING_LENGTH + 1];
    uuid_str[UUID_STRING_LENGTH] = '\0';

    uuid_t out;
    hal::UuidCopy(out);

    uuid_unparse(out, uuid_str);

    printf("CPU  : %s\n", m_aCpuName);
    printf("SoC  : %s\n", m_aSocName);
    printf("Board: %s\n", m_aBoardName);
    printf("UUID : %s\n", uuid_str);
}

// void linux_soft_reset() {
//	if (m_argv != 0) {
//		sync();
//
//		if (m_pSoftResetHandler != 0) {
//			m_pSoftResetHandler->Run();
//		}
//
//		if (execve(m_argv[0], m_argv, nullptr) == -1) {
//			perror("call to execve failed.\n");
//		}
//	}
// }

bool linux_power_off()
{
#if defined(__APPLE__)
    return false;
#else
    if (geteuid() == 0)
    {
        sync();

        if (reboot(RB_POWER_OFF) == 0)
        {
            return true;
        }

        perror("Call to reboot(RB_POWER_OFF) failed.\n");
        return false;
    }

    printf("Only the superuser may call reboot(RB_POWER_OFF).\n");
    return false;
#endif
}
#endif
