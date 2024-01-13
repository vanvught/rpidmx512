/**
 * @file sc16is7x0.h
 *
 */
/* Copyright (C) 2020-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef SC16IS7X0_H_
#define SC16IS7X0_H_

static constexpr uint32_t SC16IS7X0_DEFAULT_BAUDRATE = 115200;

#define SC16IS7X0_REG_SHIFT		3

static constexpr uint8_t SC16IS7X0_RHR = 0x00 << SC16IS7X0_REG_SHIFT;	///< Receive Holding Register - Read only
static constexpr uint8_t SC16IS7X0_THR = 0x00 << SC16IS7X0_REG_SHIFT;
static constexpr uint8_t SC16IS7X0_IER = 0x01 << SC16IS7X0_REG_SHIFT;
static constexpr uint8_t SC16IS7X0_FCR = 0x02 << SC16IS7X0_REG_SHIFT;
static constexpr uint8_t SC16IS7X0_IIR = 0x02 << SC16IS7X0_REG_SHIFT;
static constexpr uint8_t SC16IS7X0_LCR = 0x03 << SC16IS7X0_REG_SHIFT;
static constexpr uint8_t SC16IS7X0_MCR = 0x04 << SC16IS7X0_REG_SHIFT;
static constexpr uint8_t SC16IS7X0_LSR = 0x05 << SC16IS7X0_REG_SHIFT;	///< Line status register - Read only
static constexpr uint8_t SC16IS7X0_MSR = 0x06 << SC16IS7X0_REG_SHIFT;
static constexpr uint8_t SC16IS7X0_SPR = 0x07 << SC16IS7X0_REG_SHIFT;
static constexpr uint8_t SC16IS7X0_TCR = 0x06 << SC16IS7X0_REG_SHIFT;
static constexpr uint8_t SC16IS7X0_TLR = 0x07 << SC16IS7X0_REG_SHIFT;
static constexpr uint8_t SC16IS7X0_TXLVL = 0x08 << SC16IS7X0_REG_SHIFT;
static constexpr uint8_t SC16IS7X0_RXLVL = 0x09 << SC16IS7X0_REG_SHIFT;
static constexpr uint8_t SC16IS7X0_IODIR = 0x0A << SC16IS7X0_REG_SHIFT;
static constexpr uint8_t SC16IS7X0_IOSTATE = 0x0B << SC16IS7X0_REG_SHIFT;
static constexpr uint8_t SC16IS7X0_IOINTENA = 0x0C << SC16IS7X0_REG_SHIFT;
static constexpr uint8_t SC16IS7X0_IOCONTROL = 0x0E << SC16IS7X0_REG_SHIFT;
static constexpr uint8_t SC16IS7X0_EFCR = 0x0F << SC16IS7X0_REG_SHIFT;

static constexpr uint8_t SC16IS7X0_DLL = 0x00 << SC16IS7X0_REG_SHIFT;
static constexpr uint8_t SC16IS7X0_DLH = 0X01 << SC16IS7X0_REG_SHIFT;
static constexpr uint8_t SC16IS7X0_EFR = 0X02 << SC16IS7X0_REG_SHIFT;

/** See section 8.3 of the datasheet for definitions
 * of bits in the FIFO Control Register (FCR)
 */
static constexpr uint8_t FCR_RX_IRQ_60 = (3u << 6);
static constexpr uint8_t FCR_RX_IRQ_56 = (2u << 6);
static constexpr uint8_t FCR_RX_IRQ_16 = (1u << 6);
static constexpr uint8_t FCR_RX_IRQ_8 = (0u << 6);
//TX Level only accessible when EFR[4] is set
static constexpr uint8_t FCR_TX_IRQ_56 = (3u << 4);
static constexpr uint8_t FCR_TX_IRQ_32 = (2u << 4);
static constexpr uint8_t FCR_TX_IRQ_16 = (1u << 4);
static constexpr uint8_t FCR_TX_IRQ_8 = (0u << 4);
//static constexpr uint8_t FCR_RESERVED                (1 << 3);
static constexpr uint8_t FCR_TX_FIFO_RST = (1u << 2);
static constexpr uint8_t FCR_RX_FIFO_RST = (1u << 1);
static constexpr uint8_t FCR_ENABLE_FIFO = (1u << 0);

/** See section 8.4 of the datasheet for definitions
 * of bits in the Line Control Register (LCR)
 */
static constexpr uint8_t LCR_BITS5 = 0x00;
static constexpr uint8_t LCR_BITS6 = 0x01;
static constexpr uint8_t LCR_BITS7 = 0x02;
static constexpr uint8_t LCR_BITS8 = 0x03;

static constexpr uint8_t LCR_BITS1 = 0x00;
static constexpr uint8_t LCR_BITS2 = 0x04;

static constexpr uint8_t LCR_NONE = 0x00;
static constexpr uint8_t LCR_ODD = 0x08;
static constexpr uint8_t LCR_EVEN = 0x18;
static constexpr uint8_t LCR_FORCED1 = 0x28;
static constexpr uint8_t LCR_FORCED0 = 0x38;

static constexpr uint8_t LCR_BRK_ENA = 0x40;
static constexpr uint8_t LCR_BRK_DIS = 0x00;

static constexpr uint8_t LCR_ENABLE_DIV = 0x80;
static constexpr uint8_t LCR_DISABLE_DIV = 0x00;

/** See section 8.5 of the datasheet for definitions
 * of bits in the Line status register (LSR)
 */
static constexpr uint8_t LSR_DR = (0x01); ///< Data ready in RX FIFO
static constexpr uint8_t LSR_OE = (0x02); ///< Overrun error
static constexpr uint8_t LSR_PE = (0x04); ///< Parity error
static constexpr uint8_t LSR_FE = (0x08); ///< Framing error
static constexpr uint8_t LSR_BI = (0x10); ///< Break interrupt
static constexpr uint8_t LSR_THRE = (0x20); ///< Transmitter holding register (FIFO empty)
static constexpr uint8_t LSR_TEMT = (0x40); ///< Transmitter empty (FIFO and TSR both empty)
static constexpr uint8_t LSR_FFE = (0x80); ///< At least one PE, FE or BI in FIFO

/**
 * 8.6 Modem Control Register (MCR)
 */
//MCR[2] only accessible when EFR[4] is set
static constexpr uint8_t MCR_ENABLE_TCR_TLR = (1u << 2);
static constexpr uint8_t MCR_PRESCALE_4 = (1u << 7);

/** See section 8.8 of the datasheet for definitions
 * of bits in the Interrupt enable register (IER)
 */
static constexpr uint8_t IER_ERHRI = (0x01); /* Enable received data available interrupt            */
static constexpr uint8_t IER_ETHRI = (0x02); /* Enable transmitter holding register empty interrupt */
static constexpr uint8_t IER_ELSI = (0x04); /* Enable receiver line status interrupt               */
static constexpr uint8_t IER_EMSI = (0x08); /* Enable modem status interrupt                       */
//IER[7:5] only accessible when EFR[4] is set
static constexpr uint8_t IER_SLEEP = (0x10); /* Enable sleep mode                                   */
static constexpr uint8_t IER_XOFFI = (0x20); /* Enable XOFF interrupt                               */
static constexpr uint8_t IER_RTSI = (0x40); /* Enable RTS interrupt                                */
static constexpr uint8_t IER_CTSI = (0x80); /* Enable CTS interrupt                                */

/**
 * 8.11 Enhanced Features Register (EFR)
 */
static constexpr uint8_t EFR_ENABLE_ENHANCED_FUNCTIONS = (1u << 4);

#endif /* SC16IS7X0_H_ */
