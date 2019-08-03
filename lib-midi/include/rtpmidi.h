/**
 * @file rtpmidi.h
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef RTPMIDI_H_
#define RTPMIDI_H_

#include <stdint.h>

#include "applemidi.h"
#include "midi.h"

#include "rtpmidihandler.h"

class RtpMidi: public AppleMidi {
public:
	RtpMidi(void);
	~RtpMidi(void);

	void Start(void);
	void Stop(void);

	void Run(void);

	void SetHandler(RtpMidiHandler *pRtpMidiHandler) {
		m_pRtpMidiHandler = pRtpMidiHandler;
	}

	void Print(void);

private:
	void HandleRtpMidi(const uint8_t *pBuffer) override;

	int32_t DecodeTime(uint32_t nCommandLength, uint32_t nOffset);
	int32_t DecodeMidi(uint32_t nCommandLength, uint32_t nOffset);
	uint8_t GetTypeFromStatusByte(uint8_t nStatusByte);
	uint8_t GetChannelFromStatusByte(uint8_t nStatusByte);

private:
	struct _midi_message m_tMidiMessage;
	RtpMidiHandler *m_pRtpMidiHandler;
	uint8_t *m_pBuffer;
};

#endif /* RTPMIDI_H_ */
