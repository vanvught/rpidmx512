/**
 * @file sparkfundmx.cpp
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

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cassert>

#include "sparkfundmx.h"
#include "configstore.h"
#include "dmxnode.h"
#include "sparkfundmx_internal.h"
#include "json/sparkfundmxparams.h"
#include "json/modeparams.h"
#include "json/motorparams.h"
#include "json/l6470params.h"
#include "l6470dmxmodes.h"
#include "modestore.h"
#include "hal.h"
#include "hal_api.h"
#include "hal_spi.h"
#include "hal_gpio.h"
#include "hal_millis.h"

 #include "firmware/debug/debug_debug.h"

SparkFunDmx::SparkFunDmx() : dmx_start_address_(dmxnode::kAddressInvalid)
{
    DEBUG_ENTRY();;

    m_nGlobalSpiCs = SPI_CS0;
    m_nGlobalResetPin = GPIO_RESET_OUT;
    m_nGlobalBusyPin = GPIO_BUSY_IN;

    m_bIsGlobalSpiCsSet = false;
    m_bIsGlobalResetSet = false;
    m_bIsGlobalBusyPinSet = false;

    m_nLocalPosition = 0;
    m_nLocalSpiCs = SPI_CS0;
    m_nLocalResetPin = GPIO_RESET_OUT;
    m_nLocalBusyPin = GPIO_BUSY_IN;

    m_bIsLocalPositionSet = false;
    m_bIsLocalSpiCsSet = false;
    m_bIsLocalResetSet = false;
    m_bIsLocalBusyPinSet = false;

    for (uint32_t i = 0; i < common::store::l6470dmx::kMaxMotors; i++)
    {
        autodriver_[i] = nullptr;
        l6470dmx_modes_[i] = nullptr;
        slotinfo_[i] = nullptr;
    }

    DEBUG_EXIT();;
}

SparkFunDmx::~SparkFunDmx()
{
    DEBUG_ENTRY();;

    for (uint32_t i = 0; i < common::store::l6470dmx::kMaxMotors; i++)
    {
        if (autodriver_[i] != nullptr)
        {
            delete autodriver_[i];
            autodriver_[i] = nullptr;
        }

        if (l6470dmx_modes_[i] != nullptr)
        {
            delete l6470dmx_modes_[i];
            l6470dmx_modes_[i] = nullptr;
        }

        if (slotinfo_[i] != nullptr)
        {
            delete slotinfo_[i];
            slotinfo_[i] = nullptr;
        }
    }

    DEBUG_EXIT();;
}

void SparkFunDmx::Start([[maybe_unused]] uint32_t port_index)
{
    DEBUG_ENTRY();;

    for (uint32_t i = 0; i < common::store::l6470dmx::kMaxMotors; i++)
    {
        if (l6470dmx_modes_[i] != nullptr)
        {
            l6470dmx_modes_[i]->Start();
        }
    }

    DEBUG_EXIT();;
}

void SparkFunDmx::Stop([[maybe_unused]] uint32_t port_index)
{
    DEBUG_ENTRY();;

    for (uint32_t i = 0; i < common::store::l6470dmx::kMaxMotors; i++)
    {
        if (l6470dmx_modes_[i] != nullptr)
        {
            l6470dmx_modes_[i]->Stop();
        }
    }

    DEBUG_EXIT();;
}

void SparkFunDmx::ReadConfigFiles()
{
    DEBUG_ENTRY();;
#if !defined(H3)
    m_bIsGlobalSpiCsSet = false;
#else
    m_bIsGlobalSpiCsSet = true;
    m_nGlobalSpiCs = SPI_CS0;
#endif
    m_bIsGlobalResetSet = false;
    m_bIsGlobalBusyPinSet = false;

    json::SparkFunDmxParams sparkfun_dmxparams;
    sparkfun_dmxparams.Load();
    sparkfun_dmxparams.Set(this);

    if (m_bIsGlobalBusyPinSet)
    {
        FUNC_PREFIX(GpioFsel(m_nGlobalBusyPin, GPIO_FSEL_INPUT));
    }

    FUNC_PREFIX(GpioFsel(m_nGlobalResetPin, GPIO_FSEL_OUTPUT));
    FUNC_PREFIX(GpioSet(m_nGlobalResetPin));

    FUNC_PREFIX(GpioClr(m_nGlobalResetPin));
    udelay(10000);
    FUNC_PREFIX(GpioSet(m_nGlobalResetPin));
    udelay(10000);

    FUNC_PREFIX(SpiBegin());

    for (uint32_t i = 0; i < common::store::l6470dmx::kMaxMotors; i++)
    {
        printf("SparkFun motor%d.txt:\n", i);

        m_bIsLocalPositionSet = false;
        m_bIsLocalSpiCsSet = false;
        m_bIsLocalResetSet = false;
        m_bIsLocalBusyPinSet = false;

        json::SparkFunDmxParams sparkfun_dmxparams_index(i);
        sparkfun_dmxparams_index.Load();
        sparkfun_dmxparams_index.Set(this);

        if ((m_bIsLocalPositionSet) && (m_nLocalPosition < common::store::l6470dmx::kMaxMotors))
        {
            const uint8_t kSpiCs = m_bIsLocalSpiCsSet ? m_nLocalSpiCs : m_nGlobalSpiCs;
            const uint8_t kResetPin = m_bIsLocalResetSet ? m_nLocalResetPin : m_nGlobalResetPin;
            const uint8_t kBusyPin = m_bIsLocalBusyPinSet ? m_nLocalBusyPin : m_nGlobalBusyPin;

            printf("nSpiCs=%d [m_bIsLocalSpiCsSet=%d], nResetPin=%d [m_bIsLocalResetSet=%d], nBusyPin=%d [m_bIsLocalBusyPinSet=%d, m_bIsGlobalBusyPinSet=%d]\n",
                   static_cast<int>(kSpiCs), static_cast<int>(m_bIsLocalSpiCsSet), static_cast<int>(kResetPin), static_cast<int>(m_bIsLocalResetSet),
                   static_cast<int>(kBusyPin), static_cast<int>(m_bIsLocalBusyPinSet), static_cast<int>(m_bIsGlobalBusyPinSet));

            if (m_bIsGlobalBusyPinSet || m_bIsLocalBusyPinSet)
            {
                autodriver_[i] = new AutoDriver(m_nLocalPosition, kSpiCs, kResetPin, kBusyPin);
            }
            else
            {
                autodriver_[i] = new AutoDriver(m_nLocalPosition, kSpiCs, kResetPin);
            }
        }
        else
        {
            printf("Local position is not set\n");
        }
    }

    printf("NumBoards : %d\n", static_cast<int>(AutoDriver::getNumBoards()));

    for (uint32_t i = 0; i < common::store::l6470dmx::kMaxMotors; i++)
    {
#ifndef NDEBUG
        printf("motor%d.txt:\n", i);
#endif
        if (autodriver_[i] != nullptr)
        {
            json::ModeParams mode_params(i);
            mode_params.Load();

            const auto kDmxMode = ConfigStore::Instance().DmxL6470GetModeIndexed(i, &common::store::l6470dmx::Mode::dmx_mode);
            const auto kDmxStartAddressMode = ConfigStore::Instance().DmxL6470GetModeIndexed(i, &common::store::l6470dmx::Mode::dmx_start_address);

#ifndef NDEBUG
            printf("\t-----------------------------\n");
            printf("\tkDmxMode=%d (DMX footprint=%d)\n", kDmxMode, L6470DmxModes::GetDmxFootPrintMode(kDmxMode));
            printf("\kDmxStartAddressMode=%d\n", kDmxStartAddressMode);
            printf("\t=============================\n");
#endif

            if ((kDmxStartAddressMode <= dmxnode::kUniverseSize) && (L6470DmxModes::GetDmxFootPrintMode(kDmxMode) != 0))
            {
                if (autodriver_[i]->IsConnected())
                {
                    printf("Motor %d is connected\n", i);

                    autodriver_[i]->setMotorNumber(i);
                    autodriver_[i]->Dump();

                    json::MotorParams motor_params(i);
                    motor_params.Load();
                    motor_params.Set(autodriver_[i]);

                    json::L6470Params l6470_params(i);
                    l6470_params.Load();
                    l6470_params.Set(autodriver_[i]);

                    autodriver_[i]->Dump();

                    l6470dmx_modes_[i] = new L6470DmxModes(static_cast<TL6470DmxModes>(kDmxMode), kDmxStartAddressMode, autodriver_[i], i);
                    assert(l6470dmx_modes_[i] != nullptr);

                    if (dmx_start_address_ == dmxnode::kAddressInvalid)
                    {
                        dmx_start_address_ = l6470dmx_modes_[i]->GetDmxStartAddress();
                        dmx_footprint_ = l6470dmx_modes_[i]->GetDmxFootPrint();
                    }
                    else
                    {
                        const uint16_t kDmxChannelLastCurrent = dmx_start_address_ + dmx_footprint_;
                        dmx_start_address_ = std::min(dmx_start_address_, l6470dmx_modes_[i]->GetDmxStartAddress());

                        const uint16_t kDmxChannelLastNew = kDmxStartAddressMode + l6470dmx_modes_[i]->GetDmxFootPrint();
                        dmx_footprint_ = std::max(kDmxChannelLastCurrent, kDmxChannelLastNew) - dmx_start_address_;
                    }
#ifndef NDEBUG
                    printf("DMX Mode: %d, DMX Start Address: %d\n", l6470dmx_modes_[i]->GetMode(), l6470dmx_modes_[i]->GetDmxStartAddress());
                    printf("DMX Start Address:%d, DMX Footprint:%d\n", static_cast<int>(dmx_start_address_), static_cast<int>(dmx_footprint_));
#endif
                    const uint32_t kMaxSlots = std::min(common::store::l6470dmx::mode::kMaxDmxFootprint, l6470dmx_modes_[i]->GetDmxFootPrint());
#ifndef NDEBUG
                    printf("SlotInfo slots: %d\n", static_cast<int>(kMaxSlots));
#endif
                    slotinfo_[i] = new dmxnode::SlotInfo[kMaxSlots];
                    assert(slotinfo_[i] != nullptr);

                    for (uint32_t j = 0; j < kMaxSlots; j++)
                    {
                        //                       m_pModeParams[i]->GetSlotInfo(j, slotinfo_[i][j]);
#ifndef NDEBUG
                        printf(" Slot:%d %2x:%4x\n", j, slotinfo_[i][j].type, slotinfo_[i][j].category);
#endif
                    }
                }
                else
                {
                    delete autodriver_[i];
                    autodriver_[i] = nullptr;
                    printf("Motor %d - Communication issues! Check SPI configuration and cables\n", i);
                }
            }
            else
            {
                delete autodriver_[i];
                autodriver_[i] = nullptr;
                printf("Motor %d - Configuration error! Check Mode parameters\n", i);
            }
#ifndef NDEBUG
            printf("Motor %d --------- end ---------\n", i);
#endif
        }
        else
        {
            printf("Skipping Motor %d\n", i);
        }
    }

    printf("InitSwitch()\n");
    for (uint32_t i = 0; i < common::store::l6470dmx::kMaxMotors; i++)
    {
        if (l6470dmx_modes_[i] != nullptr)
        {
            printf(" Motor %d\n", i);
            l6470dmx_modes_[i]->InitSwitch();
        }
    }

    printf("busyCheck()\n");
    for (uint32_t i = 0; i < common::store::l6470dmx::kMaxMotors; i++)
    {
        if (autodriver_[i] != nullptr)
        {
            printf(" Motor %d\n", i);
            const uint32_t kMillis = hal::Millis();
            while (autodriver_[i]->busyCheck())
            {
                if ((hal::Millis() - kMillis) > 1000)
                {
                    printf("  Time-out!\n");
                    break;
                }
            }
        }
    }

    printf("InitPos()\n");
    for (uint32_t i = 0; i < common::store::l6470dmx::kMaxMotors; i++)
    {
        if (l6470dmx_modes_[i] != nullptr)
        {
            printf(" Motor %d\n", i);
            l6470dmx_modes_[i]->InitPos();
        }
    }

    DEBUG_EXIT();;
}

template <bool doUpdate> void SparkFunDmx::SetData(uint32_t port_index, const uint8_t* pData, uint32_t length)
{
    // delegate to single function
    SetDataImpl(port_index, pData, length);
}

void SparkFunDmx::SetDataImpl([[maybe_unused]] uint32_t port_index, const uint8_t* pData, uint32_t length)
{
    DEBUG_ENTRY();;
    assert(pData != nullptr);
    assert(length <= dmxnode::kUniverseSize);

    bool bIsDmxDataChanged[common::store::l6470dmx::kMaxMotors];

    for (uint32_t i = 0; i < common::store::l6470dmx::kMaxMotors; i++)
    {
        if (l6470dmx_modes_[i] != nullptr)
        {
            bIsDmxDataChanged[i] = l6470dmx_modes_[i]->IsDmxDataChanged(pData, length);

            if (bIsDmxDataChanged[i])
            {
                l6470dmx_modes_[i]->HandleBusy();
            }
        }
        else
        {
            bIsDmxDataChanged[i] = false;
        }
#ifndef NDEBUG
        printf("bIsDmxDataChanged[%d]=%d\n", i, bIsDmxDataChanged[i]);
#endif
    }

    for (uint32_t i = 0; i < common::store::l6470dmx::kMaxMotors; i++)
    {
        if (bIsDmxDataChanged[i])
        {
            while (l6470dmx_modes_[i]->BusyCheck())
            {
            }
        }
    }

    for (uint32_t i = 0; i < common::store::l6470dmx::kMaxMotors; i++)
    {
        if (bIsDmxDataChanged[i])
        {
            l6470dmx_modes_[i]->DmxData(pData, length);
        }
    }

    DEBUG_EXIT();;
}

void SparkFunDmx::Sync([[maybe_unused]] uint32_t const port_index)
{
    // TODO (a) Implement Sync
}

void SparkFunDmx::Sync()
{
    // TODO (a) Implement Sync
}

bool SparkFunDmx::SetDmxStartAddress(uint16_t dmx_start_address)
{
    DEBUG_ENTRY();;

    if (dmx_start_address == 0)
    {
        return false;
    }

    if (dmx_start_address == dmx_start_address_)
    {
        return true;
    }

    for (uint32_t i = 0; i < common::store::l6470dmx::kMaxMotors; i++)
    {
        if (l6470dmx_modes_[i] != nullptr)
        {
            const auto kCurrentDmxStartAddress = l6470dmx_modes_[i]->GetDmxStartAddress();
            const auto kNewDmxStartAddress = static_cast<uint16_t>((kCurrentDmxStartAddress - dmx_start_address_) + dmx_start_address);
#ifndef NDEBUG
            printf("\tMotor=%d, Current DMX Start Address=%d, New DMX Start Address=%d\n", i, kCurrentDmxStartAddress, kNewDmxStartAddress);
#endif
            l6470dmx_modes_[i]->SetDmxStartAddress(kNewDmxStartAddress);
            l6470mode_store::SaveDmxStartAddress(i, kNewDmxStartAddress);
        }
    }

    dmx_start_address_ = dmx_start_address;

    DEBUG_EXIT();;
    return true;
}

bool SparkFunDmx::GetSlotInfo(uint16_t slot_offset, dmxnode::SlotInfo& slot_info)
{
    DEBUG_ENTRY();;

    if (slot_offset > dmx_footprint_)
    {
        DEBUG_EXIT();
        return false;
    }

    const uint16_t kDmxAddress = dmx_start_address_ + slot_offset;

    for (uint32_t i = 0; i < common::store::l6470dmx::kMaxMotors; i++)
    {
        if ((l6470dmx_modes_[i] != nullptr) && (slotinfo_[i] != nullptr))
        {
            const auto kOffset = static_cast<int16_t>(kDmxAddress - l6470dmx_modes_[i]->GetDmxStartAddress());

            if ((kDmxAddress >= l6470dmx_modes_[i]->GetDmxStartAddress()) && (kOffset < l6470dmx_modes_[i]->GetDmxFootPrint()))
            {
                slot_info.type = slotinfo_[i][kOffset].type;
                slot_info.category = slotinfo_[i][kOffset].category;

                DEBUG_EXIT();
                return true;
            }
        }
    }

    return false;
}

void SparkFunDmx::Print()
{
    for (uint32_t i = 0; i < common::store::l6470dmx::kMaxMotors; i++)
    {
        if (autodriver_[i] != nullptr)
        {
            autodriver_[i]->Print();

            if (l6470dmx_modes_[i] != nullptr)
            {
                l6470dmx_modes_[i]->Print();

                printf(" SlotInfo: ");
                for (uint32_t j = 0; j < l6470dmx_modes_[i]->GetDmxFootPrint(); j++)
                {
                    printf("%.2x:%.4x ", slotinfo_[i][j].type, slotinfo_[i][j].category);
                }
                puts("");
            }
        }
    }
}

// Explicit template instantiations
template void SparkFunDmx::SetData<true>(uint32_t, const uint8_t*, uint32_t);
template void SparkFunDmx::SetData<false>(uint32_t, const uint8_t*, uint32_t);
