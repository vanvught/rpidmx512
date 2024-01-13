/**
 * @file rdmidentify.h
 *
 */
/* Copyright (C) 2018-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef RDMIDENTIFY_H_
#define RDMIDENTIFY_H_

#include <cstdint>

#include "hardware.h"

namespace rdm {
namespace identify {
enum class Mode : uint8_t {
	QUIET = 0x00, LOUD = 0xFF,
};
}  // namespace identify
}  // namespace rdm

class RDMIdentify {
public:
	RDMIdentify();
	~RDMIdentify() = default;

	void On() {
		m_bIsEnabled = true;
		Hardware::Get()->SetModeWithLock(hardware::ledblink::Mode::FAST, true);

		if (m_nMode != rdm::identify::Mode::QUIET) {
			On(m_nMode);
		}
	}

	void Off() {
		m_bIsEnabled = false;
		Hardware::Get()->SetModeWithLock(hardware::ledblink::Mode::NORMAL, false);

		if (m_nMode != rdm::identify::Mode::QUIET) {
			Off(m_nMode);
		}
	}

	bool IsEnabled() const {
		return m_bIsEnabled;
	}

	void SetMode(rdm::identify::Mode nMode) {
		if ((nMode == rdm::identify::Mode::QUIET) || (nMode == rdm::identify::Mode::LOUD)) {
			m_nMode = nMode;

			if (m_bIsEnabled) {
				if (nMode != rdm::identify::Mode::QUIET) {
					On(m_nMode);
				} else {
					Off(m_nMode);
				}
			} else {
				Off(m_nMode);
			}
		}
	}

	rdm::identify::Mode GetMode() const {
		return m_nMode;
	}

	static RDMIdentify* Get() {
		return s_pThis;
	}

private:
	void On(rdm::identify::Mode nMode) __attribute__((weak));
	void Off(rdm::identify::Mode nMode) __attribute__((weak));

private:
	static bool m_bIsEnabled;
	static rdm::identify::Mode m_nMode;
	static RDMIdentify *s_pThis;
};

#endif /* RDMIDENTIFY_H_ */
