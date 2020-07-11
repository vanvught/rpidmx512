/**
 * @file mididescription.h
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

#include <stdint.h>
#include <stdio.h>

#include "mididescription.h"

#include "midi.h"

static constexpr char KEY_NAMES[][3] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
static char key_name[8] __attribute__((aligned(4)));

struct TDrumKits {			// Channel 10
	const uint8_t program;
	const char name[16];	// Including '\0' byte
};

static constexpr struct TDrumKits DRUM_KITS[8] =  {
	{  1, "Standaard Kit"  },
	{  9, "Room Kit"       },
	{ 17, "Rock Kit"       },
	{ 25, "Electronic Kit" },
	{ 26, "Analog Kit"     },
	{ 33, "Jazz Kit"       },
	{ 41, "Brush Kit"      },
	{ 49, "Classical Kit"  }
};

static constexpr char INSTRUMENT_NAMES[127][32] = {
	"Grand Piano", "Brite Piano", "Electronic Grand Piano", "Honky Tonk Piano", "Electronic Piano 1", "Electronic Piano 2", "Harpsichord", "Clavinet",	//   1-  8 Pianoforte
	"Celesta", "Glocken", "MusicBox", "Vibes", "Marimba", "Xylophone", "Tubular Bells", "Dulcimer", 													//   9- 16 Chromatic percussion
	"Drawbar Organ", "Percussive Organ", "Rock Organ", "Church Organ", "Reed Organ", "Accordion", "Harmonica", "Tango Accordion",						//  17- 24 Organ
	"Nylon Guitar", "Steel Guitar", "Jazz Guitar", "Clean Guitar", "Mute Guitar", "Overdrive Guitar", "Dist Guitar", "Guitar Harmonics",				//  25- 32 Guitar
	"Acoustic Bass", "Fingered Bass", "Pick Bass", "Fretless Bass", "Slap Bas 1", "Slap Bas 2", "Synth Bas 1", "Synth Bas 2",							//  33- 40 Bass
	"Violin", "Viola", "Cello", "Contrabass", "Tremolo Strings", "Pizzicato Strings", "Harp", "Timpani",												//  41- 48 Strings
	"Strings1", "Strings2", "Synth Strings 1", "Synth Strings 2", "Choir Aah", "Voice Ooh", "Synth Voice", "Orchestral Hit"								//  49- 56 Ensemble
	"Trumpet", "Trombone", "Tuba", "Mute.Trumpet", "French Horn", "Brass Section", "Synth Brass 1", "Synth Brass 2",									//  57- 54 Brass
	"Soprano Sax", "Alto Sax", "Tenor Sax", "Bariton Sax", "Oboe", "English Horn (althobo)", "Bassoon", "Clarinet",										//  65- 72 Reed
	"Piccolo", "Flute", "Recorder", "PanFlute", "Bottle", "Shakuhachi", "Whistle", "Ocarina", 															//  73- 80 Flutes
	"Square Lead", "Sawtooth Lead", "Caliope Lead", "Chiff Lead", "Charang Lead", "Voice Lead", "Fifth Lead", "Bas & Lead",								//  81- 88 Lead synth
	"New Age Pad", "Warm Pad", "Poly Synth Pad", "Choir Pad", "Bowed Pad", "Metal Pad", "Halo Pad", "Sweep Pad",										//  89- 96 Synth pads
	"Rain", "SoundTrack", "Crystal", "Atmosphere", "Bright", "Goblins", "Echoes", "Sci-Fi",																//  97-104 Effects
	"Sitar", "Banjo", "Shamisen", "Koto", "Kalimba", "Bagpipe", "Fiddle", "Shanai",																		// 105-112 Ethnic
	"Tinkle Bell", "Agogo", "Steel Drum", "Woodblock", "Taiko Drum", "Melodic Tom", "Synth Drum", "Rev Cymbal",											// 115-120 Percussion
	"Fret Noise", "Breath Noise", "Seashore", "Tweet", "Telephone", "Helicopter", "Applause", "Gunshot"													// 121-128 Sound effects
};

const char* MidiDescription::GetType(uint8_t nType) {
	switch (nType) {
	case MIDI_TYPES_INVALIDE_TYPE:
		return "> program internal use <";
	case MIDI_TYPES_NOTE_OFF:
		return "Note Off";
	case MIDI_TYPES_NOTE_ON:
		return "Note On";
	case MIDI_TYPES_AFTER_TOUCH_POLY:
		return "Polyphonic AfterTouch";
	case MIDI_TYPES_CONTROL_CHANGE:
		if (nType < 120) {
			return "Control Change";
		} else {
			// Controller numbers 120-127 are reserved for Channel Mode Messages,
			// which rather than controlling sound parameters, affect the channel's operating mode.
			return "Channel Mode";
		}
	case MIDI_TYPES_PROGRAM_CHANGE:
		return "Program Change";
	case MIDI_TYPES_AFTER_TOUCH_CHANNEL:
		return "Channel (monophonic) AfterTouch";
	case MIDI_TYPES_PITCH_BEND:
		return "Pitch Bend";
	case MIDI_TYPES_SYSTEM_EXCLUSIVE:
		return "Sys Exclusive";
	case MIDI_TYPES_TIME_CODE_QUARTER_FRAME:
		return "Sys Common - MIDI Time Code Quarter Frame";
	case MIDI_TYPES_SONG_POSITION:
		return "Sys Common - Song Position Pointer";
	case MIDI_TYPES_SONG_SELECT:
		return "Sys Common - Song Select";
	case MIDI_TYPES_TUNE_REQUEST:
		return "Sys Common - Tune Request";
	case MIDI_TYPES_CLOCK:
		return "Sys Real Time - Timing Clock";
	case MIDI_TYPES_START:
		return "Sys Real Time - Start";
	case MIDI_TYPES_CONTINUE:
		return "Sys Real Time - Continue";
	case MIDI_TYPES_STOP:
		return "Sys Real Time - Stop";
	case MIDI_TYPES_ACTIVE_SENSING:
		return "Sys Real Time - Active Sensing";
	case MIDI_TYPES_SYSTEM_RESET:
		return "Sys Real Time - Sys Reset";
	default:
		return "Unknown Type";
	}
}

const char* MidiDescription::GetControlChange(uint8_t nControlChange) {
	switch (nControlChange) {
	case MIDI_CONTROL_CHANGE_ALL_SOUND_OFF:
		return "All Sound Off";
	case MIDI_CONTROL_CHANGE_RESET_ALL_CONTROLLERS:
		return "Reset All Controllers";
	case MIDI_CONTROL_CHANGE_LOCAL_CONTROL:
		return "Local Control";
	case MIDI_CONTROL_CHANGE_ALL_NOTES_OFF:
		return "All Notes Off";
	case MIDI_CONTROL_CHANGE_OMNI_MODE_OFF:
		return "Omni Mode Off (+ all notes off)";
	case MIDI_CONTROL_CHANGE_OMNI_MODE_ON:
		return "Omni Mode On (+ all notes off)";
	case MIDI_CONTROL_CHANGE_MONO_MODE_ON:
		return "Mono Mode On (+ poly off, + all notes off)";
	case MIDI_CONTROL_CHANGE_POLY_MODE_ON:
		return "Poly Mode On (+ mono off, + all notes off)";
	default:
		return "Unknown Control Change";
	}
}

const char* MidiDescription::GetControlFunction(uint8_t nControlFunction) {
	if (nControlFunction >= 0x14 && nControlFunction <= 0x1F) {
		return "Undefined";
	}

	if (nControlFunction >= 0x66 && nControlFunction <= 0x77) {
		return "Undefined";
	}

	switch (nControlFunction) {
	case MIDI_CONTROL_FUNCTION_BANK_SELECT:
		return "Bank Select";
	case MIDI_CONTROL_FUNCTION_MODULATION_WHEEL:
		return "Modulation Wheel or Lever";
	case MIDI_CONTROL_FUNCTION_BREATH_CONTROLLER:
		return "Breath Controller";
	case MIDI_CONTROL_FUNCTION_FOOT_CONTROLLER:
		return "Foot Controller";
	case MIDI_CONTROL_FUNCTION_PORTAMENTO_TIME:
		return "Portamento Time";
	case MIDI_CONTROL_FUNCTION_DATA_ENTRY_MSB:
		return "Data Entry MSB";
	case MIDI_CONTROL_FUNCTION_CHANNEL_VOLUME:
		return "Channel Volume (formerly Main Volume)";
	case MIDI_CONTROL_FUNCTION_BALANCE:
		return "Balance";
	case MIDI_CONTROL_FUNCTION_PAN:
		return "Pan";
	case MIDI_CONTROL_FUNCTION_EXPRESSION_CONTROLLER:
		return "Expression Controller";
	case MIDI_CONTROL_FUNCTION_EFFECT_CONTROL_1:
		return "Effect Control 1";
	case MIDI_CONTROL_FUNCTION_EFFECT_CONTROL_2:
		return "Effect Control 2";
	case MIDI_CONTROL_FUNCTION_GP_CONTROLLER_1:
		return "General Purpose Controller 1";
	case MIDI_CONTROL_FUNCTION_GP_CONTROLLER_2:
		return "General Purpose Controller 2";
	case MIDI_CONTROL_FUNCTION_GP_CONTROLLER_3:
		return "General Purpose Controller 3";
	case MIDI_CONTROL_FUNCTION_GP_CONTROLLER_4:
		return "General Purpose Controller 4";
	case MIDI_CONTROL_FUNCTION_DAMPER_PEDAL_ON_OFF:
		return "Damper Pedal on/off (Sustain)";
	case MIDI_CONTROL_FUNCTION_PORTAMENTO_ON_OFF:
		return "Portamento On/Off";
	case MIDI_CONTROL_FUNCTION_SOSTENUTO_ON_OFF:
		return "Sostenuto On/Off";
	case MIDI_CONTROL_FUNCTION_SOFT_PEDAL_ON_OFF:
		return "Soft Pedal On/Off";
	case MIDI_CONTROL_FUNCTION_LEGATO_FOOTSWITCH:
		return "Legato Footswitch";
	case MIDI_CONTROL_FUNCTION_HOLD_2:
		return "Hold 2";
	case MIDI_CONTROL_FUNCTION_UNDEFINED_03:
	case MIDI_CONTROL_FUNCTION_UNDEFINED_09:
	case MIDI_CONTROL_FUNCTION_UNDEFINED_0E:
	case MIDI_CONTROL_FUNCTION_UNDEFINED_0F:
		return "Undefined";
	default:
		return "Not implemented / Unknown Control Function";
	}
}

const char* MidiDescription::GetKeyName(uint8_t nKey) {
	if (nKey > 127) {
		return "";
	}

	const int nNote = (nKey % 12);
	const int nOctave = (nKey / 12);

	snprintf(key_name, sizeof(key_name) - 1, "%s%d", KEY_NAMES[nNote], nOctave);

	return const_cast<const char*>(key_name);
}

const char* MidiDescription::GetDrumKitName(uint8_t nDrumKit) {
	constexpr uint32_t nArraySize = sizeof(DRUM_KITS) / sizeof(DRUM_KITS[0]);

	for (uint32_t i = 0; i < nArraySize; i++) {
		if (DRUM_KITS[i].program == (nDrumKit + 1)) {
			return const_cast<const char*>(DRUM_KITS[i].name);
		}
	}

	return "Percussion";
}

const char* MidiDescription::GetInstrumentName(uint8_t nInstrumentName) {
	if (nInstrumentName > 127) {
		return "";
	}

	return const_cast<const char*>(INSTRUMENT_NAMES[nInstrumentName]);
}
