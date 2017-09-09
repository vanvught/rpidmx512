/**
 * @file midi.h
 *
 */
/* Copyright (C) 2016, 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef MIDI_H_
#define MIDI_H_

#include <stdint.h>
#include <stdbool.h>

#include "midi_interface.h"

#include "util.h"

#define MIDI_RX_BUFFER_INDEX_ENTRIES			(1 << 6)							///<
#define MIDI_RX_BUFFER_INDEX_MASK 				(MIDI_RX_BUFFER_INDEX_ENTRIES - 1)	///<

#define MIDI_BAUDRATE_DEFAULT					31250

#define MIDI_SYSTEM_EXCLUSIVE_INDEX_ENTRIES		128

/*! NoteOn with 0 velocity should be handled as NoteOf.
 Set to true  to get NoteOff events when receiving null-velocity NoteOn messages.
 Set to false to get NoteOn  events when receiving null-velocity NoteOn messages.
*/
#define HANDLE_NULL_VELOCITY_NOTE_ON_AS_NOTE_OFF	true
/*! Setting this to true will make midi_read parse only one byte of data for each
  call when data is available. This can speed up your application if receiving
  a lot of traffic, but might induce MIDI Thru and treatment latency.
*/
#define USE_1_BYTE_PARSING							true

#define MIDI_CHANNEL_OMNI		0		///<
#define MIDI_CHANNEL_OFF		17		///<

#define MIDI_PITCHBEND_MIN		-8192	///<
#define MIDI_PITCHBEND_MAX 		8191	///<

struct _midi_message {
	uint32_t timestamp;												///<
	uint8_t type;													///<
	uint8_t channel;												///<
	uint8_t data1;													///<
	uint8_t data2;													///<
	uint8_t system_exclusive[MIDI_SYSTEM_EXCLUSIVE_INDEX_ENTRIES];	///<
	uint8_t bytes_count;											///<
};

typedef enum midi_active_sense_state {
	MIDI_ACTIVE_SENSE_NOT_ENABLED = 0,	///<
	MIDI_ACTIVE_SENSE_ENABLED,			///<
	MIDI_ACTIVE_SENSE_FAILED			///<
} _midi_active_sense_state;

typedef enum midi_types {
	MIDI_TYPES_INVALIDE_TYPE 			= 0x00,	///< For notifying errors
	MIDI_TYPES_NOTE_OFF					= 0x80,	///< Note Off
	MIDI_TYPES_NOTE_ON					= 0x90,	///< Note On
	MIDI_TYPES_AFTER_TOUCH_POLY			= 0xA0,	///< Polyphonic AfterTouch
	MIDI_TYPES_CONTROL_CHANGE			= 0xB0,	///< Control Change / Channel Mode
	MIDI_TYPES_PROGRAM_CHANGE			= 0xC0,	///< Program Change
	MIDI_TYPES_AFTER_TOUCH_CHANNEL		= 0xD0,	///< Channel (monophonic) AfterTouch
	MIDI_TYPES_PITCH_BEND				= 0xE0,	///< Pitch Bend
	MIDI_TYPES_SYSTEM_EXCLUSIVE			= 0xF0,	///< System Exclusive
	MIDI_TYPES_TIME_CODE_QUARTER_FRAME	= 0xF1,	///< System Common - MIDI Time Code Quarter Frame
	MIDI_TYPES_SONG_POSITION			= 0xF2,	///< System Common - Song Position Pointer
	MIDI_TYPES_SONG_SELECT				= 0xF3,	///< System Common - Song Select
	MIDI_TYPES_TUNE_REQUEST				= 0xF6,	///< System Common - Tune Request
	MIDI_TYPES_CLOCK					= 0xF8,	///< System Real Time - Timing Clock
	MIDI_TYPES_START					= 0xFA,	///< System Real Time - Start
	MIDI_TYPES_CONTINUE					= 0xFB,	///< System Real Time - Continue
	MIDI_TYPES_STOP						= 0xFC,	///< System Real Time - Stop
	MIDI_TYPES_ACTIVE_SENSING			= 0xFE,	///< System Real Time - Active Sensing
	MIDI_TYPES_SYSTEM_RESET				= 0xFF	///< System Real Time - System Reset
} _midi_types;

typedef enum midi_channel_mode_message {
	MIDI_CONTROL_CHANGE_ALL_SOUND_OFF			= 0x78, ///<
	MIDI_CONTROL_CHANGE_RESET_ALL_CONTROLLERS	= 0x79,	///<
	MIDI_CONTROL_CHANGE_LOCAL_CONTROL			= 0x7A,	///<
	MIDI_CONTROL_CHANGE_ALL_NOTES_OFF			= 0x7B,	///<
	MIDI_CONTROL_CHANGE_OMNI_MODE_OFF			= 0x7C,	///<
	MIDI_CONTROL_CHANGE_OMNI_MODE_ON			= 0x7D,	///<
	MIDI_CONTROL_CHANGE_MONO_MODE_ON			= 0x7E,	///<
	MIDI_CONTROL_CHANGE_POLY_MODE_ON			= 0x7F	///<
} _midi_channel_mode_message;

typedef enum midi_channel_control_function {
	MIDI_CONTROL_FUNCTION_BANK_SELECT			= 0x00,	///< MSB
	MIDI_CONTROL_FUNCTION_MODULATION_WHEEL		= 0x01,	///< MSB
	MIDI_CONTROL_FUNCTION_BREATH_CONTROLLER		= 0x02,	///< MSB
	MIDI_CONTROL_FUNCTION_UNDEFINED_03			= 0x03,	///< MSB
	MIDI_CONTROL_FUNCTION_FOOT_CONTROLLER		= 0x04,	///< MSB
	MIDI_CONTROL_FUNCTION_PORTAMENTO_TIME		= 0x05,	///< MSB
	MIDI_CONTROL_FUNCTION_DATA_ENTRY_MSB		= 0x06,	///< MSB
	MIDI_CONTROL_FUNCTION_CHANNEL_VOLUME		= 0x07,	///< MSB
	MIDI_CONTROL_FUNCTION_BALANCE				= 0x08,	///< MSB
	MIDI_CONTROL_FUNCTION_UNDEFINED_09			= 0x09,	///< MSB
	MIDI_CONTROL_FUNCTION_PAN					= 0x0A,	///< MSB
	MIDI_CONTROL_FUNCTION_EXPRESSION_CONTROLLER	= 0x0B,	///< MSB
	MIDI_CONTROL_FUNCTION_EFFECT_CONTROL_1		= 0x0C,	///< MSB
	MIDI_CONTROL_FUNCTION_EFFECT_CONTROL_2		= 0x0D,	///< MSB
	MIDI_CONTROL_FUNCTION_UNDEFINED_0E			= 0x0E,	///< MSB
	MIDI_CONTROL_FUNCTION_UNDEFINED_0F			= 0x0F,	///< MSB
	MIDI_CONTROL_FUNCTION_GP_CONTROLLER_1		= 0x10, ///< MSB
	MIDI_CONTROL_FUNCTION_GP_CONTROLLER_2		= 0x11, ///< MSB
	MIDI_CONTROL_FUNCTION_GP_CONTROLLER_3		= 0x12, ///< MSB
	MIDI_CONTROL_FUNCTION_GP_CONTROLLER_4		= 0x13, ///< MSB
	MIDI_CONTROL_FUNCTION_DAMPER_PEDAL_ON_OFF	= 0x40,	///< 63 off, 64 on
	MIDI_CONTROL_FUNCTION_PORTAMENTO_ON_OFF		= 0x41,	///< 63 off, 64 on
	MIDI_CONTROL_FUNCTION_SOSTENUTO_ON_OFF		= 0x42,	///< 63 off, 64 on
	MIDI_CONTROL_FUNCTION_SOFT_PEDAL_ON_OFF		= 0x43,	///< 63 off, 64 on
	MIDI_CONTROL_FUNCTION_LEGATO_FOOTSWITCH		= 0x44,	///< 63 off, 64 on
	MIDI_CONTROL_FUNCTION_HOLD_2				= 0x45	///< 63 off, 64 on
} _midi_channel_control_function;

typedef enum midi_timecode_type {
	MIDI_TC_TYPE_FILM = 0,
	MIDI_TC_TYPE_EBU,
	MIDI_TC_TYPE_DF,
	MIDI_TC_TYPE_SMPTE,
	MIDI_TC_TYPE_UNKNOWN = 255
} _midi_timecode_type;

typedef enum midi_direction {
	MIDI_DIRECTION_INPUT 	= (1 << 0),
	MIDI_DIRECTION_OUTPUT	= (1 << 1)
} _midi_direction;

struct _midi_send_tc {
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint8_t frame;
	_midi_timecode_type rate;
} ;

#ifdef __cplusplus
extern "C" {
#endif

extern void midi_init(const _midi_direction);
extern void midi_set_baudrate(const uint32_t);
extern const uint32_t midi_get_baudrate(void);
extern /*@shared@*/const char *midi_get_interface_description(void);
extern void midi_set_interface(const _midi_interfaces);

extern /*@shared@*/struct _midi_message *midi_message_get(void) ASSUME_ALIGNED;
extern bool midi_read(void);
extern bool midi_read_channel(uint8_t);
extern uint8_t midi_get_input_channel(void);
extern void midi_set_input_channel(uint8_t);

extern _midi_active_sense_state midi_active_get_sense_state(void);
extern const bool midi_active_get_sense(void);
extern void midi_active_set_sense(const bool);

extern void midi_send_tc(const struct _midi_send_tc *);
extern void midi_send_raw(const uint8_t *, const uint16_t);

#ifdef __cplusplus
}
#endif

#endif /* MIDI_H_ */
