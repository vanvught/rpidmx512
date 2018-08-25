/**
 * @file midi.c
 *
 * @brief This file implements the MIDI receive state-machine.
 *
 */
/* Parts of this code is inspired by:
 *  Project     Arduino MIDI Library
 *  https://github.com/FortySevenEffects/arduino_midi_library/blob/master/src/MIDI.cpp
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

#include <stdint.h>
#include <stdbool.h>

#include "midi.h"

#include "util.h"

#include "arm/arm.h"
#include "arm/pl011.h"
#include "arm/synchronize.h"

#include "bcm2835.h"
#include "bcm2835_gpio.h"
#include "bcm2835_vc.h"

#include "irq_timer.h"

#include "sc16is740.h"
#include "device_info.h"

struct _midi_receive {
	uint32_t timestamp;
	uint8_t data;
};

static struct _midi_message midi_message ALIGNED;											///<

static volatile struct _midi_receive midi_rx_buffer[MIDI_RX_BUFFER_INDEX_ENTRIES] ALIGNED;	///<
static volatile uint16_t midi_rx_buffer_index_head = (uint16_t) 0;							///<
static volatile uint16_t midi_rx_buffer_index_tail = (uint16_t) 0;							///<

static uint8_t input_channel = MIDI_CHANNEL_OMNI;											///<
static uint8_t pending_message_index = (uint8_t) 0;											///<
static uint8_t pending_message_expected_lenght = (uint8_t) 0;								///<
static uint8_t running_status_rx = MIDI_TYPES_INVALIDE_TYPE;								///<
static uint8_t pending_message[8] ALIGNED;													///<

static uint32_t midi_baudrate = MIDI_BAUDRATE_DEFAULT;										///<

static uint16_t midi_active_sense_timeout = 0;												///<
static _midi_active_sense_state midi_active_sense_state = MIDI_ACTIVE_SENSE_NOT_ENABLED;	///<

static bool midi_active_sense = false;
static _midi_direction midi_direction = MIDI_DIRECTION_INPUT;

static void pl011_init(void);
static void pl011_set(const uint32_t);
static void pl011_poll(void);
static const char *pl011_get(void);
static void pl011_send(const uint8_t *, uint16_t);

static void spi_init(void);
static void spi_set(const uint32_t);
static void spi_poll(void);
static const char *spi_get(void);
static void spi_send(const uint8_t *, uint16_t);

static device_info_t spi_device_info;

struct _midi_interface {
	void (*init)(void);							///< Pointer to function for
	void (*set)(const uint32_t);				///< Pointer to function for
	void (*poll)();								///< Pointer to function for
	const char * (*get)();						///< Pointer to function for
	void (*send)(const uint8_t *, uint16_t);	///< Pointer to function for
};

static struct _midi_interface midi_interface_f;
static struct _midi_interface midi_interfaces_f[] = {
		{ pl011_init, pl011_set, pl011_poll, pl011_get, pl011_send },
		{ spi_init, spi_set, spi_poll, spi_get, spi_send  }
};

/**
 *
 * @return
 */
const bool midi_active_get_sense(void) {
	return midi_active_sense;
}

/**
 *
 * @param sense
 */
void midi_active_set_sense(const bool sense) {
	midi_active_sense = sense;
}

/**
 * @ingroup midi
 *
 * @param interface
 */
void midi_set_interface(const uint8_t interface) {

	if (interface < sizeof(midi_interfaces_f) / sizeof(midi_interfaces_f[0])) {
		midi_interface_f.init = midi_interfaces_f[interface].init;
		midi_interface_f.set = midi_interfaces_f[interface].set;
		midi_interface_f.poll = midi_interfaces_f[interface].poll;
		midi_interface_f.get = midi_interfaces_f[interface].get;
		midi_interface_f.send = midi_interfaces_f[interface].send;
		return;
	}
}

/**
 * @ingroup midi_in
 *
 */
static void reset_input(void) {
	pending_message_index = 0;
	pending_message_expected_lenght = 0;
	running_status_rx = MIDI_TYPES_INVALIDE_TYPE;
}

/**
 * @ingroup midi
 *
 */
void midi_set_baudrate(const uint32_t baudrate) {
	midi_interface_f.set(baudrate);
}

/**
 * @ingroup midi
 *
 */
const uint32_t midi_get_baudrate(void) {
	return midi_baudrate;
}

/**
 * @ingroup midi_in
 *
 */
void midi_poll(void) {
	midi_interface_f.poll();
}

/**
 * * @ingroup midi
 *
 * @return
 */
const char *midi_get_interface_description(void) {
	return midi_interface_f.get();
}

/**
 * @ingroup midi
 *
 * @return
 */
struct _midi_message *midi_message_get(void) {
	return &midi_message;
}

/**
 * @ingroup midi_in
 *
 * @param byte
 * @return
 */
static bool raw_read(uint8_t *byte, uint32_t *timestamp) {
	dmb();
	if (midi_rx_buffer_index_head != midi_rx_buffer_index_tail) {
		*byte = midi_rx_buffer[midi_rx_buffer_index_tail].data;
		*timestamp = midi_rx_buffer[midi_rx_buffer_index_tail].timestamp;
		midi_rx_buffer_index_tail = (midi_rx_buffer_index_tail + 1) & MIDI_RX_BUFFER_INDEX_MASK;
		return true;
	} else {
		return false;
	}
}

/**
 * @ingroup midi_in
 *
 * @return
 */
uint8_t midi_get_input_channel(void) {
	return input_channel;
}

/**
 * @ingroup midi_in
 *
 * @param channel
 */
void midi_set_input_channel(uint8_t channel) {
	input_channel = channel;
}

/**
 * * @ingroup midi_in
 *
 * @return
 */
_midi_active_sense_state midi_active_get_sense_state(void) {
	dmb();
	return midi_active_sense_state;
}

/**
 * @ingroup midi_in
 *
 * @param status_byte
 * @return
 */
static uint8_t get_type_from_status_byte(uint8_t status_byte) {
	if ((status_byte < 0x80) || (status_byte == 0xf4) || (status_byte == 0xf5) || (status_byte == 0xf9) || (status_byte == 0xfD)) {
		// Data bytes and undefined.
		return MIDI_TYPES_INVALIDE_TYPE;
	}

	if (status_byte < 0xF0) {
		// Channel message, remove channel nibble.
		return status_byte & 0xF0;
	}

	return status_byte;
}

/**
 * @ingroup midi_in
 *
 * @param status_byte
 * @return
 */
static uint8_t get_channel_from_status_byte(uint8_t status_byte) {
	return (status_byte & 0x0F) + 1;
}

/**
 * @ingroup midi
 *
 * @param type
 * @return
 */
static bool is_channel_message(uint8_t type) {
	return (type == MIDI_TYPES_NOTE_OFF
			|| type == MIDI_TYPES_NOTE_ON
			|| type == MIDI_TYPES_CONTROL_CHANGE
			|| type == MIDI_TYPES_AFTER_TOUCH_POLY
			|| type == MIDI_TYPES_AFTER_TOUCH_CHANNEL
			|| type == MIDI_TYPES_PITCH_BEND
			|| type == MIDI_TYPES_PROGRAM_CHANGE);
}

/**
 * @ingroup midi_in
 *
 * @param inChannel
 * @return
 */
static bool input_filter(uint8_t channel) {
	if (midi_message.type == MIDI_TYPES_INVALIDE_TYPE)
		return false;

	// First, check if the received message is Channel
	if (midi_message.type >= MIDI_TYPES_NOTE_OFF && midi_message.type <= MIDI_TYPES_PITCH_BEND) {
		// Then we need to know if we listen to it
		if ((midi_message.channel == channel) || (channel == MIDI_CHANNEL_OMNI)) {
			return true;
		} else {
			// We don't listen to this channel
			return false;
		}
	} else {
		// System messages are always received
		return true;
	}
}

/**
 * @ingroup midi
 *
 */
static void handle_null_velocity_note_on_as_note_off(void) {
	if (HANDLE_NULL_VELOCITY_NOTE_ON_AS_NOTE_OFF
			&& (midi_message.type == MIDI_TYPES_NOTE_ON)
			&& (midi_message.data2 == 0)) {
		midi_message.type = MIDI_TYPES_NOTE_OFF;
	}
}

/**
 * @ingroup midi_in
 *
 */
static bool parse(void) {
	uint8_t serial_data;
	uint32_t timestamp;
	uint32_t pending_timestamp;


    if (!raw_read(&serial_data, &timestamp)) {
        return false;
	}

    midi_active_sense_timeout = 0;

	if (pending_message_index == (uint8_t) 0) {
		// Start a new pending message
		pending_timestamp = timestamp;
		pending_message[0] = serial_data;

		// Check for running status first
		if (is_channel_message(get_type_from_status_byte(running_status_rx))) {
			// Only these types allow Running Status
			// If the status byte is not received, prepend it
			// to the pending message
			if (serial_data < 0x80) {
				pending_message[0] = running_status_rx;
				pending_message[1] = serial_data;
				pending_message_index = (uint8_t) 1;
			}
			// Else: well, we received another status byte,
			// so the running status does not apply here.
			// It will be updated upon completion of this message.
		}

		switch (get_type_from_status_byte(pending_message[0])) {
		// 1 byte messages
		case MIDI_TYPES_ACTIVE_SENSING:
			dmb();
			midi_active_sense_state = MIDI_ACTIVE_SENSE_ENABLED;
			/* no break */
		case MIDI_TYPES_START:
		case MIDI_TYPES_CONTINUE:
		case MIDI_TYPES_STOP:
		case MIDI_TYPES_CLOCK:
		case MIDI_TYPES_SYSTEM_RESET:
		case MIDI_TYPES_TUNE_REQUEST:
			// Handle the message type directly here.
			midi_message.timestamp = pending_timestamp;
			midi_message.type = get_type_from_status_byte(pending_message[0]);
			midi_message.channel = (uint8_t) 0;
			midi_message.data1 = (uint8_t) 0;
			midi_message.data2 = (uint8_t) 0;
			midi_message.bytes_count = (uint8_t) 1;
			//midi_message.valid = true;
			// \fix Running Status broken when receiving Clock messages.
			// Do not reset all input attributes, Running Status must remain unchanged.
			//resetInput();
			// We still need to reset these
			pending_message_index = (uint8_t) 0;
			pending_message_expected_lenght = (uint8_t) 0;
			return true;
			break;
		// 2 bytes messages
		case MIDI_TYPES_PROGRAM_CHANGE:
		case MIDI_TYPES_AFTER_TOUCH_CHANNEL:
		case MIDI_TYPES_TIME_CODE_QUARTER_FRAME:
		case MIDI_TYPES_SONG_SELECT:
			pending_message_expected_lenght = (uint8_t) 2;
			break;
		// 3 bytes messages
		case MIDI_TYPES_NOTE_ON:
		case MIDI_TYPES_NOTE_OFF:
		case MIDI_TYPES_CONTROL_CHANGE:
		case MIDI_TYPES_PITCH_BEND:
		case MIDI_TYPES_AFTER_TOUCH_POLY:
		case MIDI_TYPES_SONG_POSITION:
			pending_message_expected_lenght = (uint8_t) 3;
			break;
		case MIDI_TYPES_SYSTEM_EXCLUSIVE:
			// The message can be any length
			// between 3 and MIDI_SYSTEM_EXCLUSIVE_INDEX_ENTRIES
			pending_message_expected_lenght = MIDI_SYSTEM_EXCLUSIVE_INDEX_ENTRIES;
			running_status_rx = MIDI_TYPES_INVALIDE_TYPE;
			midi_message.system_exclusive[0] = MIDI_TYPES_SYSTEM_EXCLUSIVE;
			midi_message.channel = (uint8_t) 0;
			midi_message.timestamp = pending_timestamp;
			break;
		case MIDI_TYPES_INVALIDE_TYPE:
		default:
			// This is obviously wrong. Let's get the hell out'a here.
			reset_input();
			return false;
			break;
		}

		if (pending_message_index >= (pending_message_expected_lenght - (uint8_t) 1)) {
			// Reception complete
			midi_message.timestamp = pending_timestamp;
			midi_message.type = get_type_from_status_byte(pending_message[0]);
			midi_message.channel = get_channel_from_status_byte(pending_message[0]);
			midi_message.data1 = pending_message[1];

			// Save data2 only if applicable
			if (pending_message_expected_lenght == (uint8_t) 3) {
				midi_message.data2 = pending_message[2];
				midi_message.bytes_count = (uint8_t) 3;
			} else {
				midi_message.data2 = (uint8_t) 0;
				midi_message.bytes_count = (uint8_t) 2;
			}
			pending_message_index = (uint8_t) 0;
			pending_message_expected_lenght = (uint8_t) 0;
			//midi_message.valid = true;
			return true;
		} else {
			// Waiting for more data
			pending_message_index++;
		}

		if (USE_1_BYTE_PARSING) {
			// Message is not complete.
			return false;
		} else {
			// Call the parser recursively
			// to parse the rest of the message.
			return parse();
		}
	} else {
		// First, test if this is a status byte
		if (serial_data >= 0x80) {
			// Reception of status bytes in the middle of an uncompleted message
			// are allowed only for interleaved Real Time message or EOX
			switch (serial_data) {
			case MIDI_TYPES_ACTIVE_SENSING:
				dmb();
				midi_active_sense_state = MIDI_ACTIVE_SENSE_ENABLED;
				/* no break */
			case MIDI_TYPES_CLOCK:
			case MIDI_TYPES_START:
			case MIDI_TYPES_CONTINUE:
			case MIDI_TYPES_STOP:
			case MIDI_TYPES_SYSTEM_RESET:
				// Here we will have to extract the one-byte message,
				// pass it to the structure for being read outside
				// the MIDI class, and recompose the message it was
				// interleaved into. Oh, and without killing the running status..
				// This is done by leaving the pending message as is,
				// it will be completed on next calls.
				midi_message.timestamp = timestamp;
				midi_message.type = serial_data;
				midi_message.data1 = 0;
				midi_message.data2 = 0;
				midi_message.channel = 0;
				midi_message.bytes_count = (uint8_t) 1;
				//midi_message.valid = true;
				return true;

				break;
				// End of Exclusive
			case 0xF7:
				if (midi_message.system_exclusive[0] == MIDI_TYPES_SYSTEM_EXCLUSIVE) {
					// Store the last byte (EOX)
					midi_message.system_exclusive[pending_message_index++] = 0xF7;
					midi_message.type = MIDI_TYPES_SYSTEM_EXCLUSIVE;
					// Get length
					midi_message.data1 = pending_message_index & 0xFF; // LSB
					midi_message.data2 = pending_message_index >> 8;   // MSB
					midi_message.channel = 0;
					midi_message.bytes_count = (uint8_t) pending_message_index;
					//midi_message.valid = true;

					reset_input();
					return true;
				} else {
					// Well well well.. error.
					reset_input();
					return false;
				}
				break;
			default:
				break;
			}
		}

		// Add extracted data byte to pending message
		if (pending_message[0] == MIDI_TYPES_SYSTEM_EXCLUSIVE) {
			midi_message.system_exclusive[pending_message_index] = serial_data;
		} else {
			pending_message[pending_message_index] = serial_data;
		}

		// Now we are going to check if we have reached the end of the message
		if (pending_message_index >= (pending_message_expected_lenght - 1)) {
			// "FML" case: fall down here with an overflown SysEx..
			// This means we received the last possible data byte that can fit
			// the buffer. If this happens, try increasing MidiMessage::sSysExMaxSize.
			if (pending_message[0] == MIDI_TYPES_SYSTEM_EXCLUSIVE) {
				reset_input();
				return false;
			}

			midi_message.timestamp = timestamp;
			midi_message.type = get_type_from_status_byte(pending_message[0]);

			if (is_channel_message(midi_message.type)) {
				midi_message.channel = get_channel_from_status_byte(pending_message[0]);
			} else {
				midi_message.channel = 0;
			}

			midi_message.data1 = pending_message[1];

			// Save data2 only if applicable
			if (pending_message_expected_lenght == 3) {
				midi_message.data2 = pending_message[2];
				midi_message.bytes_count = (uint8_t) 3;
			} else {
				midi_message.data2 = 0;
				midi_message.bytes_count = (uint8_t) 2;
			}

			// Reset local variables
			pending_message_index = 0;
			pending_message_expected_lenght = 0;

			//midi_message.valid = true;

			// Activate running status (if enabled for the received type)
			switch (midi_message.type) {
			case MIDI_TYPES_NOTE_OFF:
			case MIDI_TYPES_NOTE_ON:
			case MIDI_TYPES_AFTER_TOUCH_POLY:
			case MIDI_TYPES_CONTROL_CHANGE:
			case MIDI_TYPES_PROGRAM_CHANGE:
			case MIDI_TYPES_AFTER_TOUCH_CHANNEL:
			case MIDI_TYPES_PITCH_BEND:
				// Running status enabled: store it from received message
				running_status_rx = pending_message[0];
				break;

			default:
				// No running status
				running_status_rx = MIDI_TYPES_INVALIDE_TYPE;
				break;
			}
			return true;
		} else {
			// Then update the index of the pending message.
			pending_message_index++;

			if (USE_1_BYTE_PARSING) {
				// Message is not complete.
				return false;
			} else {
				// Call the parser recursively to parse the rest of the message.
				return parse();
			}
		}
	}
}


/**
 * @ingroup midi
 *
 * @return
 */
bool midi_read(void) {
	return midi_read_channel(input_channel);
}

/**
 * @ingroup midi
 *
 * @param inChannel
 * @return
 */
bool midi_read_channel(uint8_t channel) {
	if (channel >= (uint8_t) MIDI_CHANNEL_OFF)
		return false; // MIDI Input disabled.

	if (!parse())
		return false;

	handle_null_velocity_note_on_as_note_off();
	const bool channel_match = input_filter(channel);

	return channel_match;
}

/**
 * @ingroup midi
 *
 * @param tc
 */
void midi_send_tc(const struct _midi_send_tc *tc) {
	uint8_t data[10] = {0xF0, 0x7F, 0x7F, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0xF7};

	data[5] = (((tc->rate) & 0x03) << 5) | (tc->hour & 0x1F);
	data[6] = tc->minute & 0x3F;
	data[7] = tc->second & 0x3F;
	data[8] = tc->frame & 0x1F;

	midi_interface_f.send(data, (uint16_t) 10);
}

/**
 * @ingroup midi
 *
 * @param data
 * @param length
 */
void midi_send_raw(const uint8_t *data, const uint16_t length) {
	midi_interface_f.send(data, length);
}

/**
 *
 * @param clo
 */
static void irq_timer3_sense_handler(const uint32_t clo) {
	BCM2835_ST->C3 = clo + (uint32_t) 1000;
	dmb();
	if (midi_active_sense_state == MIDI_ACTIVE_SENSE_ENABLED) {
		midi_active_sense_timeout++;
		if (midi_active_sense_timeout > 300) { // > 300 ms
			// Turn All Notes Off
			dmb();
			midi_active_sense_state = MIDI_ACTIVE_SENSE_FAILED;
		}
	}
}

/**                             MIDI_INTERFACE_UART
 *
 */

/**
 * @ingroup midi_in
 *
 */
void __attribute__((interrupt("FIQ"))) fiq_midi_in_handler(void) {
	dmb();

	const uint32_t timestamp = BCM2835_ST->CLO;
	const uint32_t data = BCM2835_PL011->DR;

	midi_rx_buffer[midi_rx_buffer_index_head].data = (uint8_t) (data & 0xFF);
	midi_rx_buffer[midi_rx_buffer_index_head].timestamp = timestamp;
	midi_rx_buffer_index_head = (midi_rx_buffer_index_head + 1) & MIDI_RX_BUFFER_INDEX_MASK;

	dmb();
}

/**
 *
 * @return
 */
static const char *pl011_get(void) {
	return "UART";
}

/**
 * @ingroup midi
 *
 * Configure PL011 for MIDI transmission. Enable the UART.
 *
 */

#define PL011_BAUD_INT_3(x) 		(3000000 / (16 * (x)))
#define PL011_BAUD_FRAC_3(x) 		(int)((((3000000.0 / (16.0 * (x))) - PL011_BAUD_INT_3(x)) * 64.0) + 0.5)
#define PL011_BAUD_INT_48(x) 		(48000000 / (16 * (x)))
#define PL011_BAUD_FRAC_48(x) 		(int)((((48000000.0 / (16.0 * (x))) - PL011_BAUD_INT_48(x)) * 64.0) + 0.5)

static void pl011_init(void) {
	uint32_t ibrd = PL011_BAUD_INT_48(midi_baudrate);			// Default UART CLOCK 48Mhz
	uint32_t fbrd = PL011_BAUD_FRAC_48(midi_baudrate);			// Default UART CLOCK 48Mhz
	uint32_t cr = PL011_CR_UARTEN;

	dmb();

	// Work around BROADCOM firmware bug
	if (bcm2835_vc_get_clock_rate(BCM2835_VC_CLOCK_ID_UART) != 48000000) {
		(void) bcm2835_vc_set_clock_rate(BCM2835_VC_CLOCK_ID_UART, 3000000);// Set UART clock rate to 3000000 (3MHz)
		ibrd = PL011_BAUD_INT_3(midi_baudrate);
		fbrd = PL011_BAUD_FRAC_3(midi_baudrate);
	}

	BCM2835_PL011->CR = 0;												// Disable everything

    bcm2835_gpio_fsel(RPI_V2_GPIO_P1_08, BCM2835_GPIO_FSEL_ALT0);		// PL011_TXD
    bcm2835_gpio_fsel(RPI_V2_GPIO_P1_10, BCM2835_GPIO_FSEL_ALT0);		// PL011_RXD

	bcm2835_gpio_set_pud(RPI_V2_GPIO_P1_08, BCM2835_GPIO_PUD_OFF);		// Disable pull-up/down
	bcm2835_gpio_set_pud(RPI_V2_GPIO_P1_10, BCM2835_GPIO_PUD_OFF);		// Disable pull-up/down

	while ((BCM2835_PL011->FR & PL011_FR_BUSY) != 0)
		;																// Poll the "flags register" to wait for the UART to stop transmitting or receiving

	BCM2835_PL011->LCRH &= ~PL011_LCRH_FEN;								// Flush the transmit FIFO by marking FIFOs as disabled in the "line control register"
	BCM2835_PL011->ICR = 0x7FF;											// Clear all interrupt status
	BCM2835_PL011->IBRD = ibrd;
	BCM2835_PL011->FBRD = fbrd;
	BCM2835_PL011->LCRH = PL011_LCRH_WLEN8; 							// Set N, 8, 1, FIFO disabled
	dmb();

	if ((midi_direction & MIDI_DIRECTION_INPUT) == MIDI_DIRECTION_INPUT) {
		BCM2835_PL011->IMSC = PL011_IMSC_RXIM;
		BCM2835_IRQ->FIQ_CONTROL = (uint32_t) BCM2835_FIQ_ENABLE | (uint32_t) INTERRUPT_VC_UART;
		dmb();
		arm_install_handler((unsigned) fiq_midi_in_handler, ARM_VECTOR(ARM_VECTOR_FIQ));
		__enable_fiq();
		cr |= PL011_CR_RXE;
	}

	if ((midi_direction & MIDI_DIRECTION_OUTPUT) == MIDI_DIRECTION_OUTPUT) {
		BCM2835_PL011->LCRH |= PL011_LCRH_FEN;							// FIFO enabled
		cr |= PL011_CR_TXE;
	}

	BCM2835_PL011->CR = cr;												// Enable UART
	dmb();
}

/**
 *
 */
static void pl011_poll(void) {
	// Nothing to do here. We have the FIQ routine.
}

/**
 *
 * @param baudrate
 */
static void pl011_set(const uint32_t baudrate) {
	midi_baudrate = baudrate;
}

/**
 *
 * @param data
 * @param length
 */
static void pl011_send(const uint8_t *data, uint16_t length) {
	const uint8_t *p = data;

	while (length > 0) {
		while ((BCM2835_PL011->FR & PL011_FR_TXFF) == PL011_FR_TXFF) {
		}
		BCM2835_PL011->DR = (uint32_t) *p;
		p++;
		length--;
	}
}

/**                             MIDI_INTERFACE_SPI
 *
 */

static const char *spi_get(void) {
	return "SPI";
}

/**
 *
 */
static void spi_init(void) {
	spi_device_info.chip_select = 0;
	spi_device_info.speed_hz = SC16IS7X0_SPI_SPEED_DEFAULT_HZ;

	sc16is740_start(&spi_device_info);
	sc16is740_set_baud(&spi_device_info, midi_baudrate);
}

/**
 *
 */
static void spi_poll(void) {
	int c;
	if ( (c = sc16is740_getc(&spi_device_info)) != -1) {
		midi_rx_buffer[midi_rx_buffer_index_head].data = (uint8_t) (c & 0xFF);
		midi_rx_buffer[midi_rx_buffer_index_head].timestamp = BCM2835_ST->CLO;
		midi_rx_buffer_index_head = (midi_rx_buffer_index_head + 1) & MIDI_RX_BUFFER_INDEX_MASK;
	}
}

/**
 *
 * @param baudrate
 */
static void spi_set(const uint32_t baudrate) {
	sc16is740_set_baud(&spi_device_info, baudrate);
	midi_baudrate = baudrate;
}

/**
 *
 * @param data
 * @param length
 */
static void spi_send(const uint8_t *data, uint16_t length) {
	sc16is740_write(&spi_device_info, data, (unsigned) length);
}

/**
 * * @ingroup midi
 *
 */
void midi_init(const _midi_direction dir) {
	uint32_t i = 0;

	midi_direction = dir;

	if ((dir & MIDI_DIRECTION_INPUT) == MIDI_DIRECTION_INPUT) {
		for (i = 0; i < (uint32_t) MIDI_RX_BUFFER_INDEX_ENTRIES; i++) {
			midi_rx_buffer[i].data = (uint8_t) 0;
			midi_rx_buffer[i].timestamp = (uint32_t) 0;
		}

		midi_rx_buffer_index_head = (uint16_t) 0;
		midi_rx_buffer_index_tail = (uint16_t) 0;

		if (midi_active_sense) {
			irq_timer_init();
			irq_timer_set(IRQ_TIMER_3, irq_timer3_sense_handler);
			BCM2835_ST->C3 = BCM2835_ST->CLO + (uint32_t) 1000;
		}

		reset_input();
	}

	if ((dir == MIDI_DIRECTION_OUTPUT) & MIDI_DIRECTION_OUTPUT) {

	}

	midi_interface_f.init();
}
