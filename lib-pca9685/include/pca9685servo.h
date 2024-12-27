/**
 * @file pca9685servo.h
 *
 */
/* Copyright (C) 2017-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef PCA9685SERVO_H_
#define PCA9685SERVO_H_

#include <cstdint>

#include "pca9685.h"


namespace pca9685::servo {
static constexpr uint32_t LEFT_DEFAULT_US 	= 1000;
static constexpr uint32_t CENTER_DEFAULT_US = 1500;
static constexpr uint32_t RIGHT_DEFAULT_US 	= 2000;
} // namespace pca9685::servo


class PCA9685Servo: public PCA9685 {
public:
	PCA9685Servo(const uint8_t nAddress): PCA9685(nAddress) {
		SetInvert(pca9685::Invert::OUTPUT_NOT_INVERTED);
		SetOutDriver(pca9685::Output::DRIVER_TOTEMPOLE);
		SetFrequency(50);
		CalcLeftCount();
		CalcRightCount();
		CalcCenterCount();
	}

	void SetLeftUs(const uint16_t nLeftUs) {
		if ((nLeftUs < m_nRightUs) && (nLeftUs < m_nCenterUs)) {
			m_nLeftUs = nLeftUs;
			CalcLeftCount();
		}
	}

	uint16_t GetLeftUs() const {
		return m_nLeftUs;
	}

	void SetRightUs(const uint16_t nRightUs) {
		if ((m_nLeftUs < nRightUs) && (m_nCenterUs < nRightUs)) {
			m_nRightUs = nRightUs;
			CalcRightCount();
		}
	}

	uint16_t GetRightUs() const {
		return m_nRightUs;
	}

	void SetCenterUs(const uint16_t nCenterUs) {
		if ((nCenterUs < m_nRightUs) && (m_nLeftUs < nCenterUs)) {
			m_nCenterUs = nCenterUs;
			CalcCenterCount();
		}
	}

	uint16_t GetCenterUs() const {
		return m_nCenterUs;
	}

	void Set(const uint32_t nChannel, uint16_t nData) {
		if (nData > m_nRightCount) {
			nData = m_nRightCount;
		} else if (nData < m_nLeftCount) {
			nData = m_nLeftCount;
		}

		Write(nChannel, nData);
	}

	void Set(const uint32_t nChannel, const uint8_t nData) {
		if (nData == 0) {
			Write(nChannel, m_nLeftCount);
		} else if (nData == (0xFF + 1) / 2) {
			Write(nChannel, m_nCenterCount);
		}  else if (nData == 0xFF) {
			Write(nChannel, m_nRightCount);
		} else {
			const auto nCount = static_cast<uint16_t>(m_nLeftCount + (.5f + (static_cast<float>((m_nRightCount - m_nLeftCount)) / 0xFF) * nData));
			Write(nChannel, nCount);
		}
	}

	void SetAngle(const uint32_t nChannel, const uint8_t nAngle) {
		if (nAngle == 0) {
			Write(nChannel, m_nLeftCount);
		} else if (nAngle == 90) {
			Write(nChannel, m_nCenterCount);
		}  else if (nAngle >= 180) {
			Write(nChannel, m_nRightCount);
		} else if (nAngle < 90) {
			const auto nCount = static_cast<uint16_t>(m_nLeftCount + (.5f + (static_cast<float>((m_nCenterCount - m_nLeftCount)) / 90) * nAngle));
			Write(nChannel, nCount);
		} else {
			const auto nCount = static_cast<uint16_t>((2.0f * m_nCenterCount) - m_nRightCount + (.5f + (static_cast<float>((m_nRightCount - m_nCenterCount)) / 90) * nAngle));
			Write(nChannel, nCount);
		}
	}

private:
	void CalcLeftCount() {
		m_nLeftCount = static_cast<uint16_t>((.5f + ((204.8f * m_nLeftUs) / 1000U)));
	}

	void CalcRightCount() {
		m_nRightCount = static_cast<uint16_t>((.5f + ((204.8f * m_nRightUs) / 1000U)));
	}

	void CalcCenterCount() {
		m_nCenterCount = static_cast<uint16_t>((.5f + ((204.8f * m_nCenterUs) / 1000U)));
	}

private:
	uint16_t m_nLeftUs { pca9685::servo::LEFT_DEFAULT_US };
	uint16_t m_nRightUs { pca9685::servo::RIGHT_DEFAULT_US };
	uint16_t m_nCenterUs { pca9685::servo::CENTER_DEFAULT_US };
	uint16_t m_nLeftCount;
	uint16_t m_nRightCount;
	uint16_t m_nCenterCount;
};

#endif /* PCA9685SERVO_H_ */
