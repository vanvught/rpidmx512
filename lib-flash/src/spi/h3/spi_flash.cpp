/**
 * @file spi_flash.cpp
 *
 */
/* Copyright (C) 2018-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast" // FIXME ignored "-Wold-style-cast"

#include <cstdint>
#ifndef NDEBUG
#include <cstdio>
#endif
#include <cassert>

#include "../spi_flash_internal.h"
#include "h3.h"
#include "h3_spi.h"
#include "h3_spi_internal.h"
#include "h3_gpio.h"
#include "h3_ccu.h"
 #include "firmware/debug/debug_debug.h"

struct Spi0Status
{
    bool transfer_active;
    uint8_t* rxbuf;
    uint32_t rxcnt;
    uint8_t* txbuf;
    uint32_t txcnt;
    uint32_t txlen;
    uint32_t rxlen;
};

static struct Spi0Status s_spi0_status;

static void Spi0SetChipSelect()
{
    uint32_t value = H3_SPI0->TC;

    value |= TC_SS_OWNER; // Software controlled

    H3_SPI0->TC = value;
}

static void Spi0SetDataMode(h3_spi_mode_t mode)
{
    uint32_t value = H3_SPI0->TC;

    value &= static_cast<uint32_t>(~TC_CPHA);
    value &= static_cast<uint32_t>(~TC_CPOL);
    value |= (mode & 0x3);

    H3_SPI0->TC = value;
}

static void Spi0Begin()
{
    H3GpioFsel(H3_PORT_TO_GPIO(H3_GPIO_PORTC, 3), H3_PC3_SELECT_SPI0_CS);
    H3GpioFsel(H3_PORT_TO_GPIO(H3_GPIO_PORTC, 2), H3_PC2_SELECT_SPI0_CLK);
    H3GpioFsel(H3_PORT_TO_GPIO(H3_GPIO_PORTC, 1), H3_PC1_SELECT_SPI0_MISO);
    H3GpioFsel(H3_PORT_TO_GPIO(H3_GPIO_PORTC, 0), H3_PC0_SELECT_SPI0_MOSI);

    H3_CCU->BUS_SOFT_RESET0 |= CCU_BUS_SOFT_RESET0_SPI0;
    udelay(1000); // 1ms
    H3_CCU->BUS_CLK_GATING0 &= ~CCU_BUS_CLK_GATING0_SPI0;
    udelay(1000); // 1ms
    H3_CCU->BUS_CLK_GATING0 |= CCU_BUS_CLK_GATING0_SPI0;
    H3_CCU->SPI0_CLK = (1U << 31) | (0x01 << 24); // Clock is ON, P0

    uint32_t value;

    value = H3_SPI0->GC;
    value |= GC_SRST;
    H3_SPI0->GC = value;

    value = H3_SPI0->GC;
    value |= GC_MODE_MASTER;
    value |= GC_EN;
    H3_SPI0->GC = value;

    H3_SPI0->IE = 0; // Disable interrupts

#ifndef NDEBUG
    const auto kPllFrequency = h3_ccu_get_pll_rate(CCU_PLL_PERIPH0);
    printf("pll_frequency=%u\n", kPllFrequency);
    assert(CCU_PERIPH0_CLOCK_HZ == kPllFrequency);
#endif
}

inline static uint32_t QueryTxfifo()
{
    uint32_t value = H3_SPI0->FS & FS_TX_CNT;
    value >>= FS_TXCNT_BIT_POS;
    return value;
}

inline static uint32_t QueryRxfifo()
{
    uint32_t value = H3_SPI0->FS & FS_RX_CNT;
    value >>= FS_RXCNT_BIT_POS;
    return value;
}

inline static void ClearFifos()
{
    H3_SPI0->FC = (FC_RX_RST | FC_TX_RST);

    int timeout;
    // TODO Do we really need to check?
    for (timeout = 1000; timeout > 0; timeout--)
    { // TODO What if failed?
        if (H3_SPI0->FC == 0) break;
    }

    assert(H3_SPI0->FC == 0);

    // TODO Do we need to set the trigger level of RxFIFO/TxFIFO?
}

static void ReadRxfifo()
{
    if (s_spi0_status.rxcnt == s_spi0_status.rxlen)
    {
        return;
    }

    uint32_t rx_count = QueryRxfifo();

    while (rx_count-- > 0)
    {
        const uint8_t kValue = H3_SPI0->RX.byte;

        if (s_spi0_status.rxcnt < s_spi0_status.rxlen) s_spi0_status.rxbuf[s_spi0_status.rxcnt++] = kValue;
    }
}

static void WriteTxfifo()
{
    if (s_spi0_status.txcnt == s_spi0_status.txlen)
    {
        return;
    }

    uint32_t tx_count = SPI_FIFO_SIZE - QueryTxfifo();

    while (tx_count-- > 0)
    {
        H3_SPI0->TX.byte = s_spi0_status.txbuf[s_spi0_status.txcnt++];

        if (s_spi0_status.txcnt == s_spi0_status.txlen)
        {
            break;
        }
    }
}

static void InterruptHandler()
{
    uint32_t intr = H3_SPI0->IS;

    if (intr & IS_RX_FULL)
    {
        ReadRxfifo();
    }

    if (intr & IS_TX_EMP)
    {
        WriteTxfifo();

        if (s_spi0_status.txcnt == s_spi0_status.txlen)
        {
            H3_SPI0->IE = IE_TC | IE_RX_FULL;
        }
    }

    if (intr & IS_TC)
    {
        ReadRxfifo();

        H3_SPI0->IE = 0;
        s_spi0_status.transfer_active = false;
    }

    H3_SPI0->IS = intr;
}

static void Spi0Writenb(const char* tx_buffer, uint32_t data_length)
{
    assert(tx_buffer != 0);

    H3_SPI0->GC &= static_cast<uint32_t>(~(GC_TP_EN)); // ignore RXFIFO

    ClearFifos();

    H3_SPI0->MBC = data_length;
    H3_SPI0->MTC = data_length;
    H3_SPI0->BCC = data_length;

    uint32_t fifo_writes = 0;

    uint32_t tx_count1 = SPI_FIFO_SIZE - QueryTxfifo();

    while ((fifo_writes < data_length) && (tx_count1-- > 0))
    {
        H3_SPI0->TX.byte = tx_buffer[fifo_writes];
        fifo_writes++;
    }

    H3_SPI0->TC |= TC_XCH;
    H3_SPI0->IE = IE_TX_EMP | IE_TC;

    while ((H3_SPI0->IS & IS_TX_EMP) != IS_TX_EMP);

    H3_SPI0->IS = IS_TX_EMP;

    while (fifo_writes < data_length)
    {
        uint32_t tx_count = SPI_FIFO_SIZE - QueryTxfifo();

        while ((fifo_writes < data_length) && (tx_count-- > 0))
        {
            H3_SPI0->TX.byte = tx_buffer[fifo_writes];
            fifo_writes++;
        }

        while ((H3_SPI0->IS & IS_TX_EMP) != IS_TX_EMP);

        H3_SPI0->IS = IS_TX_EMP;
    }

    while ((H3_SPI0->IS & IS_TC) != IS_TC);

    uint32_t value = H3_SPI0->IS;
    H3_SPI0->IS = value;
    H3_SPI0->IE = 0;
}

static void Spi0Transfernb(char* tx_buffer, /*@null@*/ char* rx_buffer, uint32_t data_length)
{
    s_spi0_status.rxbuf = reinterpret_cast<uint8_t*>(rx_buffer);
    s_spi0_status.rxcnt = 0;
    s_spi0_status.txbuf = reinterpret_cast<uint8_t*>(tx_buffer);
    s_spi0_status.txcnt = 0;
    s_spi0_status.txlen = data_length;
    s_spi0_status.rxlen = data_length;

    H3_SPI0->GC |= GC_TP_EN;

    ClearFifos();

    H3_SPI0->MBC = data_length;
    H3_SPI0->MTC = data_length;
    H3_SPI0->BCC = data_length;

    WriteTxfifo();

    H3_SPI0->TC |= TC_XCH;
    H3_SPI0->IE = IE_TX_EMP | IE_RX_FULL | IE_TC;

    s_spi0_status.transfer_active = true;

    while (s_spi0_status.transfer_active)
    {
        InterruptHandler();
    }
}

static void Spi0Transfern(char* buffer, uint32_t data_length)
{
    assert(buffer != 0);

    Spi0Transfernb(buffer, buffer, data_length);
}

static void Spi0SetupClock(uint32_t pll_clock, uint32_t spi_clock)
{
    // We can use CDR2, which is calculated with the formula: SPI_CLK = CCU_PERIPH0_CLOCK_HZ / (2 * (cdr + 1))
    uint32_t cdr = pll_clock / (2 * spi_clock);
    assert(cdr <= (0xFF + 1));

    if (cdr > 0)
    {
        cdr--;
    }

    uint32_t value = H3_SPI0->CC;
    value &= static_cast<uint32_t>(~(CC_DRS | (CC_CDR2_MASK << CC_CDR2_SHIFT)));
    value |= (CC_DRS | ((cdr & CC_CDR2_MASK) << CC_CDR2_SHIFT));
    H3_SPI0->CC = value;

#ifndef NDEBUG
    printf("H3_SPI0->CC = %p\n", H3_SPI0->CC);
#endif
}

void SpiInit()
{
    Spi0Begin();

    Spi0SetChipSelect(); // H3_SPI_CS_NONE
    Spi0SetDataMode(H3_SPI_MODE0);
    Spi0SetupClock(CCU_PERIPH0_CLOCK_HZ, SPI_XFER_SPEED_HZ);

    H3GpioFsel(H3_PORT_TO_GPIO(H3_GPIO_PORTC, 3), GPIO_FSEL_OUTPUT);
    H3GpioSet(H3_PORT_TO_GPIO(H3_GPIO_PORTC, 3));
}

void SpiXfer(uint32_t length, const uint8_t* pOut, uint8_t* pIn, uint32_t flags)
{
    if (flags & SPI_XFER_BEGIN)
    {
        H3GpioClr(H3_PORT_TO_GPIO(H3_GPIO_PORTC, 3));
    }

    if (length != 0)
    {
        if (pIn == nullptr)
        {
            Spi0Writenb((char*)pOut, length);
        }
        else if (pOut == nullptr)
        {
            Spi0Transfern(reinterpret_cast<char*>(pIn), length);
        }
        else
        {
            Spi0Transfernb((char*)pOut, reinterpret_cast<char*>(pIn), length);
        }
    }

    if (flags & SPI_XFER_END)
    {
        H3GpioSet(H3_PORT_TO_GPIO(H3_GPIO_PORTC, 3));
    }
}
