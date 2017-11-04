/**
 * @file sc16is7x0.h
 *
 */
/* Copyright (C) 2016-2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#define SC16IS7X0_OK						0
#define SC16IS7X0_ERROR						1

#define SC16IS7X0_SPI_SPEED_MAX_HZ			4000000	///< 4 MHz
#define SC16IS7X0_SPI_SPEED_DEFAULT_HZ	 	1000000	///< 1 MHz

//Default baudrate
#define SC16IS7X0_DEFAULT_BAUDRATE			115200

#define SC16IS7X0_RHR        (0x00)	///< Receive Holding Register - Read only
#define SC16IS7X0_THR        (0X00)
#define SC16IS7X0_IER        (0X01)
#define SC16IS7X0_FCR        (0X02)
#define SC16IS7X0_IIR        (0X02)
#define SC16IS7X0_LCR        (0X03)
#define SC16IS7X0_MCR        (0X04)
#define SC16IS7X0_LSR        (0X05)	///< Line status register - Read only
#define SC16IS7X0_MSR        (0X06)
#define SC16IS7X0_SPR        (0X07)
#define SC16IS7X0_TCR        (0X06)
#define SC16IS7X0_TLR        (0X07)
#define SC16IS7X0_TXLVL      (0X08)
#define SC16IS7X0_RXLVL      (0X09)
#define SC16IS7X0_IODIR      (0X0A)
#define SC16IS7X0_IOSTATE    (0X0B)
#define SC16IS7X0_IOINTENA   (0X0C)
#define SC16IS7X0_IOCONTROL  (0X0E)
#define SC16IS7X0_EFCR       (0X0F)

#define SC16IS7X0_DLL        (0x00)
#define SC16IS7X0_DLH        (0X01)

//FIFO size
#define SC16IS7X0_FIFO_RX    64
#define SC16IS7X0_FIFO_TX    64


/** See section 8.3 of the datasheet for definitions
  * of bits in the FIFO Control Register (FCR)
  */
#define FCR_RX_IRQ_60                 (3 << 6)
#define FCR_RX_IRQ_56                 (2 << 6)
#define FCR_RX_IRQ_16                 (1 << 6)
#define FCR_RX_IRQ_8                  (0 << 6)
//TX Level only accessible when EFR[4] is set
#define FCR_TX_IRQ_56                 (3 << 4)
#define FCR_TX_IRQ_32                 (2 << 4)
#define FCR_TX_IRQ_16                 (1 << 4)
#define FCR_TX_IRQ_8                  (0 << 4)
//#define FCR_RESERVED                (1 << 3)
#define FCR_TX_FIFO_RST               (1 << 2)
#define FCR_RX_FIFO_RST               (1 << 1)
#define FCR_ENABLE_FIFO               (1 << 0)

/** See section 8.4 of the datasheet for definitions
  * of bits in the Line Control Register (LCR)
  */
#define LCR_BITS5                      0x00
#define LCR_BITS6                      0x01
#define LCR_BITS7                      0x02
#define LCR_BITS8                      0x03

#define LCR_BITS1                      0x00
#define LCR_BITS2                      0x04

#define LCR_NONE                       0x00
#define LCR_ODD                        0x08
#define LCR_EVEN                       0x18
#define LCR_FORCED1                    0x28
#define LCR_FORCED0                    0x38

#define LCR_BRK_ENA                    0x40
#define LCR_BRK_DIS                    0x00

#define LCR_ENABLE_DIV                 0x80
#define LCR_DISABLE_DIV                0x00

/** See section 8.5 of the datasheet for definitions
  * of bits in the Line status register (LSR)
  */
#define LSR_DR   						(0x01) ///< Data ready in RX FIFO
#define LSR_OE   						(0x02) ///< Overrun error
#define LSR_PE   						(0x04) ///< Parity error
#define LSR_FE   						(0x08) ///< Framing error
#define LSR_BI   						(0x10) ///< Break interrupt
#define LSR_THRE 						(0x20) ///< Transmitter holding register (FIFO empty)
#define LSR_TEMT 						(0x40) ///< Transmitter empty (FIFO and TSR both empty)
#define LSR_FFE  						(0x80) ///< At least one PE, FE or BI in FIFO

/** See datasheet section 7.8 for configuring the
  * "Programmable baud rate generator"
  */
#define SC16IS7X0_XTAL_FREQ              	8000000UL	///< On-board crystal
#define SC16IS7X0_PRESCALER_1               1			///< Default prescaler after reset
#define SC16IS7X0_PRESCALER_4            	4   		///< Selectable by setting MCR[7]
#define SC16IS7X0_PRESCALER                 SC16IS7X0_PRESCALER_1
#define SC16IS7X0_BAUDRATE_DIVISOR(baud)	((SC16IS7X0_XTAL_FREQ/SC16IS7X0_PRESCALER)/(baud*16UL))

// See Chapter 11 of datasheet
#define SC16IS7X0_SPI_READ_MODE_FLAG	(0x80)

typedef enum serial_parity {
	SERIAL_PARITY_NONE,
	SERIAL_PARITY_ODD,
	SERIAL_PARITY_EVEN,
	SERIAL_PARITY_FORCED0,
	SERIAL_PARITY_FORCED1
} _serial_parity;

#endif /* SC16IS7X0_H_ */
