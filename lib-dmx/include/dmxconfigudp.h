/**
 * @file dmxconfigudp.h
 *
 */
/* Copyright (C) 2021-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef DMXCONFIGUDP_H_
#define DMXCONFIGUDP_H_

#include <cstdint>
#include <cstring>
#include <cassert>

#include "dmxconst.h"
#include "network.h"
#include "dmx.h"

 #include "firmware/debug/debug_debug.h"

/**
 * @class DmxConfigUdp
 * @brief Handles UDP-based configuration of DMX parameters.
 *
 * This class listens for UDP messages to configure DMX settings such as
 * break time, MAB time, refresh rate, and the number of slots. The message
 * format is prefixed with "dmx!" followed by the configuration command.
 *
 * Example's udp message: dmx!break#100  dmx!refresh#30 dmx!mab#20 dmx!slots#128
 * min length: dmx!mab#12 => 10 bytes
 * max length: dmx!mab#1000000/n => 16 bytes
 */
class DmxConfigUdp
{
    static constexpr uint32_t kMinSize = 10;   ///< Minimum valid size for a UDP packet.
    static constexpr uint32_t kMaxSize = 16;   ///< Maximum valid size for a UDP packet.
    static constexpr uint16_t kUdpPort = 5120; ///< UDP port used for receiving DMX configuration messages.

    static constexpr const char kCmdBreak[] = "break#";
    static constexpr uint32_t kCmdBreakLength = sizeof(kCmdBreak) - 1U; // Exclude null terminator

    static constexpr const char kCmdMab[] = "mab#";
    static constexpr uint32_t kCmdMabLength = sizeof(kCmdMab) - 1U;

    static constexpr const char kCmdRefresh[] = "refresh#";
    static constexpr uint32_t kCmdRefreshLength = sizeof(kCmdRefresh) - 1U;

    static constexpr const char kCmdSlots[] = "slots#";
    static constexpr uint32_t kCmdSlotsLength = sizeof(kCmdSlots) - 1U;

    /**
     * @brief Validates if a value lies within a specified range.
     *
     * @tparam T Data type of the value being validated.
     * @param n The value to check.
     * @param min Minimum permissible value.
     * @param max Maximum permissible value.
     * @return true if `n` is within `[min, max]`, false otherwise.
     */
    template <class T> static constexpr bool Validate(const T& n, const T& min, const T& max) { return (n >= min) && (n <= max); }

    /**
     * @brief Converts a string to an unsigned integer.
     *
     * @param buffer Pointer to the string buffer.
     * @param size Length of the string.
     * @return Converted unsigned integer value.
     */
    inline uint32_t Atoi(const char* buffer, uint32_t size)
    {
        uint32_t result = 0;
        for (uint32_t i = 0; i < size && buffer[i] >= '0' && buffer[i] <= '9'; ++i)
        {
            result = result * 10 + static_cast<uint32_t>(buffer[i] - '0');
        }
        return result;
    }

   public:
    /**
     * @brief Constructs the DmxConfigUdp object and initializes UDP listening.
     */
    DmxConfigUdp()
    {
        DEBUG_ENTRY();

        assert(s_this == nullptr);
        s_this = this;

        assert(handle_ == -1);
        handle_ = network::udp::Begin(kUdpPort, DmxConfigUdp::StaticCallbackFunction);

        DEBUG_EXIT();
    }

    /**
     * @brief Destroys the DmxConfigUdp object and stops UDP listening.
     */
    ~DmxConfigUdp()
    {
        DEBUG_ENTRY();

        assert(handle_ != -1);
        network::udp::End(kUdpPort);
        handle_ = -1;

        s_this = nullptr;

        DEBUG_EXIT();
    }

    /**
     * @brief Processes an incoming UDP packet.
     *
     * @param buffer Pointer to the packet buffer.
     * @param size Size of the packet buffer.
     * @param from_ip IP address of the sender.
     * @param from_port Port number of the sender.
     */
    void Input(const uint8_t* buffer, uint32_t size, [[maybe_unused]] uint32_t from_ip, [[maybe_unused]] uint16_t from_port)
    {
        DEBUG_ENTRY();

        if (!Validate(size, kMinSize, kMaxSize))
        {
            DEBUG_EXIT();
            return;
        }

        if (memcmp("dmx!", buffer, 4) != 0)
        {
            DEBUG_EXIT();
            return;
        }

        if (buffer[size - 1] == '\n')
        {
            size--;
        }

        DEBUG_PRINTF("size=%u", size);

        struct CommandHandler
        {
            const char* command;
            const uint32_t kCommandLength;
            void (DmxConfigUdp::*handler)(const uint8_t*, uint32_t);
        };

        constexpr CommandHandler kCommandHandlers[] = {{kCmdBreak, kCmdBreakLength, &DmxConfigUdp::HandleBreak},
                                                       {kCmdMab, kCmdMabLength, &DmxConfigUdp::HandleMab},
                                                       {kCmdRefresh, kCmdRefreshLength, &DmxConfigUdp::HandleRefresh},
                                                       {kCmdSlots, kCmdSlotsLength, &DmxConfigUdp::HandleSlots}};

        for (const auto& handler : kCommandHandlers)
        {
            if (memcmp(handler.command, &buffer[4], handler.kCommandLength) == 0)
            {
                (this->*handler.handler)(buffer, size);

                DEBUG_EXIT();
                return;
            }
        }

        DEBUG_EXIT();
    }

   private:
    /**
     * @brief Handles the "break#" command to configure DMX break time.
     *
     * @param buffer Pointer to the packet buffer.
     * @param size Size of the packet buffer.
     */
    void HandleBreak(const uint8_t* buffer, uint32_t size)
    {
        static constexpr auto kOffset = kCmdBreakLength + 4;
        const auto kBreakTime = Atoi(reinterpret_cast<const char*>(&buffer[kOffset]), size - kOffset);
        if (kBreakTime >= dmx::transmit::kBreakTimeMin)
        {
            Dmx::Get()->SetDmxBreakTime(kBreakTime);
            DEBUG_PRINTF("kBreakTime=%u", kBreakTime);
        }
    }

    /**
     * @brief Handles the "mab#" command to configure DMX MAB time.
     *
     * @param buffer Pointer to the packet buffer.
     * @param size Size of the packet buffer.
     */
    void HandleMab(const uint8_t* buffer, uint32_t size)
    {
        static constexpr auto kOffset = kCmdMabLength + 4;
        const auto kMabTime = Atoi(reinterpret_cast<const char*>(&buffer[kOffset]), size - kOffset);
        if (Validate(kMabTime, dmx::transmit::kMabTimeMin, dmx::transmit::kMabTimeMax))
        {
            Dmx::Get()->SetDmxMabTime(kMabTime);
            DEBUG_PRINTF("kMabTime=%u", kMabTime);
        }
    }

    /**
     * @brief Handles the "refresh#" command to configure DMX refresh rate.
     *
     * @param buffer Pointer to the packet buffer.
     * @param size Size of the packet buffer.
     */
    void HandleRefresh(const uint8_t* buffer, uint32_t size)
    {
        static constexpr auto kOffset = kCmdRefreshLength + 4;
        const auto kRefreshRate = Atoi(reinterpret_cast<const char*>(&buffer[kOffset]), size - kOffset);
        const auto kPeriodTime = (kRefreshRate != 0) ? 1000000U / kRefreshRate : 0;
        Dmx::Get()->SetDmxPeriodTime(kPeriodTime);
        DEBUG_PRINTF("kPeriodTime=%u", kPeriodTime);
    }

    /**
     * @brief Handles the "slots#" command to configure DMX slot count.
     *
     * @param buffer Pointer to the packet buffer.
     * @param size Size of the packet buffer.
     */
    void HandleSlots(const uint8_t* buffer, uint32_t size)
    {
        static constexpr auto kOffset = kCmdSlotsLength + 4;
        const auto kSlots = Atoi(reinterpret_cast<const char*>(&buffer[kOffset]), size - kOffset);
        if (Validate(kSlots, dmx::kChannelsMin, dmx::kChannelsMax))
        {
            Dmx::Get()->SetDmxSlots(static_cast<uint16_t>(kSlots));
            DEBUG_PRINTF("kSlots=%u", kSlots);
        }
    }

    /**
     * @brief Static callback function for receiving UDP packets.
     *
     * @param buffer Pointer to the packet buffer.
     * @param size Size of the packet buffer.
     * @param from_ip IP address of the sender.
     * @param from_port Port number of the sender.
     */
    void static StaticCallbackFunction(const uint8_t* buffer, uint32_t size, uint32_t from_ip, uint16_t from_port)
    {
        s_this->Input(buffer, size, from_ip, from_port);
    }

   private:
    int32_t handle_{-1};                ///< UDP handle for the network interface.
    static inline DmxConfigUdp* s_this; ///< Static instance pointer for the callback function.
};

#endif  // DMXCONFIGUDP_H_
