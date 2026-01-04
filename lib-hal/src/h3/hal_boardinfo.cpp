/*
 * hal_boardinfo.cpp
 */

#include <cstdint>

#include <h3_board.h>

namespace hal
{
const char* BoardName(uint8_t& length)
{
    length = sizeof(H3_BOARD_NAME) - 1;
    return H3_BOARD_NAME;
}

const char* SocName(uint8_t& length)
{
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

const char* CpuName(uint8_t& length)
{
    static constexpr const char kCpuName[] = "Cortex-A7";
    static constexpr auto kCpuNameLength = sizeof(kCpuName) - 1;
    length = kCpuNameLength;
    return kCpuName;
}

const char* MachineName(uint8_t& length)
{
    static constexpr const char kMachineName[] = "arm";
    static constexpr auto kMachineNameLength = sizeof(kMachineName) - 1;
    length = kMachineNameLength;
    return kMachineName;
}

const char* SysName(uint8_t& length)
{
    static constexpr const char kSysName[] = "Baremetal";
    static constexpr auto kSysNameLength = sizeof(kSysName) - 1;
    length = kSysNameLength;
    return kSysName;
}
} // namespace hal