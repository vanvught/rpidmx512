/**
 * @file console.cpp
 *
 */
/* Copyright (C) 2022-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>

#include "console.h"
#include "hal_i2c.h"

static bool s_is_connected;

#if !defined(CONSOLE_I2C_ADDRESS)
#define CONSOLE_I2C_ADDRESS (0x4D)
#endif

#if !defined(CONSOLE_I2C_ONBOARD_CRYSTAL)
#define CONSOLE_I2C_ONBOARD_CRYSTAL (14745600UL)
#endif

#if !defined(CONSOLE_I2C_BAUDRATE)
#define CONSOLE_I2C_BAUDRATE (115200U)
#endif

#define SC16IS7X0_REG_SHIFT 3

#define SC16IS7X0_THR (0x00 << SC16IS7X0_REG_SHIFT)
#define SC16IS7X0_FCR (0x02 << SC16IS7X0_REG_SHIFT)
#define SC16IS7X0_LCR (0x03 << SC16IS7X0_REG_SHIFT)
#define SC16IS7X0_MCR (0x04 << SC16IS7X0_REG_SHIFT)
#define SC16IS7X0_SPR (0x07 << SC16IS7X0_REG_SHIFT)
#define SC16IS7X0_TLR (0x07 << SC16IS7X0_REG_SHIFT)
#define SC16IS7X0_TXLVL (0x08 << SC16IS7X0_REG_SHIFT)

#define SC16IS7X0_DLL (0x00 << SC16IS7X0_REG_SHIFT)
#define SC16IS7X0_DLH (0x01 << SC16IS7X0_REG_SHIFT)
#define SC16IS7X0_EFR (0x02 << SC16IS7X0_REG_SHIFT)

/** See section 8.3 of the datasheet for definitions
 * of bits in the FIFO Control Register (FCR)
 */
#define FCR_TX_FIFO_RST (1U << 2)
#define FCR_ENABLE_FIFO (1U << 0)

/** See section 8.4 of the datasheet for definitions
 * of bits in the Line Control Register (LCR)
 */

#define LCR_BITS8 (0x03)
#define LCR_BITS1 (0x00)
#define LCR_NONE (0x00)
#define LCR_ENABLE_DIV (0x80)

/**
 * 8.6 Modem Control Register (MCR)
 */
// MCR[2] only accessible when EFR[4] is set
#define MCR_ENABLE_TCR_TLR (1U << 2)
#define MCR_PRESCALE_4 (1U << 7)

/**
 * 8.11 Enhanced Features Register (EFR)
 */
#define EFR_ENABLE_ENHANCED_FUNCTIONS (1U << 4)

static bool IsConnected(uint8_t address, uint32_t baudrate)
{
    char buf;

    FUNC_PREFIX(I2cSetAddress(address));
    FUNC_PREFIX(I2cSetBaudrate(baudrate));

    if ((address >= 0x30 && address <= 0x37) || (address >= 0x50 && address <= 0x5F))
    {
        return FUNC_PREFIX(I2cRead(&buf, 1)) == 0;
    }

    /* This is known to corrupt the Atmel AT24RF08 EEPROM */
    return FUNC_PREFIX(I2cWrite(nullptr, 0)) == 0;
}

static void Setup()
{
    FUNC_PREFIX(I2cSetAddress(CONSOLE_I2C_ADDRESS));
    FUNC_PREFIX(I2cSetBaudrate(400000));
}

static void WriteRegister(uint8_t reg, uint8_t value)
{
    char buffer[2];

    buffer[0] = static_cast<char>(reg);
    buffer[1] = static_cast<char>(value);

    Setup();
    FUNC_PREFIX(I2cWrite(buffer, 2));
}

static uint8_t ReadByte()
{
    char buffer[1];

    Setup();
    FUNC_PREFIX(I2cRead(buffer, 1));

    return static_cast<uint8_t>(buffer[0]);
}

static uint8_t ReadRegister(uint8_t reg)
{
    char buffer[1];

    buffer[0] = static_cast<char>(reg);

    Setup();
    FUNC_PREFIX(I2cWrite(buffer, 1));

    return ReadByte();
}

static bool IsWritable()
{
    return (ReadRegister(SC16IS7X0_TXLVL) != 0);
}

static void SetBaud(uint32_t baud)
{
    uint32_t nPrescaler;

    if ((ReadRegister(SC16IS7X0_MCR) & MCR_PRESCALE_4) == MCR_PRESCALE_4)
    {
        nPrescaler = 4;
    }
    else
    {
        nPrescaler = 1;
    }

    const uint32_t kDivisor = ((CONSOLE_I2C_ONBOARD_CRYSTAL / nPrescaler) / (baud * 16));
    const uint8_t nRegisterLCR = ReadRegister(SC16IS7X0_LCR);

    WriteRegister(SC16IS7X0_LCR, (nRegisterLCR | LCR_ENABLE_DIV));
    WriteRegister(SC16IS7X0_DLL, (kDivisor & 0xFF));
    WriteRegister(SC16IS7X0_DLH, ((kDivisor >> 8) & 0xFF));
    WriteRegister(SC16IS7X0_LCR, nRegisterLCR);
}

namespace console
{

void __attribute__((cold)) Init()
{
    FUNC_PREFIX(I2cBegin());

    s_is_connected = IsConnected(CONSOLE_I2C_ADDRESS, 100000);

    if (!s_is_connected)
    {
        return;
    }

    uint8_t LCR = LCR_BITS8;
    LCR |= LCR_NONE;
    LCR |= LCR_BITS1;
    WriteRegister(SC16IS7X0_LCR, LCR);

    SetBaud(CONSOLE_I2C_BAUDRATE);

    uint8_t test_character = 'A';
    WriteRegister(SC16IS7X0_SPR, test_character);

    if ((ReadRegister(SC16IS7X0_SPR) != test_character))
    {
        s_is_connected = false;
        return;
    }

    s_is_connected = true;

    uint8_t MCR = ReadRegister(SC16IS7X0_MCR);
    MCR |= MCR_ENABLE_TCR_TLR;
    WriteRegister(SC16IS7X0_MCR, MCR);

    uint8_t EFR = ReadRegister(SC16IS7X0_EFR);
    WriteRegister(SC16IS7X0_EFR, (EFR | EFR_ENABLE_ENHANCED_FUNCTIONS));

    WriteRegister(SC16IS7X0_TLR, (0x10));

    WriteRegister(SC16IS7X0_EFR, EFR);

    WriteRegister(SC16IS7X0_FCR, (FCR_TX_FIFO_RST));
    WriteRegister(SC16IS7X0_FCR, FCR_ENABLE_FIFO);
}

void Putc(int c)
{
    if (!s_is_connected)
    {
        return;
    }

    if (c == '\n')
    {
        while (!IsWritable())
        {
        }

        WriteRegister(SC16IS7X0_THR, static_cast<uint8_t>('\r'));
    }

    while (!IsWritable())
    {
    }

    WriteRegister(SC16IS7X0_THR, static_cast<uint8_t>(c));
}

void Puts(const char* s)
{
    if (!s_is_connected)
    {
        return;
    }

    uint8_t* p = (uint8_t*)(s);

    while (*p != '\0')
    {
        uint32_t tx_level = ReadRegister(SC16IS7X0_TXLVL);

        while ((*p != '\0') && (tx_level > 0))
        {
            WriteRegister(SC16IS7X0_THR, *p);
            tx_level--;
            p++;
        }
    }
}

void Write(const char* s, unsigned int n)
{
    if (!s_is_connected)
    {
        return;
    }

    uint8_t* p = (uint8_t*)(s);

    while (n > 0)
    {
        uint32_t tx_level = ReadRegister(SC16IS7X0_TXLVL);

        while ((n > 0) && (tx_level > 0))
        {
            WriteRegister(SC16IS7X0_THR, *p);
            n--;
            tx_level--;
            p++;
        }
    }
}

void Error(const char* s)
{
    if (!s_is_connected)
    {
        return;
    }

    Puts("\x1b[31m");
    Puts(s);
    Puts("\x1b[37m");
}

void SetFgColour(Colours fg)
{
    switch (fg)
    {
        case Colours::kConsoleBlack:
            Puts("\x1b[30m");
            break;
        case Colours::kConsoleRed:
            Puts("\x1b[31m");
            break;
        case Colours::kConsoleGreen:
            Puts("\x1b[32m");
            break;
        case Colours::kConsoleYellow:
            Puts("\x1b[33m");
            break;
        case Colours::kConsoleWhite:
            Puts("\x1b[37m");
            break;
        default:
            Puts("\x1b[39m");
            break;
    }
}

void SetBgColour(Colours bg)
{
    switch (bg)
    {
        case Colours::kConsoleBlack:
            Puts("\x1b[40m");
            break;
        case Colours::kConsoleRed:
            Puts("\x1b[41m");
            break;
        case Colours::kConsoleWhite:
            Puts("\x1b[47m");
            break;
        default:
            Puts("\x1b[49m");
            break;
    }
}

void Status(Colours colour, const char* s)
{
    if (!s_is_connected)
    {
        return;
    }

    SetFgColour(colour);
    SetBgColour(Colours::kConsoleBlack);
    Puts(s);
    SetFgColour(Colours::kConsoleWhite);
}
} // namespace console
