/**
 * @file ili9341.h
 *
 */
/* Copyright (C) 2022-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef ILI9341_H_
#define ILI9341_H_

#include <cstdint>

#include "spi/config.h"

namespace ili9341 {
namespace cmd {
static constexpr uint8_t NOP 		= 0x00;
static constexpr uint8_t SWRESET	= 0x01;
static constexpr uint8_t RDDID		= 0x04;
static constexpr uint8_t RDDST		= 0x09;
static constexpr uint8_t SLPIN		= 0x10;
static constexpr uint8_t SLPOUT		= 0x11;
static constexpr uint8_t PTLON		= 0x12;
static constexpr uint8_t NORON		= 0x13;
static constexpr uint8_t INVOFF		= 0x20;
static constexpr uint8_t INVON		= 0x21;
static constexpr uint8_t DISPOFF	= 0x28;
static constexpr uint8_t DISPON		= 0x29;
static constexpr uint8_t CASET		= 0x2A;
static constexpr uint8_t RASET		= 0x2B;
static constexpr uint8_t RAMWR		= 0x2C;
static constexpr uint8_t RAMRD		= 0x2E;
static constexpr uint8_t PTLAR		= 0x30;
static constexpr uint8_t MADCTL		= 0x36;
static constexpr uint8_t PIXFMT		= 0x3A;
}  // namespace cmd
namespace data {
/**
 * Memory Data Access Control Register (0x36H)
 * MAP:     D7  D6  D5  D4  D3  D2  D1  D0
 * param:   MY  MX  MV  ML  RGB MH  -   -
 *
 */
static constexpr uint8_t MADCTL_MY	= 0x80;	///< Page Address Order ('0': Top to Bottom, '1': the opposite)
static constexpr uint8_t MADCTL_MX	= 0x40;	///< Column Address Order ('0': Left to Right, '1': the opposite)
static constexpr uint8_t MADCTL_MV	= 0x20;	///< Page/Column Order ('0' = Normal Mode, '1' = Reverse Mode)
static constexpr uint8_t MADCTL_ML	= 0x10;	///< Line Address Order ('0' = LCD Refresh Top to Bottom, '1' = the opposite)
static constexpr uint8_t MADCTL_RGB	= 0x00;	///< Red-Green-Blue pixel order
static constexpr uint8_t MADCTL_BGR	= 0x08;	///< Blue-Green-Red pixel order
}  // namespace data

namespace colour {
static constexpr uint16_t BLACK		= 0x0000;
static constexpr uint16_t BLUE		= 0x001F;
static constexpr uint16_t CYAN		= 0x07FF;
static constexpr uint16_t DARKBLUE	= 0X01CF;
static constexpr uint16_t GRAY		= 0X8430;
static constexpr uint16_t GREEN		= 0x07E0;
static constexpr uint16_t MAGENTA	= 0xF81F;
static constexpr uint16_t ORANGE 	= 0xFC00;
static constexpr uint16_t RED 		= 0xF800;
static constexpr uint16_t WHITE 	= 0xFFFF;
static constexpr uint16_t YELLOW 	= 0xFFE0;
}  // namespace colour
}  // namespace ili9341

#include "paint.h"

class ILI9341 : public Paint {
public:
	ILI9341();
	~ILI9341() override;

	void Init();

	void SetRotation(uint32_t nRotation);
	void SetBackLight(uint32_t nValue);

	void EnableDisplay(bool bEnable);
	void EnableSleep(bool bEnable);
	void EnableColourInversion(bool bEnable);

private:
	void SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) override;

protected:
	uint32_t m_nShiftX { 0 };
	uint32_t m_nShiftY { 0 };
};

#endif /* ILI9341_H_ */
