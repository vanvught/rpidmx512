
#ifndef RTPMIDIHANDLER_H_
#define RTPMIDIHANDLER_H_

#include "midi.h"

class RtpMidiHandler {
public:
	virtual ~RtpMidiHandler() = default;

	virtual void MidiMessage(const struct midi::Message *pMidiMessage)=0;
};

#endif /* RTPMIDIHANDLER_H_ */
