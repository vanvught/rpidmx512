/**
 * @file serial.h
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef SERIAL_H_
#define SERIAL_H_

#include <stdint.h>

enum class SerialType {
	UART,
	SPI,
	I2C,
	UNDEFINED
};

enum TSerialUartParity {
	SERIAL_UART_PARITY_NONE,
	SERIAL_UART_PARITY_ODD,
	SERIAL_UART_PARITY_EVEN,
	SERIAL_UART_PARITY_UNDEFINED
};

enum TSerialSpiModes {
	SERIAL_SPI_MODE0,
	SERIAL_SPI_MODE1,
	SERIAL_SPI_MODE2,
	SERIAL_SPI_MODE3
};

enum class SerialI2cSpeedMode {
	NORMAL,
	FAST,
	UNDEFINED
};

class Serial {
public:
	Serial(void);
	~Serial(void);

	void SetType(SerialType tType = SerialType::UART) {
		if (tType < SerialType::UNDEFINED) {
			m_tType = SerialType::UART;
		}
	}
	SerialType GetType(void) {
		return m_tType;
	}

	/*
	 * UART
	 */
	void SetUartBaud(uint32_t nBaud);
	void SetUartBits(uint8_t nBits = 8);
	void SetUartParity(TSerialUartParity tParity = SERIAL_UART_PARITY_NONE);
	void SetUartStopBits(uint8_t nStopBits = 1);

	/*
	 * SPI
	 */
	void SetSpiSpeedHz(uint32_t nSpeedHz);
	void SetSpiMode(TSerialSpiModes tMode);

	/*
	 * I2C
	 */
	void SetI2cAddress(uint8_t nAddress);
	void SetI2cSpeedMode(SerialI2cSpeedMode tSpeedMode = SerialI2cSpeedMode::FAST);

	bool Init(void);

	void Print(void);

	/*
	 * Send the data
	 */
	void Send(const uint8_t *pData, uint32_t nLength);

	static const char *GetType(SerialType tType);
	static SerialType GetType(const char *pType);

	static const char *GetUartParity(TSerialUartParity tParity);
	static enum TSerialUartParity GetUartParity(const char *pParity);

	static const char *GetI2cSpeed(SerialI2cSpeedMode tSpeed);
	static SerialI2cSpeedMode GetI2cSpeed(const char *pSpeed);

	static Serial *Get(void) {
		return s_pThis;
	}

private:
	bool InitUart(void);
	void SendUart(const uint8_t *pData, uint32_t nLength);

	bool InitSpi(void);
	void SendSpi(const uint8_t *pData, uint32_t nLength);

	bool InitI2c(void);
	void SendI2c(const uint8_t *pData, uint32_t nLength);

private:
	SerialType m_tType;
	struct {
		uint32_t nBaud;
		uint8_t nBits;
		TSerialUartParity tParity;
		uint8_t nStopBits;
	} m_UartConfiguration;
	struct {
		uint32_t nSpeed;
		uint8_t nMode;
	} m_SpiConfiguration;
	struct {
		uint8_t nAddress;
		SerialI2cSpeedMode tMode;
	} m_I2cConfiguration;

	static Serial *s_pThis;
};

#endif /* SERIAL_H_ */
