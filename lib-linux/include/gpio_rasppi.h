/**
 * @file gpio_rasppi.h
 *
 */
/* Copyright (C) 2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef GPIO_RASPPI_H_
#define GPIO_RASPPI_H_

/*
pi@raspberrypi-4:~ $ gpioinfo
gpiochip0 - 58 lines:
	line   0:     "ID_SDA"       unused   input  active-high 
	line   1:     "ID_SCL"       unused   input  active-high 
	line   2:       "SDA1"       unused   input  active-high 
	line   3:       "SCL1"       unused   input  active-high 
	line   4:  "GPIO_GCLK"       unused   input  active-high 
	line   5:      "GPIO5"       unused   input  active-high 
	line   6:      "GPIO6"       unused   input  active-high 
	line   7:  "SPI_CE1_N"   "spi0 CS1"  output   active-low [used]
	line   8:  "SPI_CE0_N"   "spi0 CS0"  output   active-low [used]
	line   9:   "SPI_MISO"       unused   input  active-high 
	line  10:   "SPI_MOSI"       unused   input  active-high 
	line  11:   "SPI_SCLK"       unused   input  active-high 
	line  12:     "GPIO12"       unused   input  active-high 
	line  13:     "GPIO13"       unused   input  active-high 
	line  14:       "TXD1"       unused   input  active-high 
	line  15:       "RXD1"       unused   input  active-high 
	line  16:     "GPIO16"       unused   input  active-high 
	line  17:     "GPIO17"       unused   input  active-high 
	line  18:     "GPIO18"       unused   input  active-high 
	line  19:     "GPIO19"       unused   input  active-high 
	line  20:     "GPIO20"       unused   input  active-high 
	line  21:     "GPIO21"       unused   input  active-high 
	line  22:     "GPIO22"       unused   input  active-high 
	line  23:     "GPIO23"       unused   input  active-high 
	line  24:     "GPIO24"       unused   input  active-high 
	line  25:     "GPIO25"       unused   input  active-high 
	line  26:     "GPIO26"       unused   input  active-high 
	line  27:     "GPIO27"       unused   input  active-high 
	line  28: "RGMII_MDIO"       unused   input  active-high 
	line  29:  "RGMIO_MDC"       unused   input  active-high 
	line  30:       "CTS0"       unused   input  active-high 
	line  31:       "RTS0"       unused   input  active-high 
	line  32:       "TXD0"       unused   input  active-high 
	line  33:       "RXD0"       unused   input  active-high 
	line  34:    "SD1_CLK"       unused   input  active-high 
	line  35:    "SD1_CMD"       unused   input  active-high 
	line  36:  "SD1_DATA0"       unused   input  active-high 
	line  37:  "SD1_DATA1"       unused   input  active-high 
	line  38:  "SD1_DATA2"       unused   input  active-high 
	line  39:  "SD1_DATA3"       unused   input  active-high 
	line  40:  "PWM0_MISO"       unused   input  active-high 
	line  41:  "PWM1_MOSI"       unused   input  active-high 
	line  42: "STATUS_LED_G_CLK" "ACT" output active-high [used]
	line  43: "SPIFLASH_CE_N" unused input active-high 
	line  44:       "SDA0"       unused   input  active-high 
	line  45:       "SCL0"       unused   input  active-high 
	line  46: "RGMII_RXCLK" unused input active-high 
	line  47: "RGMII_RXCTL" unused input active-high 
	line  48: "RGMII_RXD0"       unused   input  active-high 
	line  49: "RGMII_RXD1"       unused   input  active-high 
	line  50: "RGMII_RXD2"       unused   input  active-high 
	line  51: "RGMII_RXD3"       unused   input  active-high 
	line  52: "RGMII_TXCLK" unused input active-high 
	line  53: "RGMII_TXCTL" unused input active-high 
	line  54: "RGMII_TXD0"       unused   input  active-high 
	line  55: "RGMII_TXD1"       unused   input  active-high 
	line  56: "RGMII_TXD2"       unused   input  active-high 
	line  57: "RGMII_TXD3"       unused   input  active-high 
*/

#define GPIO_EXT_3	2	///< Pin P1-03, SDA when I2C in use
#define GPIO_EXT_5	3	///< Pin P1-05, SCL when I2C in use
#define GPIO_EXT_7	4	///< Pin P1-07
#define GPIO_EXT_8	14  ///< Pin P1-08, defaults to ALT function 0 PL011_TXD
#define GPIO_EXT_10	15  ///< Pin P1-10, defaults to ALT function 0 PL011_RXD
#define GPIO_EXT_11	17  ///< Pin P1-11, CE1 when SPI1 in use
#define GPIO_EXT_12	18  ///< Pin P1-12, CE0 when SPI1 in use
#define GPIO_EXT_13	27  ///< Pin P1-13
#define GPIO_EXT_15	22  ///< Pin P1-15
#define GPIO_EXT_16	23  ///< Pin P1-16
#define GPIO_EXT_18	24  ///< Pin P1-18
#define GPIO_EXT_19	10  ///< Pin P1-19, MOSI when SPI0 in use
#define GPIO_EXT_21	9	///< Pin P1-21, MISO when SPI0 in use
#define GPIO_EXT_22	25	///< Pin P1-22
#define GPIO_EXT_23	11  ///< Pin P1-23, CLK when SPI0 in use
#define GPIO_EXT_24	8	///< Pin P1-24, CE0 when SPI0 in use
#define GPIO_EXT_26	7	///< Pin P1-26, CE1 when SPI0 in use
#define GPIO_EXT_29	5	///< Pin P1-29,
#define GPIO_EXT_31	6	///< Pin P1-31,
#define GPIO_EXT_32	12	///< Pin P1-32,
#define GPIO_EXT_33	13	///< Pin P1-33,
#define GPIO_EXT_35	19	///< Pin P1-35, MISO when SPI1 in use
#define GPIO_EXT_36	16	///< Pin P1-36, CE2 when SPI1 in use
#define GPIO_EXT_37	26	///< Pin P1-37,
#define GPIO_EXT_38	20	///< Pin P1-38, MOSI when SPI1 in use
#define GPIO_EXT_40	21	///< Pin P1-40, CLK when SPI1 in use

#endif // GPIO_RASPPI_H_
