/**
 * @file mididescription.h
 */
/* Copyright (C) 2020-2021 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>
#include <cstdio>

#include "mididescription.h"

#include "midi.h"

using namespace midi;

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

static constexpr char INSTRUMENT_NAMES[128][32] = {
	"Grand Piano", "Brite Piano", "Electronic Grand Piano", "Honky Tonk Piano", "Electronic Piano 1", "Electronic Piano 2", "Harpsichord", "Clavinet",	//   1-  8 Pianoforte
	"Celesta", "Glocken", "MusicBox", "Vibes", "Marimba", "Xylophone", "Tubular Bells", "Dulcimer", 													//   9- 16 Chromatic percussion
	"Drawbar Organ", "Percussive Organ", "Rock Organ", "Church Organ", "Reed Organ", "Accordion", "Harmonica", "Tango Accordion",						//  17- 24 Organ
	"Nylon Guitar", "Steel Guitar", "Jazz Guitar", "Clean Guitar", "Mute Guitar", "Overdrive Guitar", "Dist Guitar", "Guitar Harmonics",				//  25- 32 Guitar
	"Acoustic Bass", "Fingered Bass", "Pick Bass", "Fretless Bass", "Slap Bas 1", "Slap Bas 2", "Synth Bas 1", "Synth Bas 2",							//  33- 40 Bass
	"Violin", "Viola", "Cello", "Contrabass", "Tremolo Strings", "Pizzicato Strings", "Harp", "Timpani",												//  41- 48 Strings
	"Strings1", "Strings2", "Synth Strings 1", "Synth Strings 2", "Choir Aah", "Voice Ooh", "Synth Voice", "Orchestral Hit",							//  49- 56 Ensemble
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

const char* MidiDescription::GetType(Types tType) {
	switch (tType) {
	case Types::INVALIDE_TYPE:
		return "> program internal use <";
	case Types::NOTE_OFF:
		return "Note Off";
	case Types::NOTE_ON:
		return "Note On";
	case Types::AFTER_TOUCH_POLY:
		return "Polyphonic AfterTouch";
	case Types::CONTROL_CHANGE:
		if (static_cast<uint8_t>(tType) < 120) {
			return "Control Change";
		} else {
			// Controller numbers 120-127 are reserved for Channel Mode Messages,
			// which rather than controlling sound parameters, affect the channel's operating mode.
			return "Channel Mode";
		}
	case Types::PROGRAM_CHANGE:
		return "Program Change";
	case Types::AFTER_TOUCH_CHANNEL:
		return "Channel (monophonic) AfterTouch";
	case Types::PITCH_BEND:
		return "Pitch Bend";
	case Types::SYSTEM_EXCLUSIVE:
		return "Sys Exclusive";
	case Types::TIME_CODE_QUARTER_FRAME:
		return "Sys Common - MIDI Time Code Quarter Frame";
	case Types::SONG_POSITION:
		return "Sys Common - Song Position Pointer";
	case Types::SONG_SELECT:
		return "Sys Common - Song Select";
	case Types::TUNE_REQUEST:
		return "Sys Common - Tune Request";
	case Types::CLOCK:
		return "Sys Real Time - Timing Clock";
	case Types::START:
		return "Sys Real Time - Start";
	case Types::CONTINUE:
		return "Sys Real Time - Continue";
	case Types::STOP:
		return "Sys Real Time - Stop";
	case Types::ACTIVE_SENSING:
		return "Sys Real Time - Active Sensing";
	case Types::SYSTEM_RESET:
		return "Sys Real Time - Sys Reset";
	default:
		return "Unknown Type";
	}
}

const char* MidiDescription::GetControlChange(control::Change tControlChange) {
	switch (tControlChange) {
	case control::Change::ALL_SOUND_OFF:
		return "All Sound Off";
	case control::Change::RESET_ALL_CONTROLLERS:
		return "Reset All Controllers";
	case control::Change::LOCAL_CONTROL:
		return "Local Control";
	case control::Change::ALL_NOTES_OFF:
		return "All Notes Off";
	case control::Change::OMNI_MODE_OFF:
		return "Omni Mode Off (+ all notes off)";
	case control::Change::OMNI_MODE_ON:
		return "Omni Mode On (+ all notes off)";
	case control::Change::MONO_MODE_ON:
		return "Mono Mode On (+ poly off, + all notes off)";
	case control::Change::POLY_MODE_ON:
		return "Poly Mode On (+ mono off, + all notes off)";
	default:
		return "Unknown Control Change";
	}
}

const char* MidiDescription::GetControlFunction(control::Function tControlFunction) {
	const auto nFunction = static_cast<uint8_t>(tControlFunction);

	if (nFunction >= 0x14 && nFunction <= 0x1F) {
		return "Undefined";
	}

	if (nFunction >= 0x66 && nFunction <= 0x77) {
		return "Undefined";
	}

	switch (tControlFunction) {
	case control::Function::BANK_SELECT:
		return "Bank Select";
	case control::Function::MODULATION_WHEEL:
		return "Modulation Wheel or Lever";
	case control::Function::BREATH_CONTROLLER:
		return "Breath Controller";
	case control::Function::FOOT_CONTROLLER:
		return "Foot Controller";
	case control::Function::PORTAMENTO_TIME:
		return "Portamento Time";
	case control::Function::DATA_ENTRY_MSB:
		return "Data Entry MSB";
	case control::Function::CHANNEL_VOLUME:
		return "Channel Volume (formerly Main Volume)";
	case control::Function::BALANCE:
		return "Balance";
	case control::Function::PAN:
		return "Pan";
	case control::Function::EXPRESSION_CONTROLLER:
		return "Expression Controller";
	case control::Function::EFFECT_CONTROL_1:
		return "Effect Control 1";
	case control::Function::EFFECT_CONTROL_2:
		return "Effect Control 2";
	case control::Function::GP_CONTROLLER_1:
		return "General Purpose Controller 1";
	case control::Function::GP_CONTROLLER_2:
		return "General Purpose Controller 2";
	case control::Function::GP_CONTROLLER_3:
		return "General Purpose Controller 3";
	case control::Function::GP_CONTROLLER_4:
		return "General Purpose Controller 4";
	case control::Function::DAMPER_PEDAL_ON_OFF:
		return "Damper Pedal on/off (Sustain)";
	case control::Function::PORTAMENTO_ON_OFF:
		return "Portamento On/Off";
	case control::Function::SOSTENUTO_ON_OFF:
		return "Sostenuto On/Off";
	case control::Function::SOFT_PEDAL_ON_OFF:
		return "Soft Pedal On/Off";
	case control::Function::LEGATO_FOOTSWITCH:
		return "Legato Footswitch";
	case control::Function::HOLD_2:
		return "Hold 2";
	case control::Function::UNDEFINED_03:
	case control::Function::UNDEFINED_09:
	case control::Function::UNDEFINED_0E:
	case control::Function::UNDEFINED_0F:
		return "Undefined";
	default:
		return "Not implemented / Unknown Control Function";
	}
}

const char* MidiDescription::GetKeyName(uint8_t nKey) {
	if (nKey > 127) {
		return "";
	}

	const auto nNote = (nKey % 12);
	const auto nOctave = (nKey / 12);

	snprintf(key_name, sizeof(key_name) - 1, "%s%d", KEY_NAMES[nNote], nOctave);

	return const_cast<const char*>(key_name);
}

const char* MidiDescription::GetDrumKitName(uint8_t nDrumKit) {
	constexpr auto nArraySize = sizeof(DRUM_KITS) / sizeof(DRUM_KITS[0]);

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
