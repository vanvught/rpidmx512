/**
 * @file ads1x15.h
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

#ifndef ADS1X15_H_
#define ADS1X15_H_

#define ADS1x15_I2C_ADDRESS_ADDR_TO_GND			0x48	///<
#define ADS1x15_I2C_ADDRESS_ADDR_TO_VCC			0x49	///<
#define ADS1x15_I2C_ADDRESS_ADDR_TO_SDA			0x4a	///<
#define ADS1x15_I2C_ADDRESS_ADDR_TO_SCL			0x4b	///<

#define ADS1x15_REG_CONVERSION					0x00	///<
#define ADS1x15_REG_CONFIG						0x01	///<
#define ADS1x15_REG_LO_THRESH					0x02	///<
#define ADS1x15_REG_HI_THRESH					0x03	///<

#define ADS1x15_REG_CONFIG_OS_MASK				0x8000	///<
#define ADS1x15_REG_CONFIG_MUX_MASK				0x7000	///<
#define ADS1x15_REG_CONFIG_PGA_MASK  			0x0E00	///<
#define ADS1x15_REG_CONFIG_MODE_MASK  			0x0100	///<
#define ADS1x15_REG_CONFIG_DATA_RATE_MASK		0x00E0	///<
#define ADS1x15_REG_CONFIG_COMP_MODE_MASK		0x0010	///<
#define ADS1x15_REG_CONFIG_COMP_POLARITY_MASK	0x0008	///<
#define ADS1x15_REG_CONFIG_COMP_LATCHING_MASK	0x0004	///<
#define ADS1x15_REG_CONFIG_COMP_QUEUE_MASK		0x0003	///<

#define ADS1x15_CONFIG_OS_SINGLE 				0x8000	///< Write: Set to start a single-conversion
#define ADS1x15_CONFIG_OS_IDLE 					0x8000	///< Read: bit = 1 when device is not performing a conversion

typedef enum ads1x15_chip {
	ADS1015,	///<
	ADS1115		///<
} ads1x15_chip_t;

typedef enum ads1x15_mux {
	ADS1x15_REG_MUX_DIFF_0_1 = 0x0000,	///< Differential P = AIN0, N = AIN1 (default)
	ADS1x15_REG_MUX_DIFF_0_3 = 0x1000,  ///< Differential P = AIN0, N = AIN3
	ADS1x15_REG_MUX_DIFF_1_3 = 0x2000,  ///< Differential P = AIN1, N = AIN3
	ADS1x15_REG_MUX_DIFF_2_3 = 0x3000,  ///< Differential P = AIN2, N = AIN3
	ADS1x15_REG_MUX_SINGLE_0 = 0x4000,  ///< Single-ended AIN0
	ADS1x15_REG_MUX_SINGLE_1 = 0x5000,  ///< Single-ended AIN1
	ADS1x15_REG_MUX_SINGLE_2 = 0x6000,  ///< Single-ended AIN2
	ADS1x15_REG_MUX_SINGLE_3 = 0x7000   ///< Single-ended AIN3
} ads1x15_mux_t;

typedef enum ads1x15_pga {
	ADS1x15_PGA_6144 = 0x0000,	///<  +/-6.144V range = Gain 2/3
	ADS1x15_PGA_4096 = 0x0200,	///<  +/-4.096V range = Gain 1
	ADS1x15_PGA_2048 = 0x0400,	///<  +/-2.048V range = Gain 2 (default)
	ADS1x15_PGA_1024 = 0x0600,	///<  +/-1.024V range = Gain 4
	ADS1x15_PGA_512 = 0x0800,	///<  +/-0.512V range = Gain 8
	ADS1x15_PGA_256 = 0x0A00	///<  +/-0.256V range = Gain 16
} ads1x15_pga_t;

typedef enum ads1x15_mode {
	ADS1x15_MODE_CONTINUOUS = 0x0000,	///< Continuous conversion mode
	ADS1x15_MODE_SINGLE_SHOT = 0x0100	///< Power-down single-shot mode (default)
} ads1x15_mode_t;

typedef enum ads1x15_comp_mode {
	ADS1x15_COMP_MODE_TRADITIONAL = 0x0000,	///< Traditional comparator with hysteresis (default)
	ADS1x15_COMP_MODE_WINDOW = 0x0010		///< Window comparator
} ads1x15_comp_mode_t;

typedef enum ads1x15_comp_polarity {
	ADS1x15_COMP_POLARITY_ACTIVE_LO = 0x0000,	///< ALERT/RDY pin is low when active (default)
	ADS1x15_COMP_POLARITY_ACTIVE_HI = 0x0008	///< ALERT/RDY pin is high when active
} ads1x15_comp_polarity_t;

typedef enum ads1x15_comp_latching {
	ADS1x15_COMP_NON_LATCHING = 0x0000,	///< Non-latching comparator (default)
	ADS1x15_COMP_LATCHING = 0x0004		///< Latching comparator
} ads1x15_comp_latching_t;

typedef enum ads1x15_comp_queue {
	ADS1x15_COMP_QUEUE_1 = 0x0000,		///< Assert ALERT/RDY after one conversions
	ADS1x15_COMP_QUEUE_2 = 0x0001,		///< Assert ALERT/RDY after two conversions
	ADS1x15_COMP_QUEUE_4 = 0x0002,		///< Assert ALERT/RDY after four conversions
	ADS1x15_COMP_QUEUE_NONE = 0x0003	///< Disable the comparator and put ALERT/RDY in high state (default)
} ads1x15_comp_queue_t;

#include "device_info.h"

extern const uint16_t ads1x15_get_op_status(const device_info_t *);

extern const ads1x15_mux_t ads1x15_get_mux(const device_info_t *);
extern void ads1x15_set_mux(const device_info_t *, const ads1x15_mux_t);

extern const ads1x15_pga_t ads1x15_get_pga(const device_info_t *);
extern void ads1x15_set_pga(const device_info_t *, const ads1x15_pga_t);

extern const ads1x15_mode_t ads1x15_get_mode(const device_info_t *);
extern void ads1x15_set_mode(const device_info_t *, const ads1x15_mode_t);

extern const ads1x15_comp_mode_t ads1x15_get_comp_mode(const device_info_t *);
extern const ads1x15_comp_polarity_t ads1x15_get_comp_polarity(const device_info_t *);
extern const ads1x15_comp_latching_t ads1x15_get_comp_latching(const device_info_t *);
extern const ads1x15_comp_queue_t ads1x15_get_comp_queue(const device_info_t *);

#endif /* ADS1X15_H_ */
