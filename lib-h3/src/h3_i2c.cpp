/**
 * @file h3_i2c.cpp
 *
 */
/* Copyright (C) 2018-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined(__GNUC__) && !defined(__clang__)
# if defined (CONFIG_I2C_OPTIMIZE_O2) || defined (CONFIG_I2C_OPTIMIZE_O3)
#  pragma GCC push_options
#  if defined (CONFIG_I2C_OPTIMIZE_O2)
#   pragma GCC optimize ("O2")
#  else
#   pragma GCC optimize ("O3")
#  endif
# endif
#endif

#include <cstdint>
#include <cassert>
#ifndef NDEBUG
 #include<cstdio>
#endif

#include "debug.h"

#include "h3.h"
#include "h3_ccu.h"
#include "h3_gpio.h"
#include "h3_i2c.h"

#include "h3_board.h"

static uint8_t s_slave_address;
static uint32_t s_current_baudrate;

static constexpr auto ALT_FUNCTION_SCK = (EXT_I2C_NUMBER == 0 ? static_cast<uint32_t>(H3_PA11_SELECT_TWI0_SCK) : static_cast<uint32_t>(H3_PA18_SELECT_TWI1_SCK));
static constexpr auto ALT_FUNCTION_SDA = (EXT_I2C_NUMBER == 0 ? static_cast<uint32_t>(H3_PA12_SELECT_TWI0_SDA) : static_cast<uint32_t>(H3_PA19_SELECT_TWI1_SDA));

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

#define CTL_A_ACK				(1U << 2)	///< Assert Acknowledge
#define CTL_INT_FLAG			(1U << 3)	///< Interrupt Flag
#define CTL_M_STP				(1U << 4)	///< Master Mode Stop
#define CTL_M_STA				(1U << 5)	///< Master Mode Start
#define CTL_BUS_EN				(1U << 6)	///< TWI Bus Enable
#define CTL_INT_EN				(1U << 7)	///< Interrupt Enable

#define CC_CLK_N				(0x7 << 0)
	#define CLK_N_SHIFT		0
	#define CLK_N_MASK		0x7
#define CC_CLK_M				(0xF << 3)
	#define CLK_M_SHIFT		3
	#define CLK_M_MASK		0xF

static inline void _cc_write_reg(const uint32_t clk_n, const uint32_t clk_m) {
	uint32_t value = EXT_I2C->CC;
#ifndef NDEBUG
	printf("%s: clk_n = %d, clk_m = %d\n", __FUNCTION__, clk_n, clk_m);
#endif
	value &= static_cast<uint32_t>(~(CC_CLK_M | CC_CLK_N));
	value |= ((clk_n << CLK_N_SHIFT) | (clk_m << CLK_M_SHIFT));
	EXT_I2C->CC = value;
}

static void _set_clock(const uint32_t clk_in, const uint32_t sclk_req) {
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

static int32_t _stop() {
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

static int32_t _sendstart() {
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

	EXT_I2C->DATA = static_cast<uint32_t>(s_slave_address  << 1) | mode;
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
	int32_t time_out = TIMEOUT;
	uint32_t tmp_val;
	uint32_t i;

	if (data_count == 1) {
		EXT_I2C->CTL |= (0x01 << 3);

		while ((time_out--) && (!(EXT_I2C->CTL & 0x08)))
			;

		if (time_out <= 0) {
			return -H3_I2C_NOK_TOUT;
		}

		*data_addr = static_cast<uint8_t>(EXT_I2C->DATA);

		tmp_val = EXT_I2C->STAT;

		if (tmp_val != STAT_DATAREAD_NACK) {
			return -STAT_DATAREAD_NACK;
		}
	} else {
		for (i = 0; i < data_count - 1; i++) {
			time_out = TIMEOUT;
			tmp_val = EXT_I2C->CTL | (0x01 << 2);
			tmp_val = EXT_I2C->CTL | (0x01 << 3);
			tmp_val |= 0x04;
			EXT_I2C->CTL = tmp_val;

			while ((time_out--) && (!(EXT_I2C->CTL & 0x08)))
				;

			if (time_out <= 0) {
				return -H3_I2C_NOK_TOUT;
			}

			time_out = TIMEOUT;

			data_addr[i] = static_cast<uint8_t>(EXT_I2C->DATA);

			while ((time_out--) && (EXT_I2C->STAT != STAT_DATAREAD_ACK))
				;

			if (time_out <= 0) {
				return -H3_I2C_NOK_TOUT;
			}
		}

		time_out = 0xffff;
		EXT_I2C->CTL &= 0xFb;
		EXT_I2C->CTL |= (0x01 << 3);

		while ((time_out--) && (!(EXT_I2C->CTL & 0x08)))
			;

		if (time_out <= 0) {
			return -H3_I2C_NOK_TOUT;
		}

		data_addr[data_count - 1] = static_cast<uint8_t>(EXT_I2C->DATA);

		while ((time_out--) && (EXT_I2C->STAT != STAT_DATAREAD_NACK))
			;

		if (time_out <= 0) {
			return -H3_I2C_NOK_TOUT;
		}
	}

	return H3_I2C_OK;
}

static int32_t _senddata(const uint8_t *data_addr, const uint32_t data_count) {
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

	ret = _getdata(reinterpret_cast<uint8_t *>(buffer), static_cast<uint32_t>(len));

	if (ret) {
		goto i2c_read_err_occur;
	}
	ret0 = 0;

	i2c_read_err_occur: _stop();

	return ret0;
}

static int _write(const char *buffer, const int len) {
	int ret, ret0 = -1;

	ret = _sendstart();

	if (ret != H3_I2C_OK) {
		goto i2c_write_err_occur;
	}

	ret = _sendslaveaddr(I2C_MODE_WRITE);

	if (ret) {
		goto i2c_write_err_occur;
	}

	ret = _senddata(reinterpret_cast<const uint8_t *>(buffer), static_cast<uint32_t>(len));

	if (ret) {
		goto i2c_write_err_occur;
	}

	ret0 = 0;

	i2c_write_err_occur: _stop();

	return ret0;
}

void __attribute__((cold)) h3_i2c_begin() {
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

	_set_clock(H3_F_24M, H3_I2C_FULL_SPEED);
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

void __attribute__((cold)) h3_i2c_end() {
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

uint8_t h3_i2c_write(const char *buffer, uint32_t data_length) {
	const auto ret = _write(const_cast<char *>(buffer), static_cast<int>(data_length));
#ifndef NDEBUG
	if (ret) {
		printf("ret=%d\n", ret);
	}
#endif
	return static_cast<uint8_t>(-ret);
}

uint8_t h3_i2c_read(char *buffer, uint32_t data_length) {
	const auto ret = _read(buffer, static_cast<int>(data_length));
#ifndef NDEBUG
	if (ret) {
		printf("ret=%d\n", ret);
	}
#endif
	return static_cast<uint8_t>(-ret);
}

void h3_i2c_set_baudrate(const uint32_t nBaudrate) {
	assert(nBaudrate <= H3_I2C_FULL_SPEED);

	if (__builtin_expect((s_current_baudrate != nBaudrate), 0)) {
		s_current_baudrate = nBaudrate;
		_set_clock(H3_F_24M, nBaudrate);
	}
}

void h3_i2c_set_slave_address(const uint8_t nAddress) {
	s_slave_address = nAddress;
}

bool h3_i2c_is_connected(const uint8_t nAddress, const uint32_t nBaudrate) {
	h3_i2c_set_slave_address(nAddress);
	h3_i2c_set_baudrate(nBaudrate);

	uint8_t nResult;
	char buffer;

	if ((nAddress >= 0x30 && nAddress <= 0x37) || (nAddress >= 0x50 && nAddress <= 0x5F)) {
		nResult = h3_i2c_read(&buffer, 1);
	} else {
		/* This is known to corrupt the Atmel AT24RF08 EEPROM */
		nResult = h3_i2c_write(nullptr, 0);
	}

	return (nResult == 0) ? true : false;
}

void h3_i2c_write_register(const uint8_t nRegister, const uint8_t nValue) {
	char buffer[2];

	buffer[0] = static_cast<char>(nRegister);
	buffer[1] = static_cast<char>(nValue);

	h3_i2c_write(buffer, 2);
}

void h3_i2c_read_register(const uint8_t nRegister, uint8_t& nValue) {
	char buffer[1];

	buffer[0] = static_cast<char>(nRegister);

	h3_i2c_write(buffer, 1);
	h3_i2c_read(buffer, 1);

	nValue = buffer[0];
}
