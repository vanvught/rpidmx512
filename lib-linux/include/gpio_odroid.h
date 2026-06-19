/**
 * @file gpio_odroid.h
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

#ifndef GPIO_ODROID_H_
#define GPIO_ODROID_H_

/*
odroid@gnome-desktop:~$ gpioinfo
gpiochip0 - 84 lines:
	line   0:      unnamed       unused   input  active-high 
	line   1:      unnamed       unused   input  active-high 
	line   2:      unnamed       kernel   input  active-high [used]
	line   3:      unnamed       kernel   input  active-high [used]
	line   4:      unnamed       kernel   input  active-high [used]
	line   5:      unnamed       kernel   input  active-high [used]
	line   6:      unnamed       kernel   input  active-high [used]
	line   7:      unnamed       kernel   input  active-high [used]
	line   8:      unnamed       kernel   input  active-high [used]
	line   9:      unnamed       kernel   input  active-high [used]
	line  10:      unnamed       kernel   input  active-high [used]
	line  11:      unnamed       unused  output  active-high 
	line  12:      unnamed       kernel   input  active-high [used]
	line  13:      unnamed       kernel   input  active-high [used]
	line  14:      unnamed       unused   input  active-high 
	line  15:      unnamed       unused   input  active-high 
	line  16:      unnamed       kernel   input  active-high [used]
	line  17:      unnamed       kernel   input  active-high [used]
	line  18:      unnamed       kernel   input  active-high [used]
	line  19:      unnamed       kernel   input  active-high [used]
	line  20:      unnamed       kernel   input  active-high [used]
	line  21:      unnamed       kernel   input  active-high [used]
	line  22:      unnamed         "cd"   input  active-high [used]
	line  23:      unnamed "fixed@vcc5v_reg" output active-high [used open-drain]
	line  24:      unnamed       unused   input  active-high 
	line  25:      unnamed       unused   input  active-high 
	line  26:     "PIN_28"       kernel   input  active-high [used]
	line  27:     "PIN_27"       kernel   input  active-high [used]
	line  28:      "PIN_7"       unused   input  active-high 
	line  29:      unnamed       unused   input  active-high 
	line  30:     "PIN_16"       unused   input  active-high 
	line  31:     "PIN_18"       unused   input  active-high 
	line  32:      unnamed       unused   input  active-high 
	line  33:      unnamed    "VDDIO_C"  output  active-high [used]
	line  34:   "BLUE_LED"       "blue"  output  active-high [used]
	line  35:    "RED_LED"        "red"  output   active-low [used]
	line  36:      unnamed       kernel   input  active-high [used]
	line  37:      unnamed       kernel   input  active-high [used]
	line  38:      unnamed       unused   input  active-high 
	line  39:      unnamed       kernel   input  active-high [used]
	line  40:     "PIN_29"       unused   input  active-high 
	line  41:     "PIN_31"       unused   input  active-high 
	line  42:      unnamed   "micro-sd"  output  active-high [used]
	line  43:      unnamed       unused  output  active-high 
	line  44:      unnamed "fe358000.usbphy20" output active-high [used]
	line  45:      unnamed  "PHY reset"  output   active-low [used open-drain]
	line  46:      unnamed       unused   input  active-high 
	line  47:      unnamed       kernel   input  active-high [used]
	line  48:     "PIN_33"       unused   input  active-high 
	line  49:     "PIN_35"       unused   input  active-high 
	line  50:     "PIN_32"       unused   input  active-high 
	line  51:     "PIN_36"       unused   input  active-high 
	line  52:     "PIN_15"       unused   input  active-high 
	line  53:     "PIN_11"       unused   input  active-high 
	line  54:     "PIN_12"       unused  output  active-high 
	line  55:     "PIN_26"       unused   input  active-high 
	line  56:     "PIN_19"       kernel   input  active-high [used]
	line  57:     "PIN_21"       kernel   input  active-high [used]
	line  58:     "PIN_24"   "spi0 CS0"  output   active-low [used]
	line  59:     "PIN_23"       kernel   input  active-high [used]
	line  60:      "PIN_8"       unused   input  active-high 
	line  61:     "PIN_10"       unused   input  active-high 
	line  62:     "PIN_22"       unused   input  active-high 
	line  63:     "PIN_13"       unused   input  active-high 
	line  64:      unnamed       unused   input  active-high 
	line  65:      "PIN_3"       kernel   input  active-high [used]
	line  66:      "PIN_5"       kernel   input  active-high [used]
	line  67:      unnamed    "usb_hub"  output  active-high [used]
	line  68:      unnamed       kernel   input  active-high [used]
	line  69:      unnamed       kernel   input  active-high [used]
	line  70:      unnamed       kernel   input  active-high [used]
	line  71:      unnamed       kernel   input  active-high [used]
	line  72:      unnamed       kernel   input  active-high [used]
	line  73:      unnamed       kernel   input  active-high [used]
	line  74:      unnamed       kernel   input  active-high [used]
	line  75:      unnamed       kernel   input  active-high [used]
	line  76:      unnamed       kernel   input  active-high [used]
	line  77:      unnamed       kernel   input  active-high [used]
	line  78:      unnamed       kernel   input  active-high [used]
	line  79:      unnamed       kernel   input  active-high [used]
	line  80:      unnamed       kernel   input  active-high [used]
	line  81:      unnamed       unused  output  active-high 
	line  82:      unnamed       unused   input  active-high 
	line  83:      unnamed       unused   input  active-high 
*/

// https://wiki.odroid.com/odroid-c5/hardware/expansion_connectors#j2_-_2x20_pins

#define GPIO_EXT_3   65  ///< I2C SDA [used by kernel]
#define GPIO_EXT_5   66  ///< I2C SCL [used by kernel]
#define GPIO_EXT_7   28
#define GPIO_EXT_8   60
#define GPIO_EXT_10  61
#define GPIO_EXT_11  53
#define GPIO_EXT_12  54
#define GPIO_EXT_13  63
#define GPIO_EXT_15  52
#define GPIO_EXT_16  30
#define GPIO_EXT_18  31
#define GPIO_EXT_19  56  ///< SPI MOSI [used by kernel]
#define GPIO_EXT_21  57  ///< SPI MISO [used by kernel]
#define GPIO_EXT_22  62
#define GPIO_EXT_23  59  ///< SPI CLK [used by kernel]
#define GPIO_EXT_24  58  ///< SPI CS0 [used by kernel]
#define GPIO_EXT_26  55
#define GPIO_EXT_28  26  ///< kernel used
#define GPIO_EXT_27  27  ///< kernel used
#define GPIO_EXT_29  40
#define GPIO_EXT_31  41
#define GPIO_EXT_32  50
#define GPIO_EXT_33  48
#define GPIO_EXT_35  49
#define GPIO_EXT_36  51

#endif // GPIO_ODROID_H_
