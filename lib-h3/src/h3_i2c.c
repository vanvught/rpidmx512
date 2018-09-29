/**
 * @file h3_i2c.c
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

#include <stdint.h>
#include <assert.h>
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include "debug.h"

#include "h3.h"
#include "h3_ccu.h"
#include "h3_gpio.h"
#include "h3_i2c.h"

#include "h3_board.h"

static uint8_t s_slave_address;
static uint32_t s_current_baudrate;

#define ALT_FUNCTION_SCK	(EXT_I2C_NUMBER == 0 ? (H3_PA11_SELECT_TWI0_SCK) : (H3_PA18_SELECT_TWI1_SCK))
#define ALT_FUNCTION_SDA	(EXT_I2C_NUMBER == 0 ? (H3_PA12_SELECT_TWI0_SDA) : (H3_PA19_SELECT_TWI1_SDA))

#define TIMEOUT			0xffff

typedef enum I2C_MODE {
	I2C_MODE_WRITE = 0,
	I2C_MODE_READ
} i2c_mode_t;

#define STAT_BUS_ERROR			0x00		///< Bus error
#define STAT_START_TRANSMIT     0x08		///< START condition transmitted
#define STAT_RESTART_TRANSMIT   0x10		///< Repeated START condition transmitted
#define STAT_ADDRWRITE_ACK	   	0x18		///< Address+Write bit transmitted, ACK received
#define STAT_DATAWRITE_ACK		0x28		///< Data transmitted in master mode, ACK received
#define STAT_ADDRREAD_ACK	   	0x40		///< Address+Read bit transmitted, ACK received
#define STAT_DATAREAD_ACK		0x50		///< Data byte received in master mode, ACK transmitted
#define STAT_DATAREAD_NACK	   	0x58		///< Data byte received in master mode, not ACK transmitted
#define STAT_READY			   	0xf8		///< No relevant status information, INT_FLAG=0

#define CTL_A_ACK				(1 << 2)	///< Assert Acknowledge
#define CTL_INT_FLAG			(1 << 3)	///< Interrupt Flag
#define CTL_M_STP				(1 << 4)	///< Master Mode Stop
#define CTL_M_STA				(1 << 5)	///< Master Mode Start
#define CTL_BUS_EN				(1 << 6)	///< TWI Bus Enable
#define CTL_INT_EN				(1 << 7)	///< Interrupt Enable

#define CC_CLK_N				(0x7 << 0)
	#define CLK_N_SHIFT		0
	#define CLK_N_MASK		0x7
#define CC_CLK_M				(0xF << 3)
	#define CLK_M_SHIFT		3
	#define CLK_M_MASK		0xF

static inline void _cc_write_reg(uint32_t clk_n, uint32_t clk_m) {
	uint32_t value = EXT_I2C->CC;
#ifndef NDEBUG
	printf("%s: clk_n = %d, clk_m = %d\n", __FUNCTION__, clk_n, clk_m);
#endif
	value &= ~(CC_CLK_M | CC_CLK_N);
	value |= ((clk_n << CLK_N_SHIFT) | (clk_m << CLK_M_SHIFT));
	EXT_I2C->CC = value;
}

static void _set_clock(uint32_t clk_in, uint32_t sclk_req) {
	uint32_t clk_m = 0;
	uint32_t clk_n = 0;
	uint32_t _2_pow_clk_n = 1;
	const uint32_t src_clk = clk_in / 10;
	uint32_t divider = src_clk / sclk_req;
	uint32_t sclk_real = 0;

	assert(divider != 0);

	while (clk_n < (CLK_N_MASK + 1)) {
		/* (m+1)*2^n = divider -->m = divider/2^n -1 */
		clk_m = (divider / _2_pow_clk_n) - 1;
		/* clk_m = (divider >> (_2_pow_clk_n>>1))-1 */

		while (clk_m < (CLK_M_MASK + 1)) {
			sclk_real = src_clk / (clk_m + 1) / _2_pow_clk_n; /* src_clk/((m+1)*2^n) */

			if (sclk_real <= sclk_req) {
				_cc_write_reg(clk_n, clk_m);
				return;
			} else {
				clk_m++;
			}
		}

		clk_n++;
		_2_pow_clk_n *= 2;
	}

	_cc_write_reg(clk_n, clk_m);

	return;
}

static int32_t _stop(void) {
	int32_t time = TIMEOUT;
	uint32_t tmp_val;

	EXT_I2C->CTL |= (0x01 << 4);

	while ((time--) && (EXT_I2C->CTL & 0x10))
		;

	if (time <= 0) {
		return -H3_I2C_NOK_TOUT;
	}

	time = TIMEOUT;
	while ((time--) && (EXT_I2C->STAT != STAT_READY))
		;

	tmp_val = EXT_I2C->STAT;

	if (tmp_val != STAT_READY) {
		return -H3_I2C_NOK_TOUT;
	}

	return H3_I2C_OK;
}

static int32_t _sendstart(void) {
	int32_t time = 0xfffff;
	uint32_t tmp_val;

	EXT_I2C->EFR = 0;
	EXT_I2C->SRST = 1;
	EXT_I2C->CTL |= 0x20;

	while ((time--) && (!(EXT_I2C->CTL & 0x08)))
		;

	if (time <= 0) {
		return -H3_I2C_NOK_TOUT;
	}

	tmp_val = EXT_I2C->STAT;

	if (tmp_val != STAT_START_TRANSMIT) {
		return -STAT_START_TRANSMIT;
	}

	return H3_I2C_OK;
}

static int32_t _sendslaveaddr(uint32_t mode) {
	int32_t time = TIMEOUT;
	uint32_t tmp_val;

	mode &= 1;

	EXT_I2C->DATA = (s_slave_address  << 1) | mode;
	EXT_I2C->CTL |= (0x01 << 3);

	while ((time--) && (!(EXT_I2C->CTL & 0x08)))
		;

	if (time <= 0) {
		return -H3_I2C_NOK_TOUT;
	}

	tmp_val = EXT_I2C->STAT;

	if (mode == I2C_MODE_WRITE) {
		if (tmp_val != STAT_ADDRWRITE_ACK) {
			return -STAT_ADDRWRITE_ACK;
		}
	} else {
		if (tmp_val != STAT_ADDRREAD_ACK) {
			return -STAT_ADDRREAD_ACK;
		}
	}

	return H3_I2C_OK;
}

static int32_t _getdata(uint8_t *data_addr, uint32_t data_count) {
	int32_t time = TIMEOUT;
	uint32_t tmp_val;
	uint32_t i;

	if (data_count == 1) {
		EXT_I2C->CTL |= (0x01 << 3);

		while ((time--) && (!(EXT_I2C->CTL & 0x08)))
			;

		if (time <= 0) {
			return -H3_I2C_NOK_TOUT;
		}

		for (time = 0; time < 100; time++)
			;

		*data_addr = EXT_I2C->DATA;;

		tmp_val = EXT_I2C->STAT;

		if (tmp_val != STAT_DATAREAD_NACK) {
			return -STAT_DATAREAD_NACK;
		}
	} else {
		for (i = 0; i < data_count - 1; i++) {
			time = TIMEOUT;
			tmp_val = EXT_I2C->CTL | (0x01 << 2);
			tmp_val = EXT_I2C->CTL | (0x01 << 3);
			tmp_val |= 0x04;
			EXT_I2C->CTL = tmp_val;

			while ((time--) && (!(EXT_I2C->CTL & 0x08)))
				;

			if (time <= 0) {
				return -H3_I2C_NOK_TOUT;
			}

			for (time = 0; time < 100; time++)
				;

			time = TIMEOUT;

			data_addr[i] = EXT_I2C->DATA;

			while ((time--) && (EXT_I2C->STAT != STAT_DATAREAD_ACK))
				;

			if (time <= 0) {
				return -H3_I2C_NOK_TOUT;
			}
		}

		time = 0xffff;
		EXT_I2C->CTL &= 0xFb;
		EXT_I2C->CTL |= (0x01 << 3);

		while ((time--) && (!(EXT_I2C->CTL & 0x08)))
			;

		if (time <= 0) {
			return -H3_I2C_NOK_TOUT;
		}

		for (time = 0; time < 100; time++)
			;

		data_addr[data_count - 1] = EXT_I2C->DATA;

		while ((time--) && (EXT_I2C->STAT != STAT_DATAREAD_NACK))
			;

		if (time <= 0) {
			return -H3_I2C_NOK_TOUT;
		}
	}

	return H3_I2C_OK;
}

static int32_t _senddata(uint8_t *data_addr, uint32_t data_count) {
	int32_t time = TIMEOUT;
	uint32_t i;

	for (i = 0; i < data_count; i++) {
		time = TIMEOUT;
		EXT_I2C->DATA = data_addr[i];
		EXT_I2C->CTL |= (0x01 << 3);

		while ((time--) && (!(EXT_I2C->CTL & 0x08)))
			;

		if (time <= 0) {
			return -H3_I2C_NOK_TOUT;
		}

		time = TIMEOUT;
		while ((time--) && (EXT_I2C->STAT != STAT_DATAWRITE_ACK))
			;
		if (time <= 0) {
			return -H3_I2C_NOK_TOUT;
		}
	}

	return H3_I2C_OK;
}

static int _read(char *buffer, int len) {
	int ret, ret0 = -1;

	ret = _sendstart();
	if (ret) {
		goto i2c_read_err_occur;
	}

	ret = _sendslaveaddr(I2C_MODE_READ);
	if (ret) {
		goto i2c_read_err_occur;
	}

	ret = _getdata((uint8_t *)buffer, len);

	if (ret) {
		goto i2c_read_err_occur;
	}
	ret0 = 0;

	i2c_read_err_occur: _stop();

	return ret0;
}

static int _write(char *buffer, int len) {
	int ret, ret0 = -1;

	ret = _sendstart();

	if (ret != H3_I2C_OK) {
		goto i2c_write_err_occur;
	}

	ret = _sendslaveaddr(I2C_MODE_WRITE);

	if (ret) {
		goto i2c_write_err_occur;
	}

	ret = _senddata((uint8_t *)buffer, len);

	if (ret) {
		goto i2c_write_err_occur;
	}

	ret0 = 0;

	i2c_write_err_occur: _stop();

	return ret0;
}

void h3_i2c_begin(void) {
	h3_gpio_fsel(EXT_I2C_SCL, ALT_FUNCTION_SCK);
	h3_gpio_fsel(EXT_I2C_SDA, ALT_FUNCTION_SDA);

#if (EXT_I2C_NUMBER == 0)
	H3_CCU->BUS_SOFT_RESET4 |= CCU_BUS_SOFT_RESET4_TWI0;
	udelay(1000); // 1ms
	H3_CCU->BUS_CLK_GATING3 &= ~CCU_BUS_CLK_GATING3_TWI0;
	udelay(1000); // 1ms
	H3_CCU->BUS_CLK_GATING3 |= CCU_BUS_CLK_GATING3_TWI0;
#elif (EXT_I2C_NUMBER == 1)
	H3_CCU->BUS_SOFT_RESET4 |= CCU_BUS_SOFT_RESET4_TWI1;
	udelay(1000); // 1ms
	H3_CCU->BUS_CLK_GATING3 &= ~CCU_BUS_CLK_GATING3_TWI1;
	udelay(1000); // 1ms
	H3_CCU->BUS_CLK_GATING3 |= CCU_BUS_CLK_GATING3_TWI1;
#else
 #error Unsupported I2C device configured
#endif

	EXT_I2C->CTL = 0x40;
	EXT_I2C->EFR = 0;

	_set_clock((uint32_t) H3_F_24M, H3_I2C_FULL_SPEED);
	s_current_baudrate = H3_I2C_FULL_SPEED;

#ifndef NDEBUG
	printf("%s I2C%c\n", __FUNCTION__, '0' + EXT_I2C_NUMBER);
	printf("H3_PIO_PORTA->CFG1=%p\n", H3_PIO_PORTA->CFG1);
	printf("H3_CCU->BUS_CLK_GATING4=%p\n", H3_CCU->BUS_CLK_GATING4);
	printf("H3_CCU->BUS_SOFT_RESET3=%p\n", H3_CCU->BUS_SOFT_RESET3);
	printf("EXT_I2C=%p\n", EXT_I2C);
	printf("EXT_I2C_SCL=%d\n", EXT_I2C_SCL);
	printf("EXT_I2C_SDA=%d\n", EXT_I2C_SDA);
	DEBUG_PRINTF("EXT_I2C->CTL=%p", EXT_I2C->CTL);
	DEBUG_PRINTF("EXT_I2C->CLK=%p", EXT_I2C->CC);
#endif
}

void h3_i2c_end(void) {
#if (EXT_I2C_NUMBER == 0)
	H3_CCU->BUS_CLK_GATING3 &= ~CCU_BUS_CLK_GATING3_TWI0;
	H3_CCU->BUS_SOFT_RESET4 &= ~CCU_BUS_SOFT_RESET4_TWI0;
#elif (EXT_I2C_NUMBER == 1)
	H3_CCU->BUS_CLK_GATING3 &= ~CCU_BUS_CLK_GATING3_TWI1;
	H3_CCU->BUS_SOFT_RESET4 &= ~CCU_BUS_SOFT_RESET4_TWI1;
#endif

	h3_gpio_fsel(EXT_I2C_SCL, GPIO_FSEL_DISABLE);
	h3_gpio_fsel(EXT_I2C_SDA, GPIO_FSEL_DISABLE);
}

uint8_t h3_i2c_write(/*@null@*/const char *buffer, uint32_t data_length) {
	const int32_t ret = _write((char *)buffer, (int) data_length);
#ifndef NDEBUG
	if (ret) {
		printf("ret=%d\n", ret);
	}
#endif
	return (uint8_t)-ret;;
}

uint8_t h3_i2c_read(/*@out@*/char *buffer, uint32_t data_length) {
	const int32_t ret = _read(buffer, (int) data_length);
#ifndef NDEBUG
	if (ret) {
		printf("ret=%d\n", ret);
	}
#endif
	return (uint8_t)-ret;;
}

void h3_i2c_set_baudrate(uint32_t baudrate) {
	assert(baudrate <= H3_I2C_FULL_SPEED);

	if (__builtin_expect((s_current_baudrate != baudrate),0)) {
		s_current_baudrate = baudrate;
		_set_clock((uint32_t) H3_F_24M, baudrate);
	}
}

void h3_i2c_set_slave_address(uint8_t address) {
	s_slave_address = address;
}

// Obsolete - Backwards compatibility with Raspberry Pi
#define BCM2835_CORE_CLK_HZ 250000000

void h3_i2c_setClockDivider(uint16_t divider) {
	assert(divider != 0);

	const uint32_t baudrate = (uint32_t) BCM2835_CORE_CLK_HZ / divider;
#ifndef NDEBUG
	printf("%s divider=%d, baudrate=%ld\n", __FUNCTION__, (int) divider, (long int) baudrate);
#endif

	if (divider <= ((uint32_t) BCM2835_CORE_CLK_HZ / H3_I2C_FULL_SPEED)) {
		_set_clock((uint32_t) H3_F_24M, H3_I2C_FULL_SPEED);
	} else {
		_set_clock((uint32_t) H3_F_24M, baudrate);
	}
}
