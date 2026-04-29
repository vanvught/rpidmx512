/**
 * @file dmx.cpp
 *
 */
/* Copyright (C) 2018-2026 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#if defined(DEBUG_DMX)
#undef NDEBUG
#endif

#if __GNUC__ > 8
#pragma GCC target("general-regs-only")
#endif

#include <cstdint>
#include <cstring>
#include <algorithm>
#include <cassert>

#include "dmx.h"
#include "dmxconst.h"
#include "./../dmx_internal.h"
#include "arm/arm.h"
#include "arm/synchronize.h"
#include "arm/gic.h"
#include "h3.h"
#include "h3_uart.h"
#include "h3_hs_timer.h"
#include "h3_dma.h"
#include "h3_ccu.h"
#include "h3_gpio.h"
#include "h3_timer.h"
#include "irq_timer.h"
#include "rdm.h"
#include "rdm_e120.h"
#include "logic_analyzer.h"
#include "firmware/debug/debug_debug.h"

using namespace dmx;

#ifndef ALIGNED
#define ALIGNED __attribute__((aligned(4)))
#endif

static constexpr auto DMX_DATA_OUT_INDEX = (1U << 2);

enum class TxRxState { IDLE = 0, PRE_BREAK, BREAK, MAB, DMXDATA, RDMDATA, CHECKSUMH, CHECKSUML, RDMDISC, DMXINTER };

enum class PortState { IDLE = 0, TX, RX };

struct TDmxMultiData {
    uint8_t data[buffer::SIZE]; // multiple of uint32_t
    uint32_t nLength;
};

struct TCoherentRegion {
    struct sunxi_dma_lli lli[dmx::config::max::PORTS];
    struct TDmxMultiData dmx_data[dmx::config::max::PORTS][DMX_DATA_OUT_INDEX] ALIGNED;
};

struct TRdmMultiData {
    uint8_t data[RDM_DATA_BUFFER_SIZE];
    uint32_t nIndex;
    uint32_t nDiscIndex;
};

static volatile TxRxState sv_PortReceiveState[dmx::config::max::PORTS] ALIGNED;

static volatile dmx::TotalStatistics sv_TotalStatistics[dmx::config::max::PORTS] ALIGNED;

#if defined(ORANGE_PI)
static constexpr uint8_t s_nDmxDataDirectionGpioPin[dmx::config::max::PORTS] = {GPIO_DMX_DATA_DIRECTION_OUT_C, GPIO_DMX_DATA_DIRECTION_OUT_B};
#else
static constexpr uint8_t s_nDmxDataDirectionGpioPin[dmx::config::max::PORTS] = {GPIO_DMX_DATA_DIRECTION_OUT_D, GPIO_DMX_DATA_DIRECTION_OUT_A, GPIO_DMX_DATA_DIRECTION_OUT_B, GPIO_DMX_DATA_DIRECTION_OUT_C};
#endif

// DMX TX

static uint32_t s_nDmxTransmistBreakTimeINTV;
static uint32_t s_nDmxTransmitMabTimeINTV;
static uint32_t s_nDmxTransmitPeriodINTV;

static struct TCoherentRegion* s_pCoherentRegion;

static volatile uint32_t sv_nDmxDataWriteIndex[dmx::config::max::PORTS];
static volatile uint32_t sv_nDmxDataReadIndex[dmx::config::max::PORTS];

static volatile TxRxState sv_DmxSendState ALIGNED;

// DMX RX

static uint8_t s_RxDmxPrevious[dmx::config::max::PORTS][buffer::SIZE] ALIGNED;
static volatile struct Data s_aDmxData[dmx::config::max::PORTS][buffer::INDEX_ENTRIES] ALIGNED;
static volatile uint32_t s_nDmxDataBufferIndexHead[dmx::config::max::PORTS];
static volatile uint32_t s_nDmxDataBufferIndexTail[dmx::config::max::PORTS];
static volatile uint32_t s_nDmxDataIndex[dmx::config::max::PORTS];

static volatile uint32_t sv_nDmxUpdatesPerSecond[dmx::config::max::PORTS];
static volatile uint32_t sv_nDmxPackets[dmx::config::max::PORTS];
static volatile uint32_t sv_nDmxPacketsPrevious[dmx::config::max::PORTS];

// RDM

volatile uint32_t gsv_RdmDataReceiveEnd;

static struct TRdmMultiData s_aRdmData[dmx::config::max::PORTS][RDM_DATA_BUFFER_INDEX_ENTRIES] ALIGNED;
static struct TRdmMultiData* s_pRdmDataCurrent[dmx::config::max::PORTS] ALIGNED;

static volatile uint32_t s_nRdmDataWriteIndex[dmx::config::max::PORTS];
static volatile uint32_t s_nRdmDataReadIndex[dmx::config::max::PORTS];

static volatile PortState sv_port_state[dmx::config::max::PORTS] ALIGNED;

static void irq_timer0_dmx_multi_sender([[maybe_unused]] uint32_t clo) {
    switch (sv_DmxSendState) {
        case TxRxState::IDLE:
        case TxRxState::DMXINTER:
            H3_TIMER->TMR0_INTV = s_nDmxTransmistBreakTimeINTV;
            H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD); // 0x3;

            if (sv_port_state[0] == PortState::TX) {
                H3_UART1->LCR = UART_LCR_8_N_2 | UART_LCR_BC;
            }

            if (sv_port_state[1] == PortState::TX) {
                H3_UART2->LCR = UART_LCR_8_N_2 | UART_LCR_BC;
            }
#if defined(ORANGE_PI_ONE)
            if (sv_port_state[2] == PortState::TX) {
                H3_UART3->LCR = UART_LCR_8_N_2 | UART_LCR_BC;
            }
#ifndef DO_NOT_USE_UART0
            if (sv_port_state[3] == PortState::TX) {
                H3_UART0->LCR = UART_LCR_8_N_2 | UART_LCR_BC;
            }
#endif
#endif

            if (sv_nDmxDataWriteIndex[0] != sv_nDmxDataReadIndex[0]) {
                sv_nDmxDataReadIndex[0] = (sv_nDmxDataReadIndex[0] + 1) & (DMX_DATA_OUT_INDEX - 1);

                s_pCoherentRegion->lli[0].src = reinterpret_cast<uint32_t>(&s_pCoherentRegion->dmx_data[0][sv_nDmxDataReadIndex[0]].data[0]);
                s_pCoherentRegion->lli[0].len = s_pCoherentRegion->dmx_data[0][sv_nDmxDataReadIndex[0]].nLength;
            }

            if (sv_nDmxDataWriteIndex[1] != sv_nDmxDataReadIndex[1]) {
                sv_nDmxDataReadIndex[1] = (sv_nDmxDataReadIndex[1] + 1) & (DMX_DATA_OUT_INDEX - 1);

                s_pCoherentRegion->lli[1].src = reinterpret_cast<uint32_t>(&s_pCoherentRegion->dmx_data[1][sv_nDmxDataReadIndex[1]].data[0]);
                s_pCoherentRegion->lli[1].len = s_pCoherentRegion->dmx_data[1][sv_nDmxDataReadIndex[1]].nLength;
            }
#if defined(ORANGE_PI_ONE)
            if (sv_nDmxDataWriteIndex[2] != sv_nDmxDataReadIndex[2]) {
                sv_nDmxDataReadIndex[2] = (sv_nDmxDataReadIndex[2] + 1) & (DMX_DATA_OUT_INDEX - 1);

                s_pCoherentRegion->lli[2].src = reinterpret_cast<uint32_t>(&s_pCoherentRegion->dmx_data[2][sv_nDmxDataReadIndex[2]].data[0]);
                s_pCoherentRegion->lli[2].len = s_pCoherentRegion->dmx_data[2][sv_nDmxDataReadIndex[2]].nLength;
            }
#ifndef DO_NOT_USE_UART0
            if (sv_nDmxDataWriteIndex[3] != sv_nDmxDataReadIndex[3]) {
                sv_nDmxDataReadIndex[3] = (sv_nDmxDataReadIndex[3] + 1) & (DMX_DATA_OUT_INDEX - 1);

                s_pCoherentRegion->lli[3].src = reinterpret_cast<uint32_t>(&s_pCoherentRegion->dmx_data[3][sv_nDmxDataReadIndex[3]].data[0]);
                s_pCoherentRegion->lli[3].len = s_pCoherentRegion->dmx_data[3][sv_nDmxDataReadIndex[3]].nLength;
            }
#endif
#endif
            sv_DmxSendState = TxRxState::BREAK;
            break;
        case TxRxState::BREAK:
            H3_TIMER->TMR0_INTV = s_nDmxTransmitMabTimeINTV;
            H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD); // 0x3;

            if (sv_port_state[0] == PortState::TX) {
                H3_UART1->LCR = UART_LCR_8_N_2;
            }

            if (sv_port_state[1] == PortState::TX) {
                H3_UART2->LCR = UART_LCR_8_N_2;
            }
#if defined(ORANGE_PI_ONE)
            if (sv_port_state[2] == PortState::TX) {
                H3_UART3->LCR = UART_LCR_8_N_2;
            }
#ifndef DO_NOT_USE_UART0
            if (sv_port_state[3] == PortState::TX) {
                H3_UART0->LCR = UART_LCR_8_N_2;
            }
#endif
#endif
            sv_DmxSendState = TxRxState::MAB;
            break;
        case TxRxState::MAB:
            H3_TIMER->TMR0_INTV = s_nDmxTransmitPeriodINTV;
            H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD); // 0x3;

            if (sv_port_state[0] == PortState::TX) {
                H3_DMA_CHL0->DESC_ADDR = reinterpret_cast<uint32_t>(&s_pCoherentRegion->lli[0]);
                H3_DMA_CHL0->EN = DMA_CHAN_ENABLE_START;
                sv_TotalStatistics[0].dmx.sent++;
            }

            if (sv_port_state[1] == PortState::TX) {
                H3_DMA_CHL1->DESC_ADDR = reinterpret_cast<uint32_t>(&s_pCoherentRegion->lli[1]);
                H3_DMA_CHL1->EN = DMA_CHAN_ENABLE_START;
                sv_TotalStatistics[1].dmx.sent++;
            }
#if defined(ORANGE_PI_ONE)
            if (sv_port_state[2] == PortState::TX) {
                H3_DMA_CHL2->DESC_ADDR = reinterpret_cast<uint32_t>(&s_pCoherentRegion->lli[2]);
                H3_DMA_CHL2->EN = DMA_CHAN_ENABLE_START;
                sv_TotalStatistics[2].dmx.sent++;
            }
#ifndef DO_NOT_USE_UART0
            if (sv_port_state[3] == PortState::TX) {
                H3_DMA_CHL3->DESC_ADDR = reinterpret_cast<uint32_t>(&s_pCoherentRegion->lli[3]);
                H3_DMA_CHL3->EN = DMA_CHAN_ENABLE_START;
                sv_TotalStatistics[3].dmx.sent++;
            }
#endif
#endif
            __ISB();

            sv_DmxSendState = TxRxState::DMXINTER;
            break;
        default:
            assert(0);
            __builtin_unreachable();
            break;
    }
}

template <uint32_t port_index> void FiqInHandler(H3_UART_TypeDef* uart, uint32_t iir) {
    uint32_t index;

    __ISB();

    if (uart->LSR & (UART_LSR_BI | UART_LSR_FE | UART_LSR_FIFOERR)) {
        sv_PortReceiveState[port_index] = TxRxState::PRE_BREAK;
    }

    auto rfl = uart->RFL;

    while (rfl--) {
        while ((uart->LSR & UART_LSR_DR) != UART_LSR_DR);
        const auto kData = static_cast<uint8_t>(uart->O00.RBR);
        __DMB();

        switch (sv_PortReceiveState[port_index]) {
            case TxRxState::IDLE:
                s_pRdmDataCurrent[port_index]->data[0] = kData;
                s_pRdmDataCurrent[port_index]->nIndex = 1;

                sv_PortReceiveState[port_index] = TxRxState::RDMDISC;
                break;
            case TxRxState::PRE_BREAK:
                sv_PortReceiveState[port_index] = TxRxState::BREAK;
                break;
            case TxRxState::BREAK:
                switch (kData) {
                    case kStartCode:
                        sv_PortReceiveState[port_index] = TxRxState::DMXDATA;
                        s_aDmxData[port_index][s_nDmxDataBufferIndexHead[port_index]].Data[0] = kStartCode;
                        s_nDmxDataIndex[port_index] = 1;
                        sv_nDmxPackets[port_index]++;
                        break;
                    case E120_SC_RDM:
                        s_pRdmDataCurrent[port_index]->data[0] = E120_SC_RDM;
                        s_pRdmDataCurrent[port_index]->nIndex = 1;

                        sv_PortReceiveState[port_index] = TxRxState::RDMDATA;
                        break;
                    default:
                        sv_PortReceiveState[port_index] = TxRxState::IDLE;
                        break;
                }
                break;
            case TxRxState::DMXDATA:
                s_aDmxData[port_index][s_nDmxDataBufferIndexHead[port_index]].Data[s_nDmxDataIndex[port_index]] = kData;
                s_nDmxDataIndex[port_index]++;

                if (s_nDmxDataIndex[port_index] > dmx::kChannelsMax) {
                    sv_PortReceiveState[port_index] = TxRxState::IDLE;
                    s_aDmxData[port_index][s_nDmxDataBufferIndexHead[port_index]].Statistics.nSlotsInPacket = dmx::kChannelsMax;
                    s_nDmxDataBufferIndexHead[port_index] = (s_nDmxDataBufferIndexHead[port_index] + 1) & buffer::INDEX_MASK;
                    return;
                }
                break;
            case TxRxState::RDMDATA:
                if (s_pRdmDataCurrent[port_index]->nIndex > RDM_DATA_BUFFER_SIZE) {
                    sv_PortReceiveState[port_index] = TxRxState::IDLE;
                } else {
                    index = s_pRdmDataCurrent[port_index]->nIndex;
                    s_pRdmDataCurrent[port_index]->data[index] = kData;
                    s_pRdmDataCurrent[port_index]->nIndex++;

                    const auto* p = reinterpret_cast<struct TRdmMessage*>(&s_pRdmDataCurrent[port_index]->data[0]);

                    if (s_pRdmDataCurrent[port_index]->nIndex == p->message_length) {
                        sv_PortReceiveState[port_index] = TxRxState::CHECKSUMH;
                    }
                }
                break;
            case TxRxState::CHECKSUMH:
                index = s_pRdmDataCurrent[port_index]->nIndex;
                s_pRdmDataCurrent[port_index]->data[index] = kData;
                s_pRdmDataCurrent[port_index]->nIndex++;

                sv_PortReceiveState[port_index] = TxRxState::CHECKSUML;
                break;
            case TxRxState::CHECKSUML: {
                index = s_pRdmDataCurrent[port_index]->nIndex;
                s_pRdmDataCurrent[port_index]->data[index] = kData;
                s_pRdmDataCurrent[port_index]->nIndex++;

                s_nRdmDataWriteIndex[port_index] = (s_nRdmDataWriteIndex[port_index] + 1) & RDM_DATA_BUFFER_INDEX_MASK;
                s_pRdmDataCurrent[port_index] = &s_aRdmData[port_index][s_nRdmDataWriteIndex[port_index]];
                gsv_RdmDataReceiveEnd = H3_HS_TIMER->CURNT_LO;
                __DMB();

                sv_PortReceiveState[port_index] = TxRxState::IDLE;
            } break;
            case TxRxState::RDMDISC:
                index = s_pRdmDataCurrent[port_index]->nIndex;

                if (index < 24) {
                    s_pRdmDataCurrent[port_index]->data[index] = kData;
                    s_pRdmDataCurrent[port_index]->nIndex++;
                }

                break;
            default:
                sv_PortReceiveState[port_index] = TxRxState::IDLE;
                break;
        }
    }

    if (((uart->USR & UART_USR_BUSY) == 0) && ((iir & UART_IIR_IID_TIME_OUT) == UART_IIR_IID_TIME_OUT)) {
        if (sv_PortReceiveState[port_index] == TxRxState::DMXDATA) {
            sv_PortReceiveState[port_index] = TxRxState::IDLE;
            s_aDmxData[port_index][s_nDmxDataBufferIndexHead[port_index]].Statistics.nSlotsInPacket = s_nDmxDataIndex[port_index] - 1;
            s_nDmxDataBufferIndexHead[port_index] = (s_nDmxDataBufferIndexHead[port_index] + 1) & buffer::INDEX_MASK;
        }

        if (sv_PortReceiveState[port_index] == TxRxState::RDMDISC) {
            sv_PortReceiveState[port_index] = TxRxState::IDLE;
            s_nRdmDataWriteIndex[port_index] = (s_nRdmDataWriteIndex[port_index] + 1) & RDM_DATA_BUFFER_INDEX_MASK;
            s_pRdmDataCurrent[port_index] = &s_aRdmData[port_index][s_nRdmDataWriteIndex[port_index]];
            gsv_RdmDataReceiveEnd = H3_HS_TIMER->CURNT_LO;
            __DMB();
        }
    }
}

static void __attribute__((interrupt("FIQ"))) fiq_dmx_multi() {
    __DMB();

    auto iir = H3_UART1->O08.IIR;
    if (iir & UART_IIR_IID_RD) {
        FiqInHandler<0>(reinterpret_cast<H3_UART_TypeDef*>(H3_UART1_BASE), iir);
        H3_GIC_CPUIF->EOI = H3_UART1_IRQn;
        gic_unpend<H3_UART1_IRQn>();
    }

    iir = H3_UART2->O08.IIR;
    if (iir & UART_IIR_IID_RD) {
        FiqInHandler<1>(reinterpret_cast<H3_UART_TypeDef*>(H3_UART2_BASE), iir);
        H3_GIC_CPUIF->EOI = H3_UART2_IRQn;
        gic_unpend<H3_UART2_IRQn>();
    }
#if defined(ORANGE_PI_ONE)
    iir = H3_UART3->O08.IIR;
    if (iir & UART_IIR_IID_RD) {
        FiqInHandler<2>(reinterpret_cast<H3_UART_TypeDef*>(H3_UART3_BASE), iir);
        H3_GIC_CPUIF->EOI = H3_UART3_IRQn;
        gic_unpend<H3_UART3_IRQn>();
    }

#ifndef DO_NOT_USE_UART0
    iir = H3_UART0->O08.IIR;
    if (iir & UART_IIR_IID_RD) {
        FiqInHandler<3>(reinterpret_cast<H3_UART_TypeDef*>(H3_UART0_BASE), iir);
        H3_GIC_CPUIF->EOI = H3_UART0_IRQn;
        gic_unpend<H3_UART0_IRQn>();
    }
#endif
#endif

    __DMB();
}

static void IrqTimer1DmxReceive([[maybe_unused]] uint32_t clo) {
    for (uint32_t i = 0; i < dmx::config::max::PORTS; i++) {
        sv_nDmxUpdatesPerSecond[i] = sv_nDmxPackets[i] - sv_nDmxPacketsPrevious[i];
        sv_nDmxPacketsPrevious[i] = sv_nDmxPackets[i];
    }
}

static void UartConfig(uint32_t uart) {
    H3_UART_TypeDef* p = nullptr;

    if (uart == 1) {
        p = reinterpret_cast<H3_UART_TypeDef*>(H3_UART1_BASE);

        uint32_t value = H3_PIO_PORTG->CFG0;
        // PG6, TX
        value &= static_cast<uint32_t>(~(GPIO_SELECT_MASK << PG6_SELECT_CFG0_SHIFT));
        value |= H3_PG6_SELECT_UART1_TX << PG6_SELECT_CFG0_SHIFT;
        // PG7, RX
        value &= static_cast<uint32_t>(~(GPIO_SELECT_MASK << PG7_SELECT_CFG0_SHIFT));
        value |= H3_PG7_SELECT_UART1_RX << PG7_SELECT_CFG0_SHIFT;
        H3_PIO_PORTG->CFG0 = value;

        H3_CCU->BUS_SOFT_RESET4 |= CCU_BUS_SOFT_RESET4_UART1;
        H3_CCU->BUS_CLK_GATING3 |= CCU_BUS_CLK_GATING3_UART1;
    } else if (uart == 2) {
        p = reinterpret_cast<H3_UART_TypeDef*>(H3_UART2_BASE);

        uint32_t value = H3_PIO_PORTA->CFG0;
        // PA0, TX
        value &= static_cast<uint32_t>(~(GPIO_SELECT_MASK << PA0_SELECT_CFG0_SHIFT));
        value |= H3_PA0_SELECT_UART2_TX << PA0_SELECT_CFG0_SHIFT;
        // PA1, RX
        value &= static_cast<uint32_t>(~(GPIO_SELECT_MASK << PA1_SELECT_CFG0_SHIFT));
        value |= H3_PA1_SELECT_UART2_RX << PA1_SELECT_CFG0_SHIFT;
        H3_PIO_PORTA->CFG0 = value;

        H3_CCU->BUS_SOFT_RESET4 |= CCU_BUS_SOFT_RESET4_UART2;
        H3_CCU->BUS_CLK_GATING3 |= CCU_BUS_CLK_GATING3_UART2;
    }
#if defined(ORANGE_PI_ONE)
    else if (uart == 3) {
        p = reinterpret_cast<H3_UART_TypeDef*>(H3_UART3_BASE);

        uint32_t value = H3_PIO_PORTA->CFG1;
        // PA13, TX
        value &= static_cast<uint32_t>(~(GPIO_SELECT_MASK << PA13_SELECT_CFG1_SHIFT));
        value |= H3_PA13_SELECT_UART3_TX << PA13_SELECT_CFG1_SHIFT;
        // PA14, RX
        value &= static_cast<uint32_t>(~(GPIO_SELECT_MASK << PA14_SELECT_CFG1_SHIFT));
        value |= H3_PA14_SELECT_UART3_RX << PA14_SELECT_CFG1_SHIFT;
        H3_PIO_PORTA->CFG1 = value;

        H3_CCU->BUS_SOFT_RESET4 |= CCU_BUS_SOFT_RESET4_UART3;
        H3_CCU->BUS_CLK_GATING3 |= CCU_BUS_CLK_GATING3_UART3;
    }
#ifndef DO_NOT_USE_UART0
    else if (uart == 0) {
        p = reinterpret_cast<H3_UART_TypeDef*>(H3_UART0_BASE);

        uint32_t value = H3_PIO_PORTA->CFG0;
        // PA4, TX
        value &= static_cast<uint32_t>(~(GPIO_SELECT_MASK << PA4_SELECT_CFG0_SHIFT));
        value |= H3_PA4_SELECT_UART0_TX << PA4_SELECT_CFG0_SHIFT;
        // PA5, RX
        value &= static_cast<uint32_t>(~(GPIO_SELECT_MASK << PA5_SELECT_CFG0_SHIFT));
        value |= H3_PA5_SELECT_UART0_RX << PA5_SELECT_CFG0_SHIFT;
        H3_PIO_PORTA->CFG0 = value;

        H3_CCU->BUS_SOFT_RESET4 |= CCU_BUS_SOFT_RESET4_UART0;
        H3_CCU->BUS_CLK_GATING3 |= CCU_BUS_CLK_GATING3_UART0;
    }
#endif
#endif

    assert(p != nullptr);

    if (p != nullptr) {
        p->O08.FCR = 0;
        p->LCR = UART_LCR_DLAB;
        p->O00.DLL = BAUD_250000_L;
        p->O04.DLH = BAUD_250000_H;
        p->O04.IER = 0;
        p->LCR = UART_LCR_8_N_2;
    }

    __ISB();
}

static void UartEnableFifoTx(uint32_t port_index) { // DMX TX
    auto* uart = _port_to_uart(port_index);
    assert(uart != nullptr);

    if (uart != nullptr) {
        uart->O08.FCR = UART_FCR_EFIFO | UART_FCR_TRESET;
        uart->O04.IER = 0;
        __ISB();
    }
}

static void UartEnableFifoRx(uint32_t port_index) { // RDM RX
    auto* uart = _port_to_uart(port_index);
    assert(uart != nullptr);

    if (uart != nullptr) {
        uart->O08.FCR = UART_FCR_EFIFO | UART_FCR_RRESET | UART_FCR_TRIG1;
        uart->O04.IER = UART_IER_ERBFI;
        __ISB();
    }
}

Dmx* Dmx::s_this = nullptr;

Dmx::Dmx() {
    DEBUG_ENTRY();

    assert(s_this == nullptr);
    s_this = this;

    // DMX TX

    s_pCoherentRegion = reinterpret_cast<struct TCoherentRegion*>(H3_MEM_COHERENT_REGION + MEGABYTE / 2);

    s_nDmxTransmistBreakTimeINTV = m_nDmxTransmitBreakTime * 12;
    s_nDmxTransmitMabTimeINTV = m_nDmxTransmitMabTime * 12;
    s_nDmxTransmitPeriodINTV = (transmit::kPeriodDefault * 12) - s_nDmxTransmistBreakTimeINTV - s_nDmxTransmitMabTimeINTV;

    sv_DmxSendState = TxRxState::IDLE;

    for (uint32_t port_index = 0; port_index < config::max::PORTS; port_index++) {
        // DMX TX
        ClearData(port_index);
        sv_nDmxDataWriteIndex[port_index] = 0;
        sv_nDmxDataReadIndex[port_index] = 0;
        m_nDmxTransmissionLength[port_index] = dmx::kChannelsMax;
        // DMA UART TX
        auto* lli = &s_pCoherentRegion->lli[port_index];
        H3_UART_TypeDef* p = _port_to_uart(port_index);

        uint32_t nDrqDst = DRQDST_UART0TX;

        if (port_index < 3) {
            nDrqDst = port_index + 1 + DRQDST_UART0TX;
        }

        lli->cfg = DMA_CHAN_CFG_DST_IO_MODE | DMA_CHAN_CFG_SRC_LINEAR_MODE | DMA_CHAN_CFG_SRC_DRQ(DRQSRC_SDRAM) | DMA_CHAN_CFG_DST_DRQ(nDrqDst);
        lli->src = reinterpret_cast<uint32_t>(&s_pCoherentRegion->dmx_data[port_index][sv_nDmxDataReadIndex[port_index]].data[0]);
        lli->dst = reinterpret_cast<uint32_t>(&p->O00.THR);
        lli->len = s_pCoherentRegion->dmx_data[port_index][sv_nDmxDataReadIndex[port_index]].nLength;
        lli->para = DMA_NORMAL_WAIT;
        lli->p_lli_next = DMA_LLI_LAST_ITEM;
        //
        m_dmxPortDirection[port_index] = PortDirection::kInput;
        //
        sv_port_state[port_index] = PortState::IDLE;
        // RDM RX
        s_nRdmDataWriteIndex[port_index] = 0;
        s_nRdmDataReadIndex[port_index] = 0;
        s_pRdmDataCurrent[port_index] = &s_aRdmData[port_index][0];
        sv_PortReceiveState[port_index] = TxRxState::IDLE;
        // DMX RX
        s_nDmxDataBufferIndexHead[port_index] = 0;
        s_nDmxDataBufferIndexTail[port_index] = 0;
        s_nDmxDataIndex[port_index] = 0;
        sv_nDmxUpdatesPerSecond[port_index] = 0;
        sv_nDmxPackets[port_index] = 0;
        sv_nDmxPacketsPrevious[port_index] = 0;
    }

    SetDmxBreakTime(dmx::transmit::kBreakTimeTypical);
    SetDmxMabTime(dmx::transmit::kMabTimeMin);
    SetDmxSlots(dmx::kChannelsMax);

    H3GpioFsel(s_nDmxDataDirectionGpioPin[0], GPIO_FSEL_OUTPUT);
    H3GpioClr(s_nDmxDataDirectionGpioPin[0]); // 0 = input, 1 = output
    H3GpioFsel(s_nDmxDataDirectionGpioPin[1], GPIO_FSEL_OUTPUT);
    H3GpioClr(s_nDmxDataDirectionGpioPin[1]); // 0 = input, 1 = output
#if defined(ORANGE_PI_ONE)
    H3GpioFsel(s_nDmxDataDirectionGpioPin[2], GPIO_FSEL_OUTPUT);
    H3GpioClr(s_nDmxDataDirectionGpioPin[2]); // 0 = input, 1 = output
#ifndef DO_NOT_USE_UART0
    H3GpioFsel(s_nDmxDataDirectionGpioPin[3], GPIO_FSEL_OUTPUT);
    H3GpioClr(s_nDmxDataDirectionGpioPin[3]); // 0 = input, 1 = output
#endif
#endif

    UartConfig(1);
    UartConfig(2);
#if defined(ORANGE_PI_ONE)
    UartConfig(3);
#ifndef DO_NOT_USE_UART0
    UartConfig(0);
#endif
#endif

    __disable_fiq();

    arm_install_handler(reinterpret_cast<unsigned>(fiq_dmx_multi), ARM_VECTOR(ARM_VECTOR_FIQ));

    gic_fiq_config(H3_UART1_IRQn, GIC_CORE0);
    gic_fiq_config(H3_UART2_IRQn, GIC_CORE0);
#if defined(ORANGE_PI_ONE)
    gic_fiq_config(H3_UART3_IRQn, GIC_CORE0);
#ifndef DO_NOT_USE_UART0
    gic_fiq_config(H3_UART0_IRQn, GIC_CORE0);
#endif
#endif

    UartEnableFifoTx(0);
    UartEnableFifoTx(1);
#if defined(ORANGE_PI_ONE)
    UartEnableFifoTx(2);
#ifndef DO_NOT_USE_UART0
    UartEnableFifoTx(3);
#endif
#endif

    irq_handler_init();
    irq_timer_set(IRQ_TIMER_0, irq_timer0_dmx_multi_sender);
    irq_timer_set(IRQ_TIMER_1, IrqTimer1DmxReceive);

    H3_TIMER->TMR0_CTRL |= TIMER_CTRL_SINGLE_MODE;
    H3_TIMER->TMR0_INTV = 12000;                                      // Wait 1ms
    H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD); // 0x3;

    H3_TIMER->TMR1_INTV = 0xB71B00; // 1 second
    H3_TIMER->TMR1_CTRL &= ~(TIMER_CTRL_SINGLE_MODE);
    H3_TIMER->TMR1_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);

    H3_CCU->BUS_SOFT_RESET0 |= CCU_BUS_SOFT_RESET0_DMA;
    H3_CCU->BUS_CLK_GATING0 |= CCU_BUS_CLK_GATING0_DMA;

#if 0
	H3_DMA->IRQ_PEND0 |= H3_DMA->IRQ_PEND0;
	H3_DMA->IRQ_PEND1 |= H3_DMA->IRQ_PEND1;

	H3_DMA->IRQ_EN0 = DMA_IRQ_EN0_DMA0_PKG_IRQ_EN | DMA_IRQ_EN0_DMA1_PKG_IRQ_EN
			| DMA_IRQ_EN0_DMA2_PKG_IRQ_EN | DMA_IRQ_EN0_DMA3_PKG_IRQ_EN;
#endif

    __ISB();
    __enable_fiq();

    DEBUG_EXIT();
}

void Dmx::SetPortDirection(uint32_t port_index, PortDirection port_direction, bool enable_data) {
    DEBUG_PRINTF("port_index=%u, port_direction=%u, enable_data=%u", port_index, port_direction, enable_data);
    assert(port_index < config::max::PORTS);

    const auto kUart = _port_to_uart(port_index);

    if (m_dmxPortDirection[port_index] != port_direction) {
		StopData(kUart, port_index);
		
        m_dmxPortDirection[port_index] = port_direction;
  
        switch (port_direction) {
            case PortDirection::kOutput:
                H3GpioSet(s_nDmxDataDirectionGpioPin[port_index]); // 0 = input, 1 = output
                break;
            case PortDirection::kInput:
                H3GpioClr(s_nDmxDataDirectionGpioPin[port_index]); // 0 = input, 1 = output
                break;
            default:
                assert(0);
                __builtin_unreachable();
                break;
        }
    } else if (!enable_data) {
        StopData(kUart, port_index);
    }

    if (enable_data) {
        StartData(kUart, port_index);
    }
}

void Dmx::ClearData(uint32_t port_index) {
    for (uint32_t j = 0; j < DMX_DATA_OUT_INDEX; j++) {
        auto* p = &s_pCoherentRegion->dmx_data[port_index][j];
        auto* p32 = reinterpret_cast<uint32_t*>(p->data);

        for (uint32_t i = 0; i < buffer::SIZE / 4; i++) {
            *p32++ = 0;
        }

        p->nLength = 513; // Including START Code
    }
}

volatile dmx::TotalStatistics& Dmx::GetTotalStatistics(uint32_t port_index) {
    return sv_TotalStatistics[port_index];
}

void Dmx::StartDmxOutput([[maybe_unused]] uint32_t port_index) {
    // Nothing to do here
}

void Dmx::StartOutput([[maybe_unused]] uint32_t port_index) {
    // Nothing to do here
}

void Dmx::Sync() {
    // Nothing to do here
}

void Dmx::StartData(H3_UART_TypeDef* uart, uint32_t port_index) {
    assert(sv_port_state[port_index] == PortState::IDLE);

    switch (m_dmxPortDirection[port_index]) {
        case PortDirection::kOutput:
            UartEnableFifoTx(port_index);
            sv_port_state[port_index] = PortState::TX;
            __DMB();
            break;
        case PortDirection::kInput: {
            if (uart != nullptr) {
                while (!(uart->USR & UART_USR_TFE));
            }

            UartEnableFifoRx(port_index);
            sv_PortReceiveState[port_index] = TxRxState::IDLE;
            sv_port_state[port_index] = PortState::RX;
            __DMB();
            break;
        }
        default:
            assert(0);
            __builtin_unreachable();
            break;
    }
}

void Dmx::StopData(H3_UART_TypeDef* uart, uint32_t port_index) {
    assert(pUart != nullptr);
    assert(port_index < config::max::PORTS);

    __DMB();
    if (sv_port_state[port_index] == PortState::IDLE) {
        return;
    }

    if (m_dmxPortDirection[port_index] == PortDirection::kOutput) {
        auto is_idle = false;

        do {
            __DMB();
            if (sv_DmxSendState == TxRxState::DMXINTER) {
                while (!(uart->USR & UART_USR_TFE));
                is_idle = true;
            }
        } while (!is_idle);
    } else if (m_dmxPortDirection[port_index] == PortDirection::kInput) {
        uart->O08.FCR = 0;
        uart->O04.IER = 0;

        sv_PortReceiveState[port_index] = TxRxState::IDLE;
    }

    sv_port_state[port_index] = PortState::IDLE;
    __DMB();
}

// DMX Send

void Dmx::SetDmxBreakTime(uint32_t break_time) {
    DEBUG_PRINTF("break_time=%u", break_time);

    m_nDmxTransmitBreakTime = std::max(transmit::kBreakTimeMin, break_time);
    s_nDmxTransmistBreakTimeINTV = m_nDmxTransmitBreakTime * 12;
    //
    SetDmxPeriodTime(m_nDmxTransmitPeriodRequested);
}

void Dmx::SetDmxMabTime(uint32_t mab_time) {
    DEBUG_PRINTF("mab_time=%u", mab_time);

    m_nDmxTransmitMabTime = std::min(std::max(transmit::kMabTimeMin, mab_time), transmit::kMabTimeMax);
    s_nDmxTransmitMabTimeINTV = m_nDmxTransmitMabTime * 12;
    //
    SetDmxPeriodTime(m_nDmxTransmitPeriodRequested);
}

void Dmx::SetDmxPeriodTime(uint32_t period_time) {
    DEBUG_ENTRY();

    m_nDmxTransmitPeriodRequested = period_time;

    auto length_max = m_nDmxTransmissionLength[0];

    DEBUG_PRINTF("period_time=%u, length_max=%u", period_time, length_max);

    for (uint32_t i = 1; i < config::max::PORTS; i++) {
        if (m_nDmxTransmissionLength[i] > length_max) {
            length_max = m_nDmxTransmissionLength[i];
        }
    }

    DEBUG_PRINTF("nLengthMax=%u", length_max);

    const auto kPackageLengthMicroSeconds = m_nDmxTransmitBreakTime + m_nDmxTransmitMabTime + (length_max * 44) + 44;

    if (period_time != 0) {
        if (period_time < kPackageLengthMicroSeconds) {
            m_nDmxTransmitPeriod = std::max(transmit::kBreakToBreakTimeMin, kPackageLengthMicroSeconds + 44);
        } else {
            m_nDmxTransmitPeriod = period_time;
        }
    } else {
        m_nDmxTransmitPeriod = std::max(transmit::kBreakToBreakTimeMin, kPackageLengthMicroSeconds + 44);
    }

    s_nDmxTransmitPeriodINTV = (m_nDmxTransmitPeriod * 12) - s_nDmxTransmistBreakTimeINTV - s_nDmxTransmitMabTimeINTV;

    DEBUG_PRINTF("period_time=%u, length_max=%u, m_nDmxTransmitPeriod=%u", period_time, length_max, m_nDmxTransmitPeriod);
    DEBUG_EXIT();
}

void Dmx::SetDmxSlots(uint16_t slots) {
    DEBUG_ENTRY();
    DEBUG_PRINTF("slots=%u", slots);

    if ((slots >= 2) && (slots <= dmx::kChannelsMax)) {
        m_nDmxTransmitSlots = slots;

        for (uint32_t i = 0; i < config::max::PORTS; i++) {
            m_nDmxTransmissionLength[i] = static_cast<uint32_t>(slots);
        }

        SetDmxPeriodTime(m_nDmxTransmitPeriodRequested);
    }

    DEBUG_EXIT();
}

void Dmx::SetOutputStyle([[maybe_unused]] uint32_t port_index, [[maybe_unused]] dmx::OutputStyle output_style) {
    DEBUG_PUTS("Not supported.");
}

dmx::OutputStyle Dmx::GetOutputStyle([[maybe_unused]] uint32_t port_index) const {
    return dmx::OutputStyle::kConstant;
}

template <uint32_t port_index> void Dmx::SetSendDataInternal(const uint8_t* data, uint32_t length) {
    assert(data != nullptr);
    assert(length != 0);

    const auto kNext = (sv_nDmxDataWriteIndex[port_index] + 1) & (DMX_DATA_OUT_INDEX - 1);
    auto* p = &s_pCoherentRegion->dmx_data[port_index][kNext];

    auto* dst = p->data;
    length = std::min(length, static_cast<uint32_t>(m_nDmxTransmitSlots));
    p->nLength = length + 1U;

    __builtin_prefetch(data);
    memcpy(&dst[1], data, length);

    if (length != m_nDmxTransmissionLength[port_index]) {
        m_nDmxTransmissionLength[port_index] = length;
        SetDmxPeriodTime(m_nDmxTransmitPeriodRequested);
    }

    sv_nDmxDataWriteIndex[port_index] = kNext;
}

void Dmx::Blackout() {
    DEBUG_ENTRY();

    for (uint32_t port_index = 0; port_index < config::max::PORTS; port_index++) {
        if (sv_port_state[port_index] != PortState::TX) {
            continue;
        }

        const auto nNext = (sv_nDmxDataWriteIndex[port_index] + 1) & (DMX_DATA_OUT_INDEX - 1);
        auto* p = &s_pCoherentRegion->dmx_data[port_index][nNext];

        auto* p32 = reinterpret_cast<uint32_t*>(p->data);

        for (uint32_t i = 0; i < buffer::SIZE / 4; i++) {
            *p32++ = 0;
        }

        p->data[0] = dmx::kStartCode;

        sv_nDmxDataWriteIndex[port_index] = nNext;
    }

    DEBUG_EXIT();
}

void Dmx::FullOn() {
    DEBUG_ENTRY();

    for (uint32_t port_index = 0; port_index < config::max::PORTS; port_index++) {
        if (sv_port_state[port_index] != PortState::TX) {
            continue;
        }

        const auto kNext = (sv_nDmxDataWriteIndex[port_index] + 1) & (DMX_DATA_OUT_INDEX - 1);
        auto* p = &s_pCoherentRegion->dmx_data[port_index][kNext];

        auto* p32 = reinterpret_cast<uint32_t*>(p->data);

        for (uint32_t i = 0; i < buffer::SIZE / 4; i++) {
            *p32++ = static_cast<uint32_t>(~0);
        }

        p->data[0] = dmx::kStartCode;

        sv_nDmxDataWriteIndex[port_index] = kNext;
    }

    DEBUG_EXIT();
}

// DMX Receive

const uint8_t* Dmx::GetDmxChanged(uint32_t port_index) {
    const auto* p = GetDmxAvailable(port_index);

    if (p == nullptr) {
        return nullptr;
    }

    const auto* src16 = reinterpret_cast<const uint16_t*>(p);
    auto* dst16 = reinterpret_cast<uint16_t*>(&s_RxDmxPrevious[port_index][0]);

    auto is_changed = false;

    for (auto i = 0; i < buffer::SIZE / 2; i++) {
        if (*dst16 != *src16) {
            *dst16 = *src16;
            is_changed = true;
        }
        dst16++;
        src16++;
    }

    return (is_changed ? p : nullptr);
}

const uint8_t* Dmx::GetDmxCurrentData(uint32_t port_index) {
    __DMB();
    return const_cast<const uint8_t*>(s_aDmxData[port_index][s_nDmxDataBufferIndexTail[port_index]].Data);
}

const uint8_t* Dmx::GetDmxAvailable(uint32_t port_index) {
    __DMB();

    if (s_nDmxDataBufferIndexHead[port_index] == s_nDmxDataBufferIndexTail[port_index]) {
        return nullptr;
    } else {
        const auto* p = const_cast<const uint8_t*>(s_aDmxData[port_index][s_nDmxDataBufferIndexTail[port_index]].Data);
        s_nDmxDataBufferIndexTail[port_index] = (s_nDmxDataBufferIndexTail[port_index] + 1) & buffer::INDEX_MASK;
        sv_TotalStatistics[port_index].dmx.received++;
        return p;
    }
}

uint32_t Dmx::GetDmxUpdatesPerSecond(uint32_t port_index) {
    __DMB();
    return sv_nDmxUpdatesPerSecond[port_index];
}

// RDM Send

void Dmx::RdmSendRaw(uint32_t port_index, const uint8_t* pRdmData, uint32_t nLength) {
    assert(port_index < config::max::PORTS);
    assert(pRdmData != nullptr);
    assert(nLength != 0);

    auto* p = _port_to_uart(port_index);
    assert(p != nullptr);

    while (!(p->LSR & UART_LSR_TEMT));

    p->LCR = UART_LCR_8_N_2 | UART_LCR_BC;
    udelay(RDM_TRANSMIT_BREAK_TIME);

    p->LCR = UART_LCR_8_N_2;
    udelay(RDM_TRANSMIT_MAB_TIME);

    for (uint32_t i = 0; i < nLength; i++) {
        while (!(p->LSR & UART_LSR_THRE));
        p->O00.THR = pRdmData[i];
    }

    while ((p->USR & UART_USR_BUSY) == UART_USR_BUSY) {
        static_cast<void>(EXT_UART->O00.RBR);
    }

    sv_TotalStatistics[port_index].rdm.sent.classes++;
}

void Dmx::RdmSendDiscoveryRespondMessage(uint32_t port_index, const uint8_t* pRdmData, uint32_t nLength) {
    DEBUG_PRINTF("port_index=%u, pRdmData=%p, nLength=%u", port_index, pRdmData, nLength);
    assert(port_index < dmx::config::max::PORTS);
    assert(pRdmData != nullptr);
    assert(nLength != 0);

    // 3.2.2 Responder Packet spacing
    udelay(RDM_RESPONDER_PACKET_SPACING, gsv_RdmDataReceiveEnd);

    SetPortDirection(port_index, dmx::PortDirection::kOutput, false);

    auto* p = _port_to_uart(port_index);

    p->LCR = UART_LCR_8_N_2;

    for (uint32_t i = 0; i < nLength; i++) {
        while (!(p->LSR & UART_LSR_THRE));
        p->O00.THR = pRdmData[i];
    }

    while (!((p->LSR & UART_LSR_TEMT) == UART_LSR_TEMT));

    udelay(RDM_RESPONDER_DATA_DIRECTION_DELAY);

    SetPortDirection(port_index, dmx::PortDirection::kInput, true);

    sv_TotalStatistics[port_index].rdm.sent.discovery_response++;

    DEBUG_EXIT();
}

// RDM Receive

const uint8_t* Dmx::RdmReceive(uint32_t port_index) {
    assert(port_index < config::max::PORTS);

    __DMB();

    if (s_nRdmDataWriteIndex[port_index] == s_nRdmDataReadIndex[port_index]) {
        return nullptr;
    } else {
        const auto* p = &s_aRdmData[port_index][s_nRdmDataReadIndex[port_index]].data[0];
        s_nRdmDataReadIndex[port_index] = (s_nRdmDataReadIndex[port_index] + 1) & RDM_DATA_BUFFER_INDEX_MASK;

        if (p[0] == E120_SC_RDM) {
            const auto* pRdmCommand = reinterpret_cast<const struct TRdmMessage*>(p);

            uint32_t i;
            uint16_t nChecksum = 0;

            for (i = 0; i < 24; i++) {
                nChecksum = static_cast<uint16_t>(nChecksum + p[i]);
            }

            for (; i < pRdmCommand->message_length; i++) {
                nChecksum = static_cast<uint16_t>(nChecksum + p[i]);
            }

            if (p[i++] == static_cast<uint8_t>(nChecksum >> 8)) {
                if (p[i] == static_cast<uint8_t>(nChecksum)) {
                    sv_TotalStatistics[port_index].rdm.received.good++;
                    return p;
                }
            }

            sv_TotalStatistics[port_index].rdm.received.bad++;
            return nullptr;
        } else {
            sv_TotalStatistics[port_index].rdm.received.discovery_response++;
        }

        return p;
    }
}

const uint8_t* Dmx::RdmReceiveTimeOut(uint32_t port_index, uint16_t nTimeOut) {
    assert(port_index < config::max::PORTS);

    uint8_t* p = nullptr;
    const auto kMicros = H3_TIMER->AVS_CNT1;

    do {
        if ((p = const_cast<uint8_t*>(RdmReceive(port_index))) != nullptr) {
            return p;
        }
    } while ((H3_TIMER->AVS_CNT1 - kMicros) < nTimeOut);

    return p;
}

// Explicit template instantiations
template void Dmx::SetSendDataWithoutSC<dmx::SendStyle::kDirect>(const uint32_t, const uint8_t*, uint32_t);
template void Dmx::SetSendDataWithoutSC<dmx::SendStyle::kSync>(const uint32_t, const uint8_t*, uint32_t);

template void Dmx::SetSendDataInternal<0>(const uint8_t*, uint32_t);

#if DMX_MAX_PORTS >= 2
template void Dmx::SetSendDataInternal<1>(const uint8_t*, uint32_t);
#endif

#if DMX_MAX_PORTS >= 3
template void Dmx::SetSendDataInternal<2>(const uint8_t*, uint32_t);
#endif

#if DMX_MAX_PORTS == 4
template void Dmx::SetSendDataInternal<3>(const uint8_t*, uint32_t);
#endif
