/**
 * @file bcm2835_i2c.h
 *
 */
/* Copyright (C) 2016-2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef BCM2835_I2C_H_
#define BCM2835_I2C_H_

#include <stdint.h>

#include "bcm2835.h"

#define BCM2835_BSC_C_I2CEN 		0x00008000 ///< I2C Enable, 0 = disabled, 1 = enabled
#define BCM2835_BSC_C_INTR 			0x00000400 ///< Interrupt on RX
#define BCM2835_BSC_C_INTT 			0x00000200 ///< Interrupt on TX
#define BCM2835_BSC_C_INTD 			0x00000100 ///< Interrupt on DONE
#define BCM2835_BSC_C_ST 			0x00000080 ///< Start transfer, 1 = Start a new transfer
#define BCM2835_BSC_C_CLEAR_1 		0x00000020 ///< Clear FIFO Clear
#define BCM2835_BSC_C_CLEAR_2 		0x00000010 ///< Clear FIFO Clear
#define BCM2835_BSC_C_READ 			0x00000001 ///<	Read transfer

#define BCM2835_BSC_S_CLKT 			0x00000200 ///< Clock stretch timeout
#define BCM2835_BSC_S_ERR 			0x00000100 ///< ACK error
#define BCM2835_BSC_S_RXF 			0x00000080 ///< RXF FIFO full, 0 = FIFO is not full, 1 = FIFO is full
#define BCM2835_BSC_S_TXE 			0x00000040 ///< TXE FIFO full, 0 = FIFO is not full, 1 = FIFO is full
#define BCM2835_BSC_S_RXD 			0x00000020 ///< RXD FIFO contains data
#define BCM2835_BSC_S_TXD 			0x00000010 ///< TXD FIFO can accept data
#define BCM2835_BSC_S_RXR 			0x00000008 ///< RXR FIFO needs reading (full)
#define BCM2835_BSC_S_TXW 			0x00000004 ///< TXW FIFO needs writing (full)
#define BCM2835_BSC_S_DONE 			0x00000002 ///< Transfer DONE
#define BCM2835_BSC_S_TA 			0x00000001 ///< Transfer Active

#define BCM2835_BSC_FIFO_SIZE   			16 ///< BSC FIFO size

/// Specifies the divider used to generate the I2C clock from the system clock.\n
/// Clock divided is based on nominal base clock rate of 250MHz
typedef enum {
	BCM2835_I2C_CLOCK_DIVIDER_2500	= 2500,		///< 2500 = 10us = 100 kHz
	BCM2835_I2C_CLOCK_DIVIDER_626	= 626,		///< 626 = 2.504us = 399.3610 kHz
	BCM2835_I2C_CLOCK_DIVIDER_150	= 150,		///< 150 = 60ns = 1.666 MHz (default at reset)
	BCM2835_I2C_CLOCK_DIVIDER_148	= 148,		///< 148 = 59ns = 1.689 MHz
} bcm2835I2CClockDivider;

/// Specifies the reason codes for the \ref bcm2835_i2c_write and \ref bcm2835_i2c_read functions.
typedef enum {
	BCM2835_I2C_REASON_OK			= 0x00,		///< Success
	BCM2835_I2C_REASON_ERROR_NACK 	= 0x01,		///< Received a NACK
	BCM2835_I2C_REASON_ERROR_CLKT 	= 0x02,		///< Received Clock Stretch Timeout
	BCM2835_I2C_REASON_ERROR_DATA 	= 0x04		///< Not all data is sent / received
} bcm2835I2CReasonCodes;

#ifdef __cplusplus
extern "C" {
#endif

extern void bcm2835_i2c_begin(void);
extern void bcm2835_i2c_end(void);
extern uint8_t bcm2835_i2c_write(/*@null@*/const char *, uint32_t);
extern uint8_t bcm2835_i2c_read(/*@out@*/char *, uint32_t);
extern void bcm2835_i2c_set_baudrate(uint32_t);

/**
 * @ingroup I2C
 *
 * Sets the I2C slave address
 * @param addr buffer for read.
 */
/*@unused@*/inline static void bcm2835_i2c_setSlaveAddress(uint8_t addr) {
	BCM2835_BSC1->A = addr;
}

/**
 * @ingroup I2C
 *
 * Sets the I2C clock divider and therefore the I2C clock speed.
 * @param divider The desired I2C clock divider, one of \ref bcm2835I2CClockDivider
 */
/*@unused@*/inline static void bcm2835_i2c_setClockDivider(uint16_t divider) {
	BCM2835_BSC1->DIV = divider;
}
#ifdef __cplusplus
}
#endif

#endif /* BCM2835_I2C_H_ */
