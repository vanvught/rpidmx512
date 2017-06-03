#ifndef MIDI_READER_MTC_H_
#define MIDI_READER_MTC_H_

#include "midi.h"

#include "midi.h"
#include "ltc_reader.h"

extern const _midi_timecode_type midi_reader_mtc(const struct _midi_message *);
extern const _midi_timecode_type midi_reader_mtc_qf(const struct _midi_message *);
extern void midi_reader_mtc_init(const struct _ltc_reader_output *);

#endif /* MIDI_READER_MTC_H_ */
