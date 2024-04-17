/**
 * @file h3_uart.h
 *
 */
/* Copyright (C) 2018-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef H3_UART_H_
#define H3_UART_H_

#define	UART_LCR_DLAB			(1U << 7)	///< Divisor Latch Access Bit

#define UART_LSR_DR				(1U << 0)	///< Data Ready
#define	UART_LSR_OE				(1U << 1)	///< Overrun Error
#define UART_LSR_PE				(1U << 2)	///< Parity Error
#define UART_LSR_FE				(1U << 3)	///< Framing Error
#define UART_LSR_BI				(1U << 4)	///< Break Interrupt
#define UART_LSR_THRE			(1U << 5)	///< TX Holding Register Empty
#define UART_LSR_TEMT			(1U << 6)	///< Transmitter Empty
#define UART_LSR_FIFOERR		(1U << 7)	///< RX Data Error in FIFO

#define UART_LCR_DLS_8BITS		(3U << 0)	///< Data Length Select - 8 bits
#define UART_LCR_DLS_7BITS		(2U << 0)	///< Data Length Select - 7 bits
#define UART_LCR_DLS_6BITS		(1U << 0)	///< Data Length Select	- 6 bits
#define UART_LCR_DLS_5BITS		(0U << 0)	///< Data Length Select - 5 bits
#define UART_LCR_STOP_1BIT		(0U << 2)	///< 1 stop bit
#define UART_LCR_STOP_2BITS		(1U << 2)	///< 2 stop bits , 1.5 stop bits when DLS[1:0] = 0x0
#define UART_LCR_PEN			(1U << 3)	///< Parity enable
#define UART_LCR_EPS_ODD		(0U << 4)
#define UART_LCR_EPS_EVEN		(1U << 4)
 #define UART_LCR_8_N_1				(UART_LCR_DLS_8BITS)
 #define UART_LCR_8_N_2 			(UART_LCR_DLS_8BITS | UART_LCR_STOP_2BITS)
										///< EPS bit 5:4
										///
#define UART_LCR_BC				(0x1 << 6)	///< Cause a Break condition

#define UART_IER_ERBFI			(1U << 0)	///< Enable Received Data Available Interrupts
#define UART_IER_ETBEI			(1U << 1)	///< Enable Transmit Holding Register Empty Interrupt
#define UART_IER_PTIME			(1U << 7)	///< Programmable THRE Interrupt Mode Enable

#define UART_IIR_IID_NO			(1U << 0)	///< No Interrupt pending
#define UART_IIR_IID_THRE		(2U << 0)	///< THR empty
#define UART_IIR_IID_RD			(4U << 0)	///< Received data
#define UART_IIR_IID_RCVR_LINE	(6U << 0)	///< Receiver line status
#define UART_IIR_IID_TIME_OUT	(12U << 0)	///< Character time-out

#define UART_FCR_EFIFO	0x01	///< Enable in and out hardware FIFOs
#define UART_FCR_RRESET 0x02	///< Reset receiver FIFO
#define UART_FCR_TRESET 0x04	///< Reset transmit FIFO
#define UART_FCR_TRIG0	0x00	///< RCVR FIFO trigger level one char
#define UART_FCR_TRIG1	0x40	///< RCVR FIFO trigger level 1/4
#define UART_FCR_TRIG2	0x80	///< RCVR FIFO trigger level 2/4
#define UART_FCR_TRIG3	0xC0	///< RCVR FIFO trigger level 3/4

#define UART_USR_BUSY			(1U << 0)
#define UART_USR_TFE			(1U << 2)	///< Transmit FIFO empty
#define UART_USR_RFNE			(1U << 3)	///< Receive FIFO not empty

#define BAUD_31250_L	 (48)		///< 24000000 / 16 / 31250 = 48
#define BAUD_31250_H     (0)		///<

#define BAUD_115200_L    (13)		///< 24000000 / 16 / 115200 = 13
#define BAUD_115200_H    (0)		///<

#define BAUD_250000_L    (6)		///< 24000000 / 16 / 250000 = 6
#define BAUD_250000_H    (0)		///<

typedef enum H3_UART_BITS {
	H3_UART_BITS_5 = 5,
	H3_UART_BITS_6 = 6,
	H3_UART_BITS_7 = 7,
	H3_UART_BITS_8 = 8
} h3_uart_bits_t;

typedef enum H3_UART_PARITY {
	H3_UART_PARITY_NONE = 0,
	H3_UART_PARITY_ODD = 1,
	H3_UART_PARITY_EVEN = 2
} h3_uart_parity_t;

typedef enum H3_UART_STOPBITS {
	H3_UART_STOP_1BIT = 1,
	H3_UART_STOP_2BITS = 2
} h3_uart_stopbits_t;

#include <stdint.h>

#include "h3.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void h3_uart_begin(const uint32_t uart_base, uint32_t baudrate, uint32_t bits, uint32_t parity, uint32_t stop_bits);
extern void h3_uart_set_baudrate(const uint32_t uart_base, uint32_t baudrate);
extern void h3_uart_transmit(const uint32_t uart_base, const uint8_t *data, uint32_t length);
extern void h3_uart_transmit_string(const uint32_t uart_base, const char *data);

inline uint32_t h3_uart_get_rx_fifo_level(const uint32_t uart_base) {
	return ((const H3_UART_TypeDef *)(uintptr_t)(uart_base))->RFL;
}

inline uint8_t h3_uart_get_rx_data(const uint32_t uart_base) {
	return (uint8_t) ((const H3_UART_TypeDef *)(uintptr_t)(uart_base))->O00.RBR;
}

#ifdef __cplusplus
}
#endif

#endif /* H3_UART_H_ */
