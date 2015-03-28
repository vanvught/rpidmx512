/**
 * @file bcm2835_pl011.h
 *
 */
/* Copyright (C) 2014 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
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

#ifndef BCM2835_PL011_H_
#define BCM2835_PL011_H_

#include <stdint.h>

#define PL011_DR_OE 			((uint32_t)(1 << 11))	///< Set to 1 on overrun error
#define PL011_DR_BE 			((uint32_t)(1 << 10))	///< Set to 1 on break condition
#define PL011_DR_PE 			((uint32_t)(1 <<  9))	///< Set to 1 on parity error
#define PL011_DR_FE 			((uint32_t)(1 <<  8))	///< Set to 1 on framing error

#define PL011_RSRECR_OE 		((uint32_t)(1 << 3))	///< Set to 1 on overrun error
#define PL011_RSRECR_BE 		((uint32_t)(1 << 2))	///< Set to 1 on break condition
#define PL011_RSRECR_PE 		((uint32_t)(1 << 1))	///< Set to 1 on parity error
#define PL011_RSRECR_FE 		((uint32_t)(1 << 0))	///< Set to 1 on framing error

#define PL011_FR_BUSY 			((uint32_t)(1 << 3))	///< Set to 1 when UART is transmitting data
#define PL011_FR_RXFE			((uint32_t)(1 << 4))	///< Set to 1 when RX FIFO/register is empty
#define PL011_FR_TXFF 			((uint32_t)(1 << 5))	///< Set to 1 when TX FIFO/register is full
#define PL011_FR_RXFF			((uint32_t)(1 << 6))	///< Set to 1 when RX FIFO/register is full
#define PL011_FR_TXFE 			((uint32_t)( 1<< 7))	///< Set to 1 when TX FIFO/register is empty

#define PL011_LCRH_BRK			((uint32_t)(1 << 0))	///< Send break
#define PL011_LCRH_PEN			((uint32_t)(1 << 1))	///< Parity enable
#define PL011_LCRH_EPS			((uint32_t)(1 << 2))	///< Even parity select
#define PL011_LCRH_STP2			((uint32_t)(1 << 3))	///< Two stop bits select
#define PL011_LCRH_FEN			((uint32_t)(1 << 4))	///< Enable FIFOs
#define PL011_LCRH_WLEN8		((uint32_t)(0b11<<5))	///< Word length 8 bits
#define PL011_LCRH_WLEN7 		((uint32_t)(0b10<<5))	///< Word length 7 bits
#define PL011_LCRH_WLEN6 		((uint32_t)(0b01<<5))	///< Word length 6 bits
#define PL011_LCRH_WLEN5 		((uint32_t)(0b00<<5))	///< Word length 5 bits
#define PL011_LCRH_SPS			((uint32_t)(1 << 7))	///< Sticky parity select

#define PL011_IMSC_RXIM			((uint32_t)(1 << 4))	///<
#define PL011_IMSC_FEIM 		((uint32_t)(1 << 7))	///<
#define PL011_IMSC_BEIM 		((uint32_t)(1 << 9))	///<

#define PL011_MIS_RXMIS			((uint32_t)(1 << 4))	///<
#define PL011_MIS_FEMIS			((uint32_t)(1 << 7))	///<

#define PL011_ICRC_RXIC			((uint32_t)(1 << 4))	///<
#define PL011_ICR_FEIC 			((uint32_t)(1 << 7))	///<

#define PL011_BAUD_INT(x) 		(3000000 / (16 * (x)))
#define PL011_BAUD_FRAC(x) 		(int)((((3000000.0 / (16.0 * (x))) - PL011_BAUD_INT(x)) * 64.0) + 0.5)

extern void bcm2835_pl011_begin(void);
void bcm2835_pl011_send(const uint32_t c);

#endif /* BCM2835_PL011_H_ */
