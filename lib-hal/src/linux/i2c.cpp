/*
 * i2c.cpp
 */

#include <cstdint>

#include "hal_i2c.h"

bool i2c_is_connected(const uint8_t nAddress, const uint32_t nBaudrate) {
	FUNC_PREFIX(i2c_set_address	(nAddress));
	FUNC_PREFIX(i2c_set_baudrate(nBaudrate));

	uint8_t nResult;
	char buffer;

	if ((nAddress >= 0x30 && nAddress <= 0x37) || (nAddress >= 0x50 && nAddress <= 0x5F)) {
		nResult = FUNC_PREFIX(i2c_read(&buffer, 1));
	} else {
		/* This is known to corrupt the Atmel AT24RF08 EEPROM */
		nResult = FUNC_PREFIX(i2c_write(nullptr, 0));
	}

	return (nResult == 0) ? true : false;
}

void i2c_write_register(const uint8_t nRegister, const uint8_t nValue) {
	char buffer[2];

	buffer[0] = static_cast<char>(nRegister);
	buffer[1] = static_cast<char>(nValue);

	FUNC_PREFIX(i2c_write(buffer, 2));
}

void i2c_read_register(const uint8_t nRegister, uint8_t& nValue) {
	char buffer[1];

	buffer[0] = static_cast<char>(nRegister);

	FUNC_PREFIX(i2c_write(buffer, 1));
	FUNC_PREFIX(i2c_read(buffer, 1));

	nValue = buffer[0];
}
