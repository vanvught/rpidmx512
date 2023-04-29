/*
 * i2c.cpp
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#include "debug.h"

static int i2cbus;
static constexpr char fileName[] = "/dev/i2c-1";

void i2c_begin() {
	DEBUG_ENTRY

	if (i2cbus != 0) {
		close(i2cbus);
	}

	if ((i2cbus = open(fileName, O_RDWR)) < 0) {
		fprintf(stderr, "Failed to open i2c port %s \n", strerror(errno));
	}

	DEBUG_EXIT
}

void i2c_set_baudrate(__attribute__((unused)) uint32_t baudrate) {
}

void i2c_set_address(uint8_t address) {
	if (i2cbus == 0) {
		fprintf(stderr, "i2c_begin has failed\n");
		return;
	}

	if (ioctl(i2cbus, I2C_SLAVE, address) < 0) {
		fprintf(stderr, "Failed to set address %s\n", strerror(errno));
		i2cbus = 0;
	}
}

uint8_t i2c_write(const char *buf, uint32_t len) {
	if (i2cbus == 0) {
#if !defined (NDEBUG)
		fprintf(stderr, "i2c_begin has failed\n");
#endif
		return 1;
	}

	if (write(i2cbus, buf, len) != static_cast<ssize_t>(len)) {
#if !defined (NDEBUG)
		fprintf(stderr, "Failed to write i2c %s\n", strerror(errno));
#endif
		return 1;
	}

	return 0;
}

uint8_t i2c_read(char *buf, uint32_t len) {
	if (i2cbus == 0) {
#if !defined (NDEBUG)
		fprintf(stderr, "i2c_begin has failed\n");
#endif
		return 1;
	}

	if (read(i2cbus, buf, len) != static_cast<ssize_t>(len)) {
#if !defined (NDEBUG)
		fprintf(stderr, "Failed to read i2c %s\n", strerror(errno));
#endif
		return 1;
	}

	return 0;
}
