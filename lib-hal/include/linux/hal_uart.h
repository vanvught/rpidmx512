/**
 * @file hal_uart.h
 *
 */
/* Copyright (C) 2021-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef LINUX_HAL_UART_H_
#define LINUX_HAL_UART_H_

#include <cstdint>

typedef enum LINUX_UART_BITS
{
    LINUX_UART_BITS_5 = 5,
    LINUX_UART_BITS_6 = 6,
    LINUX_UART_BITS_7 = 7,
    LINUX_UART_BITS_8 = 8
} linux_uart_bits_t;

typedef enum LINUX_UART_PARITY
{
    LINUX_UART_PARITY_NONE = 0,
    LINUX_UART_PARITY_ODD = 1,
    LINUX_UART_PARITY_EVEN = 2
} linux_uart_parity_t;

typedef enum LINUX_UART_STOPBITS
{
    LINUX_UART_STOP_1BIT = 1,
    LINUX_UART_STOP_2BITS = 2
} linux_uart_stopbits_t;

namespace hal::uart {
static constexpr auto BITS_5 = LINUX_UART_BITS_5;
static constexpr auto BITS_6 = LINUX_UART_BITS_6;
static constexpr auto BITS_7 = LINUX_UART_BITS_7;
static constexpr auto BITS_8 = LINUX_UART_BITS_8;

static constexpr auto PARITY_NONE = LINUX_UART_PARITY_NONE;
static constexpr auto PARITY_ODD = LINUX_UART_PARITY_ODD;
static constexpr auto PARITY_EVEN = LINUX_UART_PARITY_EVEN;

static constexpr auto STOP_1BIT = LINUX_UART_STOP_1BIT;
static constexpr auto STOP_2BITS = LINUX_UART_STOP_2BITS;
} // namespace hal::uart

#if defined(RASPPI)
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>
    void bcm2835_UartBegin(uint32_t uart_base, uint32_t baudrate, uint32_t bits, uint32_t parity, uint32_t stop_bits);
    void bcm2835_UartSetBaudrate(uint32_t uart_base, uint32_t baudrate);
    void bcm2835_UartTransmit(uint32_t uart_base, const uint8_t* data, uint32_t length);
    void bcm2835_UartTransmitString(uint32_t uart_base, const char* data);

    uint32_t bcm2835_UartGetRxFifoLevel(uint32_t uart_base);
    uint8_t bcm2835_UartGetRxData(uint32_t uart_base);
    uint32_t bcm2835_uart_get_rx(uint32_t uart_base, char* pData, uint32_t nLength);

#define bcm2835_UartBegin UartBegin
#define bcm2835_UartSetBaudrate UartSetBaudrate
#define bcm2835_UartTransmit UartTransmit
#define bcm2835_UartTransmitString UartTransmitString

#define bcm2835_uart_get_rx UartGetRx
#ifdef __cplusplus
}
#endif
#else
#define FUNC_PREFIX(x) x
#endif

#define EXT_UART_NUMBER 0
#define EXT_UART_BASE 0
#define EXT_MIDI_UART_BASE 0

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

    void UartBegin(uint32_t uart_base, uint32_t baudrate, uint32_t bits, uint32_t parity, uint32_t stop_bits);
    void UartSetBaudrate(uint32_t uart_base, uint32_t baudrate);
    void UartTransmit(uint32_t uart_base, const uint8_t* data, uint32_t length);
    void UartTransmitString(uint32_t uart_base, const char* data);

    uint32_t UartGetRxFifoLevel(uint32_t uart_base);
    uint8_t UartGetRxData(uint32_t uart_base);
    uint32_t UartGetRx(uint32_t uart_base, char* data, uint32_t length);

#ifdef __cplusplus
}
#endif

#endif  // LINUX_HAL_UART_H_
