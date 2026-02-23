/**
 * @file e131controller.h
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

#ifndef E131CONTROLLER_H_
#define E131CONTROLLER_H_

#include <cstdint>

#include "e131.h"
#include "dmxnode.h"
#include "softwaretimers.h"

enum
{
    DEFAULT_SYNCHRONIZATION_ADDRESS = 5000
};

struct TE131ControllerState
{
    uint16_t nActiveUniverses;
    uint32_t DiscoveryTime;
    uint8_t priority;
    struct TSynchronizationPacket
    {
        uint16_t nUniverseNumber;
        uint32_t nIpAddress;
        uint8_t sequence_number;
    } SynchronizationPacket;
};

class E131Controller
{
   public:
    E131Controller();
    ~E131Controller();

    void Start();
    void Stop();

    void Print();

    void HandleDmxOut(uint16_t nUniverse, const uint8_t* pDmxData, uint32_t nLength);
    void HandleSync();
    void HandleBlackout();

    void SetSynchronizationAddress(uint16_t nSynchronizationAddress = DEFAULT_SYNCHRONIZATION_ADDRESS)
    {
        state_.SynchronizationPacket.nUniverseNumber = nSynchronizationAddress;
        state_.SynchronizationPacket.nIpAddress = e131::UniverseToMulticastIp(nSynchronizationAddress);
    }
    uint16_t GetSynchronizationAddress() const { return state_.SynchronizationPacket.nUniverseNumber; }

    void SetMaster(uint32_t nMaster = dmxnode::kDmxMaxValue)
    {
        if (nMaster < dmxnode::kDmxMaxValue)
        {
            master_ = nMaster;
        }
        else
        {
            master_ = dmxnode::kDmxMaxValue;
        }
    }
    uint32_t GetMaster() const { return master_; }

    const uint8_t* GetSoftwareVersion();

    void SetSourceName(const char* pSourceName);
    void SetPriority(uint8_t nPriority);

    static E131Controller* Get() { return s_this; }

   private:
    void FillDataPacket();
    void FillDiscoveryPacket();
    void FillSynchronizationPacket();
    uint8_t GetSequenceNumber(uint16_t nUniverse, uint32_t& nMulticastIpAddress);

    void SendDiscoveryPacket();

    void static StaticCallbackFunctionSendDiscoveryPacket([[maybe_unused]] TimerHandle_t timerHandle) { s_this->SendDiscoveryPacket(); }

   private:
    int32_t handle_{-1};
    struct TE131ControllerState state_;
    e131::DataPacket* m_pE131DataPacket{nullptr};
    e131::DiscoveryPacket* m_pE131DiscoveryPacket{nullptr};
    e131::SynchronizationPacket* m_pE131SynchronizationPacket{nullptr};
    uint32_t m_DiscoveryIpAddress{0};
    uint8_t cid_[e117::kCidLength];
    char source_name_[e131::kSourceNameLength];
    uint32_t master_{dmxnode::kDmxMaxValue};
    TimerHandle_t timer_handle_send_discovery_packet_{-1};

    static inline E131Controller* s_this;
};

#endif  // E131CONTROLLER_H_
