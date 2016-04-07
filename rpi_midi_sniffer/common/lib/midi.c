/**
 * @file midi.c
 *
 * @brief This file implements the MIDI receive state-machine.
 *
 */
/* This code is inspired by:
 *
 *  @file       MIDI.cpp
 *  Project     Arduino MIDI Library
 *  @brief      MIDI Library for the Arduino
 *  @version    4.2
 *  @author     Francois Best
 *  @license    MIT - Copyright (c) 2015 Francois Best
 *
 *  https://github.com/FortySevenEffects/arduino_midi_library/blob/master/src/MIDI.cpp
 */
/* Copyright (C) 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <string.h>

#include "midi.h"
#include "util.h"
#include "arm/synchronize.h"

#if defined (MIDI_INTERFACE_UART)
#include "bcm2835.h"
#include "bcm2835_gpio.h"
#include "bcm2835_vc.h"
#include "arm/pl011.h"
#endif

#if defined (MIDI_INTERFACE_SPI)
#include "sc16is740.h"
#include "device_info.h"
#endif

#if defined (MIDI_DMX_BRIDGE) && defined (MIDI_INTERFACE_UART)
#error The UART is not available for this configuration
#endif

static struct _midi_message midi_message ALIGNED;								///<

static volatile uint8_t midi_rx_buffer[MIDI_RX_BUFFER_INDEX_ENTRIES] ALIGNED;	///<
static volatile uint16_t midi_rx_buffer_index_head = (uint16_t) 0;				///<
static volatile uint16_t midi_rx_buffer_index_tail = (uint16_t) 0;				///<

static uint8_t input_channel = MIDI_CHANNEL_OMNI;								///<
static uint8_t pending_message_index = (uint8_t) 0;								///<
static uint8_t pending_message_expected_lenght = (uint8_t) 0;					///<
static uint8_t running_status_rx = MIDI_TYPES_INVALIDE_TYPE;					///<
static uint8_t pending_message[8] ALIGNED;										///<

static uint32_t midi_baudrate = 115200;											///<

static uint16_t midi_active_sense_timeout = 0;									///<
//bool static midi_active_sense_flag = false;										///<
//bool static midi_active_sense_failed = false;									///<
_midi_active_sense_state midi_active_sense_state = MIDI_ACTIVE_SENSE_NOT_ENABLED;

#if defined (MIDI_INTERFACE_UART)
void pl011_init(void);
void pl011_set(const uint32_t);
void pl011_poll(void);
const char * pl011_get(void);
#endif

#if defined (MIDI_INTERFACE_SPI)
void spi_init(void);
void spi_set(const uint32_t);
void spi_poll(void);
const char * spi_get(void);

static device_info_t spi_device_info;
#endif

struct _midi_interface {
	void (*init)(void);				///< Pointer to function for
	void (*set)(const uint32_t);	///< Pointer to function for
	void (*poll)();					///< Pointer to function for
	const char * (*get)();			///< Pointer to function for
};

static struct _midi_interface midi_interface_f;
static struct _midi_interface midi_interfaces_f[] = {
		{ spi_init, spi_set, spi_poll, spi_get  }
#if defined (MIDI_INTERFACE_UART)
		, { pl011_init, pl011_set, pl011_poll, pl011_get  }
#endif

};

void midi_set_interface(const uint8_t interface) {

	if (interface < sizeof(midi_interfaces_f) / sizeof(midi_interfaces_f[0])) {
		midi_interface_f.init = midi_interfaces_f[interface].init;
		midi_interface_f.set = midi_interfaces_f[interface].set;
		midi_interface_f.poll = midi_interfaces_f[interface].poll;
		midi_interface_f.get = midi_interfaces_f[interface].get;
		return;
	}

#if defined (MIDI_DMX_BRIDGE)
	midi_interface_f.init = spi_init;
	midi_interface_f.set = spi_set;
	midi_interface_f.poll = spi_poll;
	midi_interface_f.get = spi_get;
#else
	midi_interface_f.init = pl011_init;
	midi_interface_f.set = pl011_set;
	midi_interface_f.poll = pl011_poll;
	midi_interface_f.get = pl011_get;
#endif

}

/**
 * @ingroup midi
 *
 */
static void reset_input(void) {
	pending_message_index = 0;
	pending_message_expected_lenght = 0;
	running_status_rx = MIDI_TYPES_INVALIDE_TYPE;
}

/**
 *
 */
void midi_init(void) {
	uint32_t i = 0;

	for (i = 0 ; i < MIDI_RX_BUFFER_INDEX_ENTRIES ; i++) {
		midi_rx_buffer[i] = (uint8_t) 0;
	}

	midi_rx_buffer_index_head = (uint16_t) 0;
	midi_rx_buffer_index_tail = (uint16_t) 0;

#if	defined (MIDI_SNIFFER)
	BCM2835_ST->C3 = BCM2835_ST->CLO + (uint32_t) 1000;
	BCM2835_ST->CS = BCM2835_ST_CS_M3;
	BCM2835_IRQ->IRQ_ENABLE1 = BCM2835_TIMER3_IRQn;

	__enable_irq();
#endif

	reset_input();

	midi_interface_f.init();

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
 * @ingroup midi
 *
 */
void midi_poll(void) {
	midi_interface_f.poll();
}

/**
 *
 * @return
 */
const char *midi_get_description(void) {
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
 * @ingroup midi
 *
 * @param byte
 * @return
 */
static bool raw_read(uint8_t *byte) {
	dmb();
	if (midi_rx_buffer_index_head != midi_rx_buffer_index_tail) {
		*byte = midi_rx_buffer[midi_rx_buffer_index_tail];
		midi_rx_buffer_index_tail = (midi_rx_buffer_index_tail + 1) & MIDI_RX_BUFFER_INDEX_MASK;
		return true;
	} else {
		return false;
	}
}

/**
 * @ingroup midi
 *
 * @return
 */
uint8_t midi_get_input_channel(void) {
	return input_channel;
}

/**
 *
 * @param channel
 */
void midi_set_input_channel(uint8_t channel) {
	input_channel = channel;
}

/**
 *
 * @return
 */
_midi_active_sense_state midi_get_active_sense_state(void) {
	return midi_active_sense_state;
}

/**
 * @ingroup midi
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
 * @ingroup midi
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
 * @ingroup midi
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
		if ((midi_message.channel == input_channel) || (input_channel == MIDI_CHANNEL_OMNI)) {
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
 * @ingroup midi
 *
 */
static bool parse(void) {
	uint8_t serial_data;
    if (!raw_read(&serial_data)) {
        // No data available.
        return false;
	}

    midi_active_sense_timeout = 0;

	if (pending_message_index == (uint8_t) 0) {
		// Start a new pending message
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
			midi_active_sense_state = MIDI_ACTIVE_SENSE_ENABLED;
			/* no break */
		case MIDI_TYPES_START:
		case MIDI_TYPES_CONTINUE:
		case MIDI_TYPES_STOP:
		case MIDI_TYPES_CLOCK:
		case MIDI_TYPES_SYSTEM_RESET:
		case MIDI_TYPES_TUNE_REQUEST:
			// Handle the message type directly here.
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
			// The message can be any lenght
			// between 3 and MIDI_SYSTEM_EXCLUSIVE_INDEX_ENTRIES
			pending_message_expected_lenght = MIDI_SYSTEM_EXCLUSIVE_INDEX_ENTRIES;
			running_status_rx = MIDI_TYPES_INVALIDE_TYPE;
			midi_message.system_exclusive[0] = MIDI_TYPES_SYSTEM_EXCLUSIVE;
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
				if (midi_message.system_exclusive[0]
						== MIDI_TYPES_SYSTEM_EXCLUSIVE) {
					// Store the last byte (EOX)
					midi_message.system_exclusive[pending_message_index++] = 0xF7;
					midi_message.type = MIDI_TYPES_SYSTEM_EXCLUSIVE;
					// Get length
					midi_message.data1 = pending_message_index & 0xFF; // LSB
					midi_message.data2 = pending_message_index >> 8;   // MSB
					midi_message.channel = 0;
					midi_message.bytes_count = (uint8_t) 3;
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
	if (channel >= MIDI_CHANNEL_OFF)
		return false; // MIDI Input disabled.

	if (!parse())
		return false;

	handle_null_velocity_note_on_as_note_off();
	const bool channel_match = input_filter(channel);

	return channel_match;
}

#if defined (MIDI_SNIFFER)
void __attribute__((interrupt("IRQ"))) c_irq_handler(void) {
	dmb();

	if (BCM2835_ST->CS & BCM2835_ST_CS_M3) {
		BCM2835_ST->CS = BCM2835_ST_CS_M3;
		BCM2835_ST->C3 = BCM2835_ST->CLO + (uint32_t)1000;
		if (midi_active_sense_state == MIDI_ACTIVE_SENSE_ENABLED) {
			midi_active_sense_timeout++;
			if (midi_active_sense_timeout > 300) { // > 300 ms
				// Turn All Notes Off
				midi_active_sense_state = MIDI_ACTIVE_SENSE_FAILED;
			}
		}
	}

	dmb();
}
#endif

/**                             MIDI_INTERFACE_UART
 *
 */

#if defined (MIDI_INTERFACE_UART)

/**
 * @ingroup midi
 *
 */
void __attribute__((interrupt("FIQ"))) c_fiq_handler(void) {
	dmb();

	if (BCM2835_PL011 -> MIS & PL011_MIS_RXMIS ) {
		do {
			const uint32_t data = BCM2835_PL011 ->DR;
			midi_rx_buffer[midi_rx_buffer_index_head] = (uint8_t) (data & 0xFF);
			midi_rx_buffer_index_head = (midi_rx_buffer_index_head + 1) & MIDI_RX_BUFFER_INDEX_MASK;
		} while (!(BCM2835_PL011 ->FR & PL011_FR_RXFE));
	}

	dmb();
}

/**
 *
 * @return
 */
const char *pl011_get(void) {
	return "UART";
}

/**
 * @ingroup midi
 *
 * Configure PL011 for MIDI transmission. Enable the UART.
 *
 */
void pl011_init(void) {
	uint32_t value;

	dmb();

	(void) bcm2835_vc_set_clock_rate(BCM2835_VC_CLOCK_ID_UART, 3000000);// Set UART clock rate to 3000000 (3MHz)
	BCM2835_PL011->CR = 0;												// Disable everything
	value = BCM2835_GPIO->GPFSEL1;
	value &= ~(7 << 12);
	value |= BCM2835_GPIO_FSEL_ALT0 << 12;								// Pin 14 PL011_TXD
	value &= ~(7 << 15);
	value |= BCM2835_GPIO_FSEL_ALT0 << 15;								// Pin 15 PL011_RXD
	BCM2835_GPIO->GPFSEL1 = value;
	bcm2835_gpio_set_pud(RPI_V2_GPIO_P1_08, BCM2835_GPIO_PUD_OFF);		// Disable pull-up/down
	bcm2835_gpio_set_pud(RPI_V2_GPIO_P1_10, BCM2835_GPIO_PUD_OFF);		// Disable pull-up/down
	while ((BCM2835_PL011->FR & PL011_FR_BUSY) != 0)
		;																// Poll the "flags register" to wait for the UART to stop transmitting or receiving
	BCM2835_PL011->LCRH &= ~PL011_LCRH_FEN;								// Flush the transmit FIFO by marking FIFOs as disabled in the "line control register"
	BCM2835_PL011->ICR = 0x7FF;											// Clear all interrupt status
	BCM2835_PL011->IBRD = PL011_BAUD_INT(midi_baudrate);
	BCM2835_PL011->FBRD = PL011_BAUD_FRAC(midi_baudrate);
	BCM2835_PL011->LCRH = PL011_LCRH_WLEN8; 							// Set N, 8, 1, FIFO disabled
	BCM2835_PL011->CR = 0x301;											// Enable UART

	BCM2835_PL011->IMSC = PL011_IMSC_RXIM;
	BCM2835_IRQ->FIQ_CONTROL = (uint32_t) BCM2835_FIQ_ENABLE | (uint32_t) INTERRUPT_VC_UART;

	dmb();

	__enable_fiq();
}

/**
 *
 */
void pl011_poll(void) {
	// Nothing to do here. We have the FIQ routine.
}

void pl011_set(const uint32_t baudrate) {
	midi_baudrate = baudrate;
}

#endif

/**                             MIDI_INTERFACE_SPI
 *
 */

#if defined (MIDI_INTERFACE_SPI)

const char *spi_get(void) {
	return "SPI";
}

/**
 *
 */
void spi_init(void) {
	spi_device_info.chip_select = 0;
	spi_device_info.speed_hz = 100000;

	sc16is740_start(&spi_device_info);
	sc16is740_set_baud(&spi_device_info, midi_baudrate);
}

/**
 *
 */
void spi_poll(void) {
	int c;
	if ( (c = sc16is740_getc(&spi_device_info)) != -1) {
		midi_rx_buffer[midi_rx_buffer_index_head] = (uint8_t) (c & 0xFF);
		midi_rx_buffer_index_head = (midi_rx_buffer_index_head + 1) & MIDI_RX_BUFFER_INDEX_MASK;
	}
}

/**
 *
 * @param baudrate
 */
void spi_set(const uint32_t baudrate) {
	sc16is740_set_baud(&spi_device_info, baudrate);
	midi_baudrate = baudrate;
}

#endif


