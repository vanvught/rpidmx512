/**
 * @file artnetcontroller.h
 *
 */
/* Copyright (C) 2017-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef ARTNETCONTROLLER_H_
#define ARTNETCONTROLLER_H_

#include <cstdint>

#include "artnet.h"
#include "artnettrigger.h"
#include "artnetpolltable.h"
#include "dmxnode.h"

struct State
{
    struct
    {
        uint32_t poll_reply_count;
        artnet::ArtPollQueue poll_reply_queue[4];
    } art;
    artnet::ReportCode reportcode;
    artnet::Status status;
};

struct TArtNetController
{
    uint8_t Oem[2];
};

class ArtNetController : public ArtNetPollTable
{
   public:
    ArtNetController();
    ~ArtNetController();

    void GetShortNameDefault(char* short_name);
    void SetShortName(const char* short_name);

    void GetLongNameDefault(char* long_name);
    void SetLongName(const char* long_name);

    void Start();
    void Stop();
    void Run()
    {
        if (m_bUnicast)
        {
            ProcessPoll();
        }
    }

    void Print();

    void HandleDmxOut(uint16_t nUniverse, const uint8_t* pDmxData, uint32_t nLength, uint8_t nPortIndex = 0);
    void HandleSync();
    void HandleBlackout();

    void SetRunTableCleanup(bool bDoTableCleanup) { m_bDoTableCleanup = bDoTableCleanup; }

    void SetSynchronization(bool bSynchronization) { m_bSynchronization = bSynchronization; }
    bool GetSynchronization() const { return m_bSynchronization; }

    void SetUnicast(bool bUnicast) { m_bUnicast = bUnicast; }
    bool GetUnicast() const { return m_bUnicast; }

    void SetForceBroadcast(bool bForceBroadcast) { m_bForceBroadcast = bForceBroadcast; }
    bool GetForceBroadcast() const { return m_bForceBroadcast; }

#ifdef CONFIG_ARTNET_CONTROLLER_ENABLE_MASTER
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
#endif

    const uint8_t* GetSoftwareVersion();

#if defined(ARTNET_HAVE_TRIGGER)
    void SetArtTriggerCallbackFunctionPtr(ArtTriggerCallbackFunctionPtr artTriggerCallbackFunctionPtr)
    {
        m_ArtTriggerCallbackFunctionPtr = artTriggerCallbackFunctionPtr;
    }
#endif

    void Input(const uint8_t* buffer, uint32_t size, uint32_t from_ip, uint16_t from_port);
    void static StaticCallbackFunction(const uint8_t* buffer, uint32_t size, uint32_t from_ip, uint16_t from_port)
    {
        s_this->Input(buffer, size, from_ip, from_port);
    }

    static ArtNetController* Get() { return s_this; }

   private:
    void ProcessPoll();
    void HandlePoll(const uint8_t* buffer, uint32_t from_ip);
    void HandlePollReply(const uint8_t* buffer, uint32_t from_ip);
    void HandleTrigger();
    void ActiveUniversesAdd(uint16_t nUniverse);
    void ActiveUniversesClear();

   private:
    TArtNetController m_ArtNetController;
    State state_;

    artnet::ArtPoll m_ArtNetPoll;
    artnet::ArtPollReply art_poll_reply_;

    struct TArtNetPacket
    {
        union artnet::UArtPacket ArtPacket;
        uint32_t IPAddressFrom;
        uint16_t nLength;
        artnet::OpCodes OpCode;
    };
    TArtNetPacket* m_pArtNetPacket;

    artnet::ArtDmx* m_pArtDmx;
    artnet::ArtSync* m_pArtSync;

    ArtTriggerCallbackFunctionPtr m_ArtTriggerCallbackFunctionPtr{nullptr};

    bool m_bSynchronization{true};
    bool m_bUnicast{true};
    bool m_bForceBroadcast{false};
    bool m_bDoTableCleanup{true};
    bool m_bDmxHandled{false};

    int32_t handle_{-1};
    uint32_t m_nLastPollMillis{0};
    uint32_t m_nActiveUniverses{0};
#ifdef CONFIG_ARTNET_CONTROLLER_ENABLE_MASTER
    uint32_t master_{dmxnode::kDmxMaxValue};
#endif
    static inline ArtNetController* s_this;
};

#endif  // ARTNETCONTROLLER_H_
