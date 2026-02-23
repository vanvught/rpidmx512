/**
 * @file l6470dmxmodes.cpp
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

#include <cstdint>
#include <cstdio>
#include <cassert>

#include "l6470.h"
#include "l6470dmxmodes.h"
#include "l6470dmxmode0.h"
#include "l6470dmxmode1.h"
#include "l6470dmxmode2.h"
#include "l6470dmxmode3.h"
#include "l6470dmxmode4.h"
#include "l6470dmxmode5.h"
#include "l6470dmxmode6.h"
#include "dmxnode.h"
 #include "firmware/debug/debug_debug.h"

L6470DmxModes::L6470DmxModes(TL6470DmxModes dmxmode, uint16_t dmx_start_address, L6470* l6470, uint32_t motor_index)
    : dmxmode_(dmxmode), dmx_start_address_(dmx_start_address)
{
    DEBUG_ENTRY();;

    assert(dmx_start_address <= dmxnode::kUniverseSize);
    assert(l6470 != nullptr);

    switch (dmxmode)
    {
        case L6470DMXMODE0:
            l6470_dmxmode_ = new L6470DmxMode0(l6470);
            dmx_footprint_ = L6470DmxMode0::GetDmxFootPrint();
            break;
        case L6470DMXMODE1:
            l6470_dmxmode_ = new L6470DmxMode1(l6470);
            dmx_footprint_ = L6470DmxMode1::GetDmxFootPrint();
            break;
        case L6470DMXMODE2:
            l6470_dmxmode_ = new L6470DmxMode2(l6470);
            dmx_footprint_ = L6470DmxMode2::GetDmxFootPrint();
            break;
        case L6470DMXMODE3:
            l6470_dmxmode_ = new L6470DmxMode3(l6470, motor_index);
            dmx_footprint_ = L6470DmxMode3::GetDmxFootPrint();
            break;
        case L6470DMXMODE4:
            l6470_dmxmode_ = new L6470DmxMode4(l6470, motor_index);
            dmx_footprint_ = L6470DmxMode4::GetDmxFootPrint();
            break;
        case L6470DMXMODE5:
            l6470_dmxmode_ = new L6470DmxMode5(l6470, motor_index);
            dmx_footprint_ = L6470DmxMode5::GetDmxFootPrint();
            break;
        case L6470DMXMODE6:
            l6470_dmxmode_ = new L6470DmxMode6(l6470, motor_index);
            dmx_footprint_ = L6470DmxMode6::GetDmxFootPrint();
            break;
        default:
            break;
    }

    assert(l6470_dmxmode_ != nullptr);

    if (l6470_dmxmode_ != nullptr)
    {
        motor_number_ = static_cast<uint8_t>(l6470->GetMotorNumber());
        assert(motor_number_ != motor_index);

        dmx_data_ = new uint8_t[dmx_footprint_];
        assert(dmx_data_ != nullptr);

        for (int i = 0; i < dmx_footprint_; i++)
        {
            dmx_data_[i] = 0;
        }
    }

    DEBUG_EXIT();;
}

L6470DmxModes::~L6470DmxModes()
{
    DEBUG_ENTRY();;

    delete[] dmx_data_;
    dmx_data_ = nullptr;

    delete l6470_dmxmode_;
    l6470_dmxmode_ = nullptr;

    DEBUG_EXIT();;
}

void L6470DmxModes::InitSwitch()
{
    DEBUG_ENTRY();;

    l6470_dmxmode_->InitSwitch();

    DEBUG_EXIT();
}

void L6470DmxModes::InitPos()
{
    DEBUG_ENTRY();;

    l6470_dmxmode_->InitPos();

    DEBUG_EXIT();
}

uint16_t L6470DmxModes::GetDmxFootPrintMode(uint32_t mode)
{
    switch (mode)
    {
        case L6470DMXMODE0:
            return L6470DmxMode0::GetDmxFootPrint();
            break;
        case L6470DMXMODE1:
            return L6470DmxMode1::GetDmxFootPrint();
            break;
        case L6470DMXMODE2:
            return L6470DmxMode2::GetDmxFootPrint();
            break;
        case L6470DMXMODE3:
            return L6470DmxMode3::GetDmxFootPrint();
            break;
        case L6470DMXMODE4:
            return L6470DmxMode4::GetDmxFootPrint();
            break;
        case L6470DMXMODE5:
            return L6470DmxMode5::GetDmxFootPrint();
            break;
        case L6470DMXMODE6:
            return L6470DmxMode6::GetDmxFootPrint();
            break;
        default:
            return 0;
            break;
    }
}

void L6470DmxModes::Start()
{
    DEBUG_ENTRY();;

    if (started_)
    {
        return;
    }

    l6470_dmxmode_->Start();

    started_ = true;

    DEBUG_EXIT();;
}

void L6470DmxModes::Stop()
{
    DEBUG_ENTRY();;

    if (!started_)
    {
        return;
    }

    l6470_dmxmode_->Stop();

    started_ = false;

    DEBUG_EXIT();;
}

void L6470DmxModes::HandleBusy()
{
    DEBUG_ENTRY();;

    l6470_dmxmode_->HandleBusy();

    DEBUG_EXIT();;
}

bool L6470DmxModes::BusyCheck()
{
    DEBUG_ENTRY();;

    DEBUG_EXIT();;
    return l6470_dmxmode_->BusyCheck();
}

bool L6470DmxModes::IsDmxDataChanged(const uint8_t* p)
{
    DEBUG_ENTRY();;

    auto is_changed = false;
    const auto kLastDmxChannel = static_cast<uint16_t>(dmx_start_address_ + dmx_footprint_ - 1);
    auto* q = dmx_data_;

#ifndef NDEBUG
    printf("\t\tDmxStartAddress = %d, LastDmxChannel = %d\n", dmx_start_address_, kLastDmxChannel);
#endif

    for (uint32_t i = dmx_start_address_; (i <= kLastDmxChannel) && (i <= 512U); i++)
    {
        if (*p != *q)
        {
            is_changed = true;
        }
        *q = *p;
        p++;
        q++;
    }

    DEBUG_EXIT();;
    return is_changed;
}

bool L6470DmxModes::IsDmxDataChanged(const uint8_t* dmx_data, uint32_t length)
{
    DEBUG_ENTRY();;

    assert(l6470_dmxmode_ != nullptr);
    assert(dmx_data != nullptr);

    if (l6470_dmxmode_ == nullptr)
    {
        DEBUG_EXIT();;
        return false;
    }

    if (length < (dmx_start_address_ + dmx_footprint_))
    {
        DEBUG_EXIT();;
        return false;
    }

    uint8_t* p = const_cast<uint8_t*>(dmx_data) + dmx_start_address_ - 1;

    return IsDmxDataChanged(p);
}

void L6470DmxModes::DmxData(const uint8_t* dmx_data, uint32_t length)
{
    DEBUG_ENTRY();;

    assert(l6470_dmxmode_ != nullptr);
    assert(dmx_data != nullptr);

    if (length < (dmx_start_address_ + dmx_footprint_))
    {
        return;
    }

    const auto* p = const_cast<uint8_t*>(dmx_data) + dmx_start_address_ - 1;

#ifndef NDEBUG
    printf("\tMotor : %d\n", motor_number_);

    for (uint32_t i = 0; i < dmx_footprint_; i++)
    {
        printf("\t\tDMX slot(%d) : %d\n", dmx_start_address_ + i, p[i]);
    }
#endif

    l6470_dmxmode_->Data(p);

    started_ = true;

    DEBUG_EXIT();;
}

void L6470DmxModes::Print()
{
    printf(" DMX: Mode=%d, StartAddress=%d, FootPrint=%d\n", static_cast<int>(dmxmode_), static_cast<int>(dmx_start_address_),
           static_cast<int>(dmx_footprint_));
}
