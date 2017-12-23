/**
 * @file i2c_lookup_device.c
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdint.h>

struct device_details {
	const uint8_t slave_address;
	const char *name;
} static const devices[] __attribute__((aligned(4))) = {
		{ 0x18, "MCP9808 {Temperature Sensor}" },
		{ 0x20, "MCP23017 {16-bit I/O port}"},
		{ 0x23, "BH1750FVI {Ambient Light Sensor}" },
		{ 0x27, "PCF8574T {8-bit I/O Expander}" },
		{ 0x3C, "SSD1306 {128 x 64 Dot Matrix}" },
		{ 0x40, "HTU21D {Humidity and Temperature Sensor} | INA219 {Current Sensor} | PCA9685 {16-channel, 12-bit PWM}" },
		{ 0x41, "BW:LCD" },
		{ 0x42, "BW:7 digital IO lines (DIO)" },
		{ 0x43, "BW:Servo" },
		{ 0x44, "BW:7 FETs" },
		{ 0x45, "BW:3 FETs" },
		{ 0x46, "BW:Temp" },
		{ 0x47, "BW:Relay" },
		{ 0x48, "PCF8591 {8-bit A/D and D/A} | ADS1x1x {A/D} | BW:Motor" },
		{ 0x4A, "BW:User Interface (UI)" },
		{ 0x4B, "BW:7 segment" },
		{ 0x4D, "BW:Pushbutton" },
		{ 0x4E, "BW:Bigrelay" },
		{ 0x4F, "BW:Dimmer" },
		{ 0x52, "BW:Raspberry juice" },
		{ 0x57, "MCP7941X {EEPROM}" },
		{ 0x6F, "MCP7941X {SRAM RTCC}" },
		{ 0x70, "TCA9548A {I2C Multiplexer} | PCA9685 {16-channel, 12-bit PWM}"}
		};

/*@observer@*/const char *i2c_lookup_device(uint8_t slave_address) {
	int i = 0;
	int j = (int) (sizeof(devices) / sizeof(struct device_details));

	for (i = 0; i < j; i++) {
		if (devices[i].slave_address == slave_address) {
			return devices[i].name;
		}
	}

	return "Unknown device";
}
