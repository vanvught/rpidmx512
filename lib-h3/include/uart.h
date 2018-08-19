/**
 * @file uart.h
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef UART_H_
#define UART_H_

#define	UART_LCR_DLAB		(1 << 7)	///< Divisor Latch Access Bit

#define UART_LSR_DR			(1 << 0)	///< Data Ready
#define	UART_LSR_OE			(1 << 1)	///< Overrun Error
#define UART_LSR_PE			(1 << 2)	///< Parity Error
#define UART_LSR_FE			(1 << 3)	///< Framing Error
#define UART_LSR_BI			(1 << 4)	///< Break Interrupt
#define UART_LSR_THRE		(1 << 5)	///< TX Holding Register Empty
#define UART_LSR_TEMT		(1 << 6)	///< Transmitter Empty
#define UART_LSR_FIFOERR	(1 << 7)	///< RX Data Error in FIFO

#define UART_LCR_DLS_8BITS	(0x3 << 0)	///< Data Length Select - 8 bits
#define UART_LCR_DLS_7BITS	(0x2 << 0)	///< Data Length Select - 7 bits
#define UART_LCR_DLS_6BITS	(0x1 << 0)	///< Data Length Select	- 6 bits
#define UART_LCR_DLS_5BITS	(0x0 << 0)	///< Data Length Select - 5 bits
#define UART_LCR_STOP_1BIT	(0x0 << 2)	///< 1 stop bit
#define UART_LCR_STOP_2BITS	(0x1 << 2)	///< 2 stop bits , 1.5 stop bits when DLS[1:0] = 0x0
 #define UART_LCR_8_N_1			(UART_LCR_DLS_8BITS)
 #define UART_LCR_8_N_2 		(UART_LCR_DLS_8BITS | UART_LCR_STOP_2BITS)
										///< EPS bit 5:4
#define UART_LCR_BC			(0x1 << 6)	///< Cause a Break condition

#define UART_IER_ERBFI		(1<<0)		///< Enable Received Data Available Interrupts
#define UART_IER_ETBEI		(1<<1)		///< Enable Transmit Holding Register Empty Interrupt
#define UART_IER_PTIME		(1<<7)		///< Programmable THRE Interrupt Mode Enable

#define UART_IIR_IID_NO			(0b0001 << 0)	///< No Interrupt pending
#define UART_IIR_IID_THRE		(0b0010 << 0)	///< THR empty
#define UART_IIR_IID_RD			(0b0100 << 0)	///< Received data
#define UART_IIR_IID_RCVR_LINE	(0b0110 << 0)	///< Receiver line status

#define UART_FCR_EFIFO	0x01	///< Enable in and out hardware FIFOs
#define UART_FCR_RRESET 0x02	///< Reset receiver FIFO
#define UART_FCR_TRESET 0x04	///< Reset transmit FIFO
#define UART_FCR_TRIG0	0x00	///< RCVR FIFO trigger level one char
#define UART_FCR_TRIG1	0x40	///< RCVR FIFO trigger level 1/4
#define UART_FCR_TRIG2	0x80	///< RCVR FIFO trigger level 2/4
#define UART_FCR_TRIG3	0xC0	///< RCVR FIFO trigger level 3/4

#define UART_USR_BUSY	(1 << 0)

#define BAUD_115200_L    (13)		///< 24000000 / 16 / 115200 = 13
#define BAUD_115200_H    (0)		///<

#define BAUD_250000_L    (6)		///< 24000000 / 16 / 250000 = 6
#define BAUD_250000_H    (0)		///<

#endif /* UART_H_ */
