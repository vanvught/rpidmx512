/**
 * @file dmx.cpp
 *
 */
/* Copyright (C) 2018-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if __GNUC__ > 8
#pragma GCC target("general-regs-only")
#endif

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <cassert>

#include "dmx.h"
#include "rdm.h"
#include "rdm_e120.h"

#include "arm/arm.h"
#include "arm/synchronize.h"
#include "arm/gic.h"
#include "irq_timer.h"

#include "h3_gpio.h"
#include "h3_uart.h"
#include "h3_ccu.h"
#include "h3_timer.h"
#include "h3_hs_timer.h"
#include "h3_board.h"

#include "firmware/debug/debug_debug.h"

#define GPIO_ANALYZER_CH1 GPIO_EXT_26
#define GPIO_ANALYZER_CH2 GPIO_EXT_24
#define GPIO_ANALYZER_CH3 GPIO_EXT_22
#define GPIO_ANALYZER_CH4 GPIO_EXT_18
#define GPIO_ANALYZER_CH5 GPIO_EXT_16

namespace console {
void Error(const char*);
} // namespace console

#if (EXT_UART_NUMBER == 1)
#define UART_IRQN H3_UART1_IRQn
#elif (EXT_UART_NUMBER == 3)
#define UART_IRQN H3_UART3_IRQn
#else
#error Unsupported UART device configured
#endif

#ifndef ALIGNED
#define ALIGNED __attribute__((aligned(4)))
#endif

typedef enum { IDLE = 0, PRE_BREAK, BREAK, MAB, DMXDATA, RDMDATA, CHECKSUMH, CHECKSUML, RDMDISC, DMXINTER } _dmx_state;

namespace dmx {
enum class PortState { IDLE, TX, RX };
} // namespace dmx

using namespace dmx;

static PortDirection s_nPortDirection = dmx::PortDirection::kInput;
static volatile PortState sv_PortState;
static OutputStyle s_OutputStyle;

static volatile dmx::TotalStatistics sv_TotalStatistics[dmx::config::max::kPorts] ALIGNED;

// DMX

static struct Data s_DmxData ALIGNED;
static uint8_t s_DmxDataPrevious[buffer::kSize] ALIGNED;
static volatile _dmx_state sv_DmxReceiveState = IDLE;
static volatile uint32_t sv_nDmxDataIndex;

static uint32_t s_DmxTransmitBreakTimeIntv;
static uint32_t s_DmxTransmitMabTimeIntv;
static uint32_t s_DmxTransmitPeriodIntv;

static uint32_t s_nDmxSendDataLength = (dmx::kChannelsMax + 1); ///< SC + UNIVERSE SIZE
static volatile uint32_t sv_nFiqMicrosCurrent;
static volatile uint32_t sv_nFiqMicrosPrevious;
static volatile bool sv_isDmxPreviousBreak = false;
static volatile uint32_t sv_DmxBreakToBreakLatest;
static volatile uint32_t sv_DmxBreakToBreakPrevious;
static volatile uint32_t sv_DmxSlotsInPacketPrevious;
static volatile _dmx_state sv_DmxTransmitState = IDLE;
static volatile uint32_t sv_DmxTransmitCurrentSlot;

static volatile uint32_t sv_nDmxUpdatesPerSecond;
static volatile uint32_t sv_nDmxPacketsPrevious;

// RDM

static volatile uint32_t sv_nRdmDataBufferIndexHead;
static volatile uint32_t sv_nRdmDataBufferIndexTail;
static uint8_t s_RdmData[RDM_DATA_BUFFER_INDEX_ENTRIES][RDM_DATA_BUFFER_SIZE] ALIGNED;
static volatile uint32_t sv_nRdmDiscSlotToSlot[RDM_DATA_BUFFER_INDEX_ENTRIES];
static volatile uint16_t sv_nRdmChecksum; ///< This must be uint16_t
volatile uint32_t gsv_RdmDataReceiveEnd;

/**
 * Timer 0 interrupt DMX Receiver
 * Slot time-out
 */
static void IrqTimer0DmxReceive(uint32_t clo) {
    H3GpioSet(GPIO_ANALYZER_CH4);

    __DMB();
    if (sv_DmxReceiveState == DMXDATA) {
        if (H3_TIMER->AVS_CNT1 - sv_nFiqMicrosCurrent > s_DmxData.Statistics.nSlotToSlot) {
            __DMB();
            sv_DmxReceiveState = IDLE;
            s_DmxData.Statistics.nSlotsInPacket |= 0x8000;
        } else {
            H3_TIMER->TMR0_INTV = s_DmxData.Statistics.nSlotToSlot * 12;
            H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD); // 0x3;
        }
    } else if (sv_DmxReceiveState == RDMDISC) {
        if (clo - sv_nFiqMicrosCurrent > sv_nRdmDiscSlotToSlot[sv_nRdmDataBufferIndexHead]) {
            __DMB();
            sv_nRdmDataBufferIndexHead = (sv_nRdmDataBufferIndexHead + 1) & RDM_DATA_BUFFER_INDEX_MASK;
            sv_DmxReceiveState = IDLE;
            gsv_RdmDataReceiveEnd = H3_HS_TIMER->CURNT_LO;
            H3GpioClr(GPIO_ANALYZER_CH3);
        } else {
            H3_TIMER->TMR0_INTV = sv_nRdmDiscSlotToSlot[sv_nRdmDataBufferIndexHead] * 12;
            H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD); // 0x3;
        }
    }

    H3GpioClr(GPIO_ANALYZER_CH4);
}

/**
 * Timer 0 interrupt DMX Sender
 */
static void IrqTimer0DmxSender([[maybe_unused]] uint32_t clo) {
    switch (sv_DmxTransmitState) {
        case DMXINTER:
            H3GpioSet(GPIO_ANALYZER_CH2);

            H3_TIMER->TMR0_INTV = s_DmxTransmitBreakTimeIntv;
            H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);

            EXT_UART->LCR = UART_LCR_8_N_2 | UART_LCR_BC;
            __DMB();
            sv_DmxTransmitState = BREAK;

            H3GpioClr(GPIO_ANALYZER_CH2);
            break;
        case BREAK:
            H3GpioSet(GPIO_ANALYZER_CH3);

            H3_TIMER->TMR0_INTV = s_DmxTransmitMabTimeIntv;
            H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);
            EXT_UART->LCR = UART_LCR_8_N_2;
            __DMB();
            sv_DmxTransmitState = MAB;

            H3GpioClr(GPIO_ANALYZER_CH3);
            break;
        case MAB: {
            H3GpioSet(GPIO_ANALYZER_CH4);

            uint32_t fifo_cnt = 16;

            for (sv_DmxTransmitCurrentSlot = 0; fifo_cnt-- > 0; sv_DmxTransmitCurrentSlot++) {
                if (sv_DmxTransmitCurrentSlot >= s_nDmxSendDataLength) {
                    break;
                }

                EXT_UART->O00.THR = s_DmxData.Data[sv_DmxTransmitCurrentSlot];
            }

            if (sv_DmxTransmitCurrentSlot < s_nDmxSendDataLength) {
                __DMB();
                sv_DmxTransmitState = DMXDATA;
                EXT_UART->O04.IER = UART_IER_ETBEI;
            } else {
                if (s_OutputStyle == dmx::OutputStyle::kDelta) {
                    EXT_UART->O04.IER &= ~UART_IER_ETBEI;
                    __DMB();
                    sv_DmxTransmitState = IDLE;
                } else {
                    __DMB();
                    sv_DmxTransmitState = DMXINTER;
                }
            }

            H3GpioClr(GPIO_ANALYZER_CH4);
        } break;
        case DMXDATA:
            printf("Output period too short (%d, slot %d)\n", s_nDmxSendDataLength, sv_DmxTransmitCurrentSlot);
            //		assert(0);
            break;
        default:
            break;
    }
}

/**
 * Timer 1 interrupt DMX Receiver
 * Statistics
 */
static void IrqTimer1DmxReceive([[maybe_unused]] uint32_t clo) {
    __DMB();
    sv_nDmxUpdatesPerSecond = sv_TotalStatistics[0].dmx.received - sv_nDmxPacketsPrevious;
    sv_nDmxPacketsPrevious = sv_TotalStatistics[0].dmx.received;
}

/**
 * Interrupt handler for continues receiving DMX512 data.
 */
static void FiqDmxInHandler() {
    sv_nFiqMicrosCurrent = H3_TIMER->AVS_CNT1;

    if (EXT_UART->LSR & UART_LSR_BI) {
        sv_DmxReceiveState = PRE_BREAK;
        sv_DmxBreakToBreakLatest = sv_nFiqMicrosCurrent;
    } else if (EXT_UART->O08.IIR & UART_IIR_IID_RD) {
        const auto kData = static_cast<uint8_t>(EXT_UART->O00.RBR);

        switch (sv_DmxReceiveState) {
            case IDLE:
                sv_DmxReceiveState = RDMDISC;
                s_RdmData[sv_nRdmDataBufferIndexHead][0] = kData;
                sv_nDmxDataIndex = 1;
                break;
            case PRE_BREAK:
                sv_DmxReceiveState = BREAK;
                break;
            case BREAK:
                switch (kData) {
                    case kStartCode:
                        sv_DmxReceiveState = DMXDATA;
                        s_DmxData.Data[0] = kStartCode;
                        s_DmxData.Statistics.nSlotsInPacket = 1;
                        sv_TotalStatistics[0].dmx.received = sv_TotalStatistics[0].dmx.received + 1;

                        if (sv_isDmxPreviousBreak) {
                            s_DmxData.Statistics.nBreakToBreak = sv_DmxBreakToBreakLatest - sv_DmxBreakToBreakPrevious;
                            sv_DmxBreakToBreakPrevious = sv_DmxBreakToBreakLatest;
                        } else {
                            sv_isDmxPreviousBreak = true;
                            sv_DmxBreakToBreakPrevious = sv_DmxBreakToBreakLatest;
                        }
                        break;
                    case E120_SC_RDM:
                        sv_DmxReceiveState = RDMDATA;
                        s_RdmData[sv_nRdmDataBufferIndexHead][0] = E120_SC_RDM;
                        sv_nRdmChecksum = E120_SC_RDM;
                        sv_nDmxDataIndex = 1;
                        sv_isDmxPreviousBreak = false;
                        break;
                    default:
                        sv_DmxReceiveState = IDLE;
                        sv_isDmxPreviousBreak = false;
                        break;
                }
                break;
            case DMXDATA: {
                s_DmxData.Statistics.nSlotToSlot = sv_nFiqMicrosCurrent - sv_nFiqMicrosPrevious;
                H3_TIMER->TMR0_INTV = (s_DmxData.Statistics.nSlotToSlot + 12) * 12;
                H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD); // 0x3;

                auto index = s_DmxData.Statistics.nSlotsInPacket;
                s_DmxData.Data[index] = kData;
                index++;
                s_DmxData.Statistics.nSlotsInPacket = index;

                if (index > dmx::kChannelsMax) {
                    index |= 0x8000;
                    s_DmxData.Statistics.nSlotsInPacket = index;
                    sv_DmxReceiveState = IDLE;
                    __DMB();
                }
            } break;
            case RDMDATA:
                if (sv_nDmxDataIndex > RDM_DATA_BUFFER_SIZE) {
                    sv_DmxReceiveState = IDLE;
                } else {
                    s_RdmData[sv_nRdmDataBufferIndexHead][sv_nDmxDataIndex++] = kData;
                    sv_nRdmChecksum = static_cast<uint16_t>(sv_nRdmChecksum + kData);

                    const auto* p = reinterpret_cast<struct TRdmMessage*>(&s_RdmData[sv_nRdmDataBufferIndexHead][0]);
                    if (sv_nDmxDataIndex == p->message_length) {
                        sv_DmxReceiveState = CHECKSUMH;
                    }
                }
                break;
            case CHECKSUMH:
                s_RdmData[sv_nRdmDataBufferIndexHead][sv_nDmxDataIndex++] = kData;
                sv_nRdmChecksum = static_cast<uint16_t>(sv_nRdmChecksum - static_cast<uint16_t>(kData << 8));
                sv_DmxReceiveState = CHECKSUML;
                break;
            case CHECKSUML: {
                s_RdmData[sv_nRdmDataBufferIndexHead][sv_nDmxDataIndex++] = kData;
                sv_nRdmChecksum = static_cast<uint16_t>(sv_nRdmChecksum - kData);
                const auto* p = reinterpret_cast<struct TRdmMessage*>(&s_RdmData[sv_nRdmDataBufferIndexHead][0]);

                if ((sv_nRdmChecksum == 0) && (p->sub_start_code == E120_SC_SUB_MESSAGE)) {
                    sv_nRdmDataBufferIndexHead = (sv_nRdmDataBufferIndexHead + 1) & RDM_DATA_BUFFER_INDEX_MASK;
                    gsv_RdmDataReceiveEnd = H3_HS_TIMER->CURNT_LO;
                    sv_TotalStatistics[0].rdm.received.good = sv_TotalStatistics[0].rdm.received.good + 1;
                    __DMB();
                } else {
                    sv_TotalStatistics[0].rdm.received.bad = sv_TotalStatistics[0].rdm.received.bad + 1;
                }

                sv_DmxReceiveState = IDLE;
            } break;
            case RDMDISC:
                sv_nRdmDiscSlotToSlot[sv_nRdmDataBufferIndexHead] = sv_nFiqMicrosCurrent - sv_nFiqMicrosPrevious;
                s_RdmData[sv_nRdmDataBufferIndexHead][sv_nDmxDataIndex++] = kData;

                H3_TIMER->TMR0_INTV = (sv_nRdmDiscSlotToSlot[sv_nRdmDataBufferIndexHead] + 12) * 12;
                H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD); // 0x3

                if (sv_nDmxDataIndex == 24) {
                    sv_nRdmDataBufferIndexHead = (sv_nRdmDataBufferIndexHead + 1) & RDM_DATA_BUFFER_INDEX_MASK;
                    sv_DmxReceiveState = IDLE;
                    gsv_RdmDataReceiveEnd = H3_HS_TIMER->CURNT_LO;
                    __DMB();
                }
                break;
            default:
                sv_DmxReceiveState = IDLE;
                sv_isDmxPreviousBreak = false;
                break;
        }
    }

    sv_nFiqMicrosPrevious = sv_nFiqMicrosCurrent;
}

/**
 * EXT_UART TX interrupt
 */
static void FiqDmxOutHandler() {
    uint32_t fifo_cnt = 16;

    for (; fifo_cnt-- > 0; sv_DmxTransmitCurrentSlot++) {
        if (sv_DmxTransmitCurrentSlot >= s_nDmxSendDataLength) {
            break;
        }

        EXT_UART->O00.THR = s_DmxData.Data[sv_DmxTransmitCurrentSlot];
    }

    if (sv_DmxTransmitCurrentSlot >= s_nDmxSendDataLength) {
        EXT_UART->O04.IER &= ~UART_IER_ETBEI;
        __DMB();
        if (s_OutputStyle == dmx::OutputStyle::kDelta) {
            sv_DmxTransmitState = IDLE;
        } else {
            H3_TIMER->TMR0_INTV = s_DmxTransmitPeriodIntv;
            H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD); // 0x3;
            sv_DmxTransmitState = DMXINTER;
        }
    }
}

static void __attribute__((interrupt("FIQ"))) FiqDmx() {
    __DMB();

    if (gic_get_active_fiq() == UART_IRQN) {
        const uint32_t iir = EXT_UART->O08.IIR;

        if (s_nPortDirection == PortDirection::kInput) {
            FiqDmxInHandler();
        } else {
            if ((iir & 0xF) == UART_IIR_IID_THRE) {
                FiqDmxOutHandler();
            } else {
                (void)EXT_UART->USR;
            }
        }

        H3_GIC_CPUIF->EOI = UART_IRQN;
        gic_unpend<UART_IRQN>();
    } else {
        console::Error("spurious interrupt\n");
    }

    __DMB();
}

static void uart_dmx_config() {
#if (EXT_UART_NUMBER == 1)
    uint32_t value = H3_PIO_PORTG->CFG0;
    // PG6, TX
    value &= static_cast<uint32_t>(~(GPIO_SELECT_MASK << PG6_SELECT_CFG0_SHIFT));
    value |= H3_PG6_SELECT_UART1_TX << PG6_SELECT_CFG0_SHIFT;
    // PG7, RX
    value &= static_cast<uint32_t>(~(GPIO_SELECT_MASK << PG7_SELECT_CFG0_SHIFT));
    value |= H3_PG7_SELECT_UART1_RX << PG7_SELECT_CFG0_SHIFT;
    H3_PIO_PORTG->CFG0 = value;

    H3_CCU->BUS_CLK_GATING3 |= CCU_BUS_CLK_GATING3_UART1;
    H3_CCU->BUS_SOFT_RESET4 |= CCU_BUS_SOFT_RESET4_UART1;
#elif (EXT_UART_NUMBER == 3)
    uint32_t value = H3_PIO_PORTA->CFG1;
    // PA13, TX
    value &= static_cast<uint32_t>(~(GPIO_SELECT_MASK << PA13_SELECT_CFG1_SHIFT));
    value |= H3_PA13_SELECT_UART3_TX << PA13_SELECT_CFG1_SHIFT;
    // PA14, RX
    value &= static_cast<uint32_t>(~(GPIO_SELECT_MASK << PA14_SELECT_CFG1_SHIFT));
    value |= H3_PA14_SELECT_UART3_RX << PA14_SELECT_CFG1_SHIFT;
    H3_PIO_PORTA->CFG1 = value;

    H3_CCU->BUS_CLK_GATING3 |= CCU_BUS_CLK_GATING3_UART3;
    H3_CCU->BUS_SOFT_RESET4 |= CCU_BUS_SOFT_RESET4_UART3;
#else
#error Unsupported UART device configured
#endif

    EXT_UART->O08.FCR = 0;

    EXT_UART->LCR = UART_LCR_DLAB;
    EXT_UART->O00.DLL = BAUD_250000_L;
    EXT_UART->O04.DLH = BAUD_250000_H;
    EXT_UART->LCR = UART_LCR_8_N_2;

    __ISB();
}

static void UartEnableFifo() { // DMX Output
    EXT_UART->O08.FCR = UART_FCR_EFIFO | UART_FCR_TRESET;
    EXT_UART->O04.IER = 0;
    __ISB();
}

static void UartDisableFifo() { // DMX Input
    EXT_UART->O08.FCR = 0;
    EXT_UART->O04.IER = UART_IER_ERBFI;
    __ISB();
}

Dmx* Dmx::s_this = nullptr;

Dmx::Dmx() {
    assert(s_this == nullptr);
    s_this = this;

    s_DmxTransmitBreakTimeIntv = m_nDmxTransmitBreakTime * 12;
    s_DmxTransmitMabTimeIntv = m_nDmxTransmitMabTime * 12;
    s_DmxTransmitPeriodIntv = (transmit::kPeriodDefault * 12) - s_DmxTransmitBreakTimeIntv - s_DmxTransmitMabTimeIntv;

    H3GpioFsel(GPIO_EXT_12, GPIO_FSEL_OUTPUT);
    H3GpioClr(GPIO_EXT_12); // 0 = input, 1 = output

#define LOGIC_ANALYZER
#ifdef LOGIC_ANALYZER
#if defined GPIO_ANALYZER_CH1
    H3GpioFsel(GPIO_ANALYZER_CH1, GPIO_FSEL_OUTPUT);
    H3GpioClr(GPIO_ANALYZER_CH1);
#endif
#if defined GPIO_ANALYZER_CH2
    H3GpioFsel(GPIO_ANALYZER_CH2, GPIO_FSEL_OUTPUT);
    H3GpioClr(GPIO_ANALYZER_CH2);
#endif
#if defined GPIO_ANALYZER_CH3
    H3GpioFsel(GPIO_ANALYZER_CH3, GPIO_FSEL_OUTPUT);
    H3GpioClr(GPIO_ANALYZER_CH3);
#endif
#if defined GPIO_ANALYZER_CH4
    H3GpioFsel(GPIO_ANALYZER_CH4, GPIO_FSEL_OUTPUT);
    H3GpioClr(GPIO_ANALYZER_CH4);
#endif
#if defined GPIO_ANALYZER_CH5
    H3GpioFsel(GPIO_ANALYZER_CH5, GPIO_FSEL_OUTPUT);
    H3GpioClr(GPIO_ANALYZER_CH5);
#endif
#if defined GPIO_ANALYZER_CH6
    H3GpioFsel(GPIO_ANALYZER_CH6, GPIO_FSEL_OUTPUT);
    H3GpioClr(GPIO_ANALYZER_CH6);
#endif
#if defined GPIO_ANALYZER_CH7
    H3GpioFsel(GPIO_ANALYZER_CH7, GPIO_FSEL_OUTPUT);
    H3GpioClr(GPIO_ANALYZER_CH7);
#endif
#endif

    ClearData(0);

    sv_nRdmDataBufferIndexHead = 0;
    sv_nRdmDataBufferIndexTail = 0;

    sv_DmxReceiveState = IDLE;

    sv_DmxTransmitState = IDLE;

    irq_handler_init();

    irq_timer_set(IRQ_TIMER_1, IrqTimer1DmxReceive);
    H3_TIMER->TMR1_INTV = 0xB71B00; // 1 second
    H3_TIMER->TMR1_CTRL &= ~(TIMER_CTRL_SINGLE_MODE);
    H3_TIMER->TMR1_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD); // 0x3;

    H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_SINGLE_MODE);

    gic_fiq_config(UART_IRQN, GIC_CORE0);

    uart_dmx_config();

    __disable_fiq();
    arm_install_handler(reinterpret_cast<unsigned>(FiqDmx), ARM_VECTOR(ARM_VECTOR_FIQ));
}

void Dmx::StartOutput(uint32_t port_index) {
    if ((sv_PortState == dmx::PortState::TX) && (s_OutputStyle == dmx::OutputStyle::kDelta) && (sv_DmxTransmitState == IDLE)) {
        StartDmxOutput(port_index);
    }
}

void Dmx::Sync() {
    if (sv_PortState == dmx::PortState::TX) {
        if ((s_OutputStyle == dmx::OutputStyle::kDelta) && (sv_DmxTransmitState == IDLE)) {
            StartDmxOutput(0);
        }
    }
}

void Dmx::StartData([[maybe_unused]] uint32_t port_index) {
    assert(sv_PortState == PortState::IDLE);

    if (s_nPortDirection == PortDirection::kOutput) {
        sv_PortState = PortState::TX;
        return;
    }

    if (s_nPortDirection == PortDirection::kInput) {
        sv_DmxReceiveState = IDLE;

        irq_timer_set(IRQ_TIMER_0, IrqTimer0DmxReceive);
        H3_TIMER->TMR0_CTRL |= TIMER_CTRL_SINGLE_MODE;

        while ((EXT_UART->USR & UART_USR_BUSY) == UART_USR_BUSY) {
            (void)EXT_UART->O00.RBR;
        }

        UartDisableFifo();
        __enable_fiq();

        __ISB();
        return;
    }

    assert(0);
    __builtin_unreachable();
}

void Dmx::StopData([[maybe_unused]] uint32_t port_index) {
    DEBUG_PRINTF("port_index=%u, sv_PortState=%u", port_index, sv_PortState);

    if (sv_PortState == PortState::IDLE) {
        return;
    }

    if (s_nPortDirection == PortDirection::kOutput) {
        do {
            __DMB();
            if (sv_DmxTransmitState == DMXINTER) {
                while (!(EXT_UART->USR & UART_USR_TFE));
                sv_DmxTransmitState = IDLE;
            }
            __DMB();
        } while (sv_DmxTransmitState != IDLE);
    } else if (s_nPortDirection == PortDirection::kInput) {
        sv_DmxReceiveState = IDLE;
    } else {
        assert(0);
        __builtin_unreachable();
    }

    irq_timer_set(IRQ_TIMER_0, nullptr);

    __disable_fiq();
    __ISB();

    s_DmxData.Statistics.nSlotsInPacket = 0;

    sv_PortState = PortState::IDLE;
}

void Dmx::SetPortDirection(uint32_t port_index, PortDirection port_direction, bool enable_data) {
    DEBUG_PRINTF("port_index=%u %s %c", port_index, port_direction == PortDirection::kInput ? "Input" : "Output", bEnableData ? 'Y' : 'N');
    assert(port_index == 0);

    if (s_nPortDirection != port_direction) {
        s_nPortDirection = port_direction;

        StopData(port_index);

        switch (port_direction) {
            case PortDirection::kOutput:
				UartEnableFifo();
                H3GpioSet(GPIO_EXT_12); // 0 = input, 1 = output
                break;
            case PortDirection::kInput:
            default:
                H3GpioClr(GPIO_EXT_12); // 0 = input, 1 = output
                break;
        }
    } else if (!enable_data) {
        StopData(port_index);
    }

    if (enable_data) {
        StartData(port_index);
    }
}

PortDirection Dmx::GetPortDirection([[maybe_unused]] uint32_t port_index) {
    assert(port_index == 0);

    return s_nPortDirection;
}

// DMX

void Dmx::SetDmxBreakTime(uint32_t break_time) {
    m_nDmxTransmitBreakTime = std::max(transmit::kBreakTimeMin, break_time);
    s_DmxTransmitBreakTimeIntv = m_nDmxTransmitBreakTime * 12;

    SetDmxPeriodTime(m_nDmxTransmitPeriodRequested);
}

void Dmx::SetDmxMabTime(uint32_t mab_time) {
    m_nDmxTransmitMabTime = std::max(transmit::kMabTimeMin, mab_time);
    s_DmxTransmitMabTimeIntv = m_nDmxTransmitMabTime * 12;

    SetDmxPeriodTime(m_nDmxTransmitPeriodRequested);
}

void Dmx::SetDmxPeriodTime(uint32_t nPeriodTime) {
    const auto kPackageLengthUs = m_nDmxTransmitBreakTime + m_nDmxTransmitMabTime + (s_nDmxSendDataLength * 44);

    m_nDmxTransmitPeriodRequested = nPeriodTime;

    if (nPeriodTime != 0) {
        if (nPeriodTime < kPackageLengthUs) {
            m_nDmxTransmitPeriod = std::max(transmit::kBreakToBreakTimeMin, kPackageLengthUs + 44);
        } else {
            m_nDmxTransmitPeriod = nPeriodTime;
        }
    } else {
        m_nDmxTransmitPeriod = std::max(transmit::kBreakToBreakTimeMin, kPackageLengthUs + 44);
    }

    s_DmxTransmitPeriodIntv = (m_nDmxTransmitPeriod * 12) - s_DmxTransmitBreakTimeIntv - s_DmxTransmitMabTimeIntv;
}

void Dmx::SetDmxSlots(uint16_t nSlots) {
    if ((nSlots >= 2) && (nSlots <= dmx::kChannelsMax)) {
        m_nDmxTransmitSlots = nSlots;
        s_nDmxSendDataLength = 1U + m_nDmxTransmitSlots;
        SetDmxPeriodTime(m_nDmxTransmitPeriodRequested);
    }
}

const uint8_t* Dmx::GetDmxCurrentData([[maybe_unused]] uint32_t port_index) {
    return s_DmxData.Data;
}

const uint8_t* Dmx::GetDmxAvailable([[maybe_unused]] uint32_t port_index) {
    __DMB();

    auto slots_in_packet = s_DmxData.Statistics.nSlotsInPacket;

    if ((slots_in_packet & 0x8000) != 0x8000) {
        return nullptr;
    }

    H3GpioSet(GPIO_ANALYZER_CH3);

    slots_in_packet &= ~0x8000;
    slots_in_packet--; // Remove SC from length
    s_DmxData.Statistics.nSlotsInPacket = slots_in_packet;

    H3GpioClr(GPIO_ANALYZER_CH3);

    return const_cast<const uint8_t*>(s_DmxData.Data);
}

const uint8_t* Dmx::GetDmxChanged([[maybe_unused]] uint32_t port_index) {
    const auto* p = GetDmxAvailable(0);
    auto* src = reinterpret_cast<const uint32_t*>(p);

    if (src == nullptr) {
        return nullptr;
    }

    auto* dst = reinterpret_cast<uint32_t*>(s_DmxDataPrevious);
    const auto* dmx_statistics = reinterpret_cast<const struct Data*>(p);

    if (dmx_statistics->Statistics.nSlotsInPacket != sv_DmxSlotsInPacketPrevious) {
        H3GpioSet(GPIO_ANALYZER_CH1);

        sv_DmxSlotsInPacketPrevious = dmx_statistics->Statistics.nSlotsInPacket;
        for (uint32_t i = 0; i < buffer::kSize / 4; i++) {
            *dst = *src;
            dst++;
            src++;
        }

        H3GpioClr(GPIO_ANALYZER_CH1);
        return p;
    }

    H3GpioSet(GPIO_ANALYZER_CH2);

    auto is_changed = false;

    for (uint32_t i = 0; i < buffer::kSize / 4; i++) {
        if (*dst != *src) {
            *dst = *src;
            is_changed = true;
        }
        dst++;
        src++;
    }

    H3GpioClr(GPIO_ANALYZER_CH2);

    return (is_changed ? p : nullptr);
}

void Dmx::SetOutputStyle(uint32_t port_index, dmx::OutputStyle output_style) {
    if (s_OutputStyle == output_style) {
        return;
    }

    s_OutputStyle = output_style;

    if ((s_OutputStyle == dmx::OutputStyle::kConstant) && (s_nPortDirection == dmx::PortDirection::kOutput)) {
        StopData(port_index);
        StartDmxOutput(port_index);
    }
}

dmx::OutputStyle Dmx::GetOutputStyle([[maybe_unused]] uint32_t port_index) const {
    return s_OutputStyle;
}

template <dmx::SendStyle dmxSendStyle> void Dmx::SetSendData([[maybe_unused]] uint32_t port_index, const uint8_t* pData, uint32_t nLength) {
    assert(port_index == 0);

    do {
        __DMB();
    } while (sv_DmxTransmitState != IDLE && sv_DmxTransmitState != DMXINTER);

    __builtin_prefetch(pData);
    memcpy(reinterpret_cast<void*>(s_DmxData.Data), pData, nLength);

    if (nLength != s_nDmxSendDataLength) {
        s_nDmxSendDataLength = nLength;
        SetDmxPeriodTime(m_nDmxTransmitPeriodRequested);
    }

    if constexpr (dmxSendStyle == dmx::SendStyle::kDirect) {
        StartOutput(port_index);
    }
}

template <dmx::SendStyle dmxSendStyle> void Dmx::SetSendDataWithoutSC([[maybe_unused]] uint32_t port_index, const uint8_t* pData, uint32_t nLength) {
    do {
        __DMB();
    } while (sv_DmxTransmitState != IDLE && sv_DmxTransmitState != DMXINTER);

    nLength = std::min(nLength, static_cast<uint32_t>(m_nDmxTransmitSlots));

    s_DmxData.Data[0] = kStartCode;

    __builtin_prefetch(pData);
    memcpy(&s_DmxData.Data[1], pData, nLength);

    if (nLength != s_nDmxSendDataLength) {
        s_nDmxSendDataLength = 1U + nLength;
        SetDmxPeriodTime(m_nDmxTransmitPeriodRequested);
    }

    if constexpr (dmxSendStyle == dmx::SendStyle::kDirect) {
        StartOutput(port_index);
    }
}

void Dmx::Blackout() {
    DEBUG_ENTRY();

    do {
        __DMB();
    } while (sv_DmxTransmitState != IDLE && sv_DmxTransmitState != DMXINTER);

    auto* p = reinterpret_cast<uint32_t*>(s_DmxData.Data);

    for (uint32_t i = 0; i < buffer::kSize / 4; i++) {
        *p++ = 0;
    }

    s_DmxData.Data[0] = kStartCode;

    DEBUG_EXIT();
}

void Dmx::FullOn() {
    DEBUG_ENTRY();

    do {
        __DMB();
    } while (sv_DmxTransmitState != IDLE && sv_DmxTransmitState != DMXINTER);

    auto* p = reinterpret_cast<uint32_t*>(s_DmxData.Data);

    for (uint32_t i = 0; i < buffer::kSize / 4; i++) {
        *p++ = static_cast<uint32_t>(~0);
    }

    s_DmxData.Data[0] = kStartCode;

    DEBUG_EXIT();
}

uint32_t Dmx::GetDmxUpdatesPerSecond([[maybe_unused]] uint32_t port_index) {
    __DMB();
    return sv_nDmxUpdatesPerSecond;
}

void Dmx::ClearData([[maybe_unused]] uint32_t port_index) {
    auto* p = reinterpret_cast<uint32_t*>(s_DmxData.Data);

    for (uint32_t i = 0; i < buffer::kSize / 4; i++) {
        *p++ = 0;
    }
}

volatile dmx::TotalStatistics& Dmx::GetTotalStatistics([[maybe_unused]] uint32_t port_index) {
    return sv_TotalStatistics[0];
}

void Dmx::StartDmxOutput([[maybe_unused]] uint32_t port_index) {
    assert(port_index == 0);

    UartEnableFifo();
    __enable_fiq();

    irq_timer_set(IRQ_TIMER_0, IrqTimer0DmxSender);

    H3_TIMER->TMR0_INTV = s_DmxTransmitBreakTimeIntv;
    H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD | TIMER_CTRL_SINGLE_MODE);

    EXT_UART->LCR = UART_LCR_8_N_2 | UART_LCR_BC;
    __ISB();

    sv_DmxTransmitState = BREAK;
}

const uint8_t* Dmx::RdmReceive([[maybe_unused]] uint32_t port_index) {
    assert(port_index == 0);

    __DMB();
    if (sv_nRdmDataBufferIndexHead == sv_nRdmDataBufferIndexTail) {
        return nullptr;
    } else {
        const auto* p = &s_RdmData[sv_nRdmDataBufferIndexTail][0];
        sv_nRdmDataBufferIndexTail = (sv_nRdmDataBufferIndexTail + 1) & RDM_DATA_BUFFER_INDEX_MASK;
        return p;
    }
}

const uint8_t* Dmx::RdmReceiveTimeOut(uint32_t port_index, uint16_t nTimeOut) {
    assert(port_index == 0);

    uint8_t* p = nullptr;
    const auto kMicros = H3_TIMER->AVS_CNT1;

    do {
        if ((p = const_cast<uint8_t*>(RdmReceive(port_index))) != nullptr) {
            return reinterpret_cast<const uint8_t*>(p);
        }
    } while ((H3_TIMER->AVS_CNT1 - kMicros) < nTimeOut);

    return p;
}

void Dmx::RdmSendRaw([[maybe_unused]] uint32_t port_index, const uint8_t* pRdmData, uint32_t nLength) {
    assert(port_index == 0);

    while (!(EXT_UART->LSR & UART_LSR_TEMT));

    EXT_UART->LCR = UART_LCR_8_N_2 | UART_LCR_BC;
    udelay(RDM_TRANSMIT_BREAK_TIME);

    EXT_UART->LCR = UART_LCR_8_N_2;
    udelay(RDM_TRANSMIT_MAB_TIME);

    for (uint32_t i = 0; i < nLength; i++) {
        while (!(EXT_UART->LSR & UART_LSR_THRE));
        EXT_UART->O00.THR = pRdmData[i];
    }

    while ((EXT_UART->USR & UART_USR_BUSY) == UART_USR_BUSY) {
        (void)EXT_UART->O00.RBR;
    }
}

void Dmx::RdmSendDiscoveryRespondMessage([[maybe_unused]] uint32_t port_index, const uint8_t* pRdmData, uint32_t nLength) {
    DEBUG_PRINTF("port_index=%u, pRdmData=%p, nLength=%u", port_index, pRdmData, nLength);
    assert(port_index < dmx::config::max::kPorts);
    assert(pRdmData != nullptr);
    assert(nLength != 0);
		
    // 3.2.2 Responder Packet spacing
    udelay(RDM_RESPONDER_PACKET_SPACING, gsv_RdmDataReceiveEnd);

    SetPortDirection(port_index, dmx::PortDirection::kOutput, false);

    EXT_UART->LCR = UART_LCR_8_N_2;

    for (uint32_t i = 0; i < nLength; i++) {
        while (!(EXT_UART->LSR & UART_LSR_THRE));
        EXT_UART->O00.THR = pRdmData[i];
    }

    while (!((EXT_UART->LSR & UART_LSR_TEMT) == UART_LSR_TEMT)) {
    }

    udelay(RDM_RESPONDER_DATA_DIRECTION_DELAY);

    SetPortDirection(port_index, dmx::PortDirection::kInput, true);

    DEBUG_EXIT();
}

// Explicit template instantiations
template void Dmx::SetSendData<dmx::SendStyle::kDirect>(const uint32_t, const uint8_t*, uint32_t);
template void Dmx::SetSendData<dmx::SendStyle::kSync>(const uint32_t, const uint8_t*, uint32_t);

template void Dmx::SetSendDataWithoutSC<dmx::SendStyle::kDirect>(const uint32_t, const uint8_t*, uint32_t);
template void Dmx::SetSendDataWithoutSC<dmx::SendStyle::kSync>(const uint32_t, const uint8_t*, uint32_t);
