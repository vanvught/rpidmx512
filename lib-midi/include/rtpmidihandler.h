
#ifndef RTPMIDIHANDLER_H_
#define RTPMIDIHANDLER_H_

#include "midi.h"

class RtpMidiHandler {
public:
	virtual ~RtpMidiHandler() {};

	virtual void MidiMessage(const struct _midi_message  *pMidiMessage)=0;
};

#endif /* RTPMIDIHANDLER_H_ */
