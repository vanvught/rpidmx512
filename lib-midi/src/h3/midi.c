/**
 * @file midi.c
 *
 */
/* Copyright (C) 2016-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "arm/arm.h"
#include "arm/synchronize.h"

#include "irq_timer.h"

#include "arm/gic.h"

#include "h3.h"
#include "h3_ccu.h"
#include "h3_gpio.h"
#include "h3_timer.h"
#include "h3_hs_timer.h"

#include "uart.h"

struct _midi_receive {
	uint32_t timestamp;
	uint8_t data;
};

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

static struct _midi_message midi_message ALIGNED;
static uint8_t input_channel = MIDI_CHANNEL_OMNI;
static uint8_t pending_message_index = 0;
static uint8_t pending_message_expected_lenght = 0;
static uint8_t running_status_rx = MIDI_TYPES_INVALIDE_TYPE;
static uint8_t pending_message[8] ALIGNED;
static uint32_t midi_baudrate = MIDI_BAUDRATE_DEFAULT;
static bool midi_active_sense = false;
static _midi_direction midi_direction = MIDI_DIRECTION_INPUT;

/*
 * IRQ UART2
 */
static volatile struct _midi_receive midi_rx_buffer[MIDI_RX_BUFFER_INDEX_ENTRIES] ALIGNED;
static volatile uint32_t midi_rx_buffer_index_head = 0;
static volatile uint32_t midi_rx_buffer_index_tail = 0;
/*
 * IRQ Timer0
 */
static volatile uint32_t updates_per_second = 0;
static volatile uint32_t updates_previous = 0;
static volatile uint32_t updates = 0;
/*
 * IRQ Timer1
 */
static volatile uint16_t midi_active_sense_timeout = 0;
static volatile _midi_active_sense_state midi_active_sense_state = MIDI_ACTIVE_SENSE_NOT_ENABLED;

void uart2_send(const uint8_t *data, uint16_t length) ;

uint32_t midi_get_updates_per_seconds(void) {
	dmb();
	return updates_per_second;
}

_midi_direction midi_get_direction(void) {
	return midi_direction;
}

bool midi_active_get_sense(void) {
	return midi_active_sense;
}

void midi_active_set_sense(bool sense) {
	midi_active_sense = sense;
}

static void reset_input(void) {
	pending_message_index = 0;
	pending_message_expected_lenght = 0;
	running_status_rx = MIDI_TYPES_INVALIDE_TYPE;
}

void midi_set_baudrate(uint32_t baudrate) {
	midi_baudrate = baudrate;
}

uint32_t midi_get_baudrate(void) {
	return midi_baudrate;
}

const char *midi_get_interface_description(void) {
	return "UART2";
}

struct _midi_message *midi_message_get(void) {
	return &midi_message;
}

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

uint8_t midi_get_input_channel(void) {
	return input_channel;
}

void midi_set_input_channel(uint8_t channel) {
	input_channel = channel;
}

_midi_active_sense_state midi_active_get_sense_state(void) {
	dmb();
	return midi_active_sense_state;
}

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

static uint8_t get_channel_from_status_byte(uint8_t status_byte) {
	return (status_byte & 0x0F) + 1;
}

static bool is_channel_message(uint8_t type) {
	return (type == MIDI_TYPES_NOTE_OFF
			|| type == MIDI_TYPES_NOTE_ON
			|| type == MIDI_TYPES_CONTROL_CHANGE
			|| type == MIDI_TYPES_AFTER_TOUCH_POLY
			|| type == MIDI_TYPES_AFTER_TOUCH_CHANNEL
			|| type == MIDI_TYPES_PITCH_BEND
			|| type == MIDI_TYPES_PROGRAM_CHANGE);
}

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

static void handle_null_velocity_note_on_as_note_off(void) {
	if (HANDLE_NULL_VELOCITY_NOTE_ON_AS_NOTE_OFF
			&& (midi_message.type == MIDI_TYPES_NOTE_ON)
			&& (midi_message.data2 == 0)) {
		midi_message.type = MIDI_TYPES_NOTE_OFF;
	}
}

static bool parse(void) {
	uint8_t serial_data;
	uint32_t timestamp;
	uint32_t pending_timestamp;

    if (!raw_read(&serial_data, &timestamp)) {
        return false;
	}

    midi_active_sense_timeout = 0;

	if (pending_message_index == 0) {
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
				pending_message_index = 1;
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
			 __attribute__ ((fallthrough));
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
			midi_message.channel = 0;
			midi_message.data1 = 0;
			midi_message.data2 = 0;
			midi_message.bytes_count = 1;
			//midi_message.valid = true;
			// \fix Running Status broken when receiving Clock messages.
			// Do not reset all input attributes, Running Status must remain unchanged.
			//resetInput();
			// We still need to reset these
			pending_message_index = 0;
			pending_message_expected_lenght = 0;
			return true;
			break;
		// 2 bytes messages
		case MIDI_TYPES_PROGRAM_CHANGE:
		case MIDI_TYPES_AFTER_TOUCH_CHANNEL:
		case MIDI_TYPES_TIME_CODE_QUARTER_FRAME:
		case MIDI_TYPES_SONG_SELECT:
			pending_message_expected_lenght = 2;
			break;
		// 3 bytes messages
		case MIDI_TYPES_NOTE_ON:
		case MIDI_TYPES_NOTE_OFF:
		case MIDI_TYPES_CONTROL_CHANGE:
		case MIDI_TYPES_PITCH_BEND:
		case MIDI_TYPES_AFTER_TOUCH_POLY:
		case MIDI_TYPES_SONG_POSITION:
			pending_message_expected_lenght = 3;
			break;
		case MIDI_TYPES_SYSTEM_EXCLUSIVE:
			// The message can be any length
			// between 3 and MIDI_SYSTEM_EXCLUSIVE_INDEX_ENTRIES
			pending_message_expected_lenght = MIDI_SYSTEM_EXCLUSIVE_INDEX_ENTRIES;
			running_status_rx = MIDI_TYPES_INVALIDE_TYPE;
			midi_message.system_exclusive[0] = MIDI_TYPES_SYSTEM_EXCLUSIVE;
			midi_message.channel = 0;
			midi_message.timestamp = pending_timestamp;
			break;
		case MIDI_TYPES_INVALIDE_TYPE:
		default:
			// This is obviously wrong. Let's get the hell out'a here.
			reset_input();
			return false;
			break;
		}

		if (pending_message_index >= (pending_message_expected_lenght - 1)) {
			// Reception complete
			midi_message.timestamp = pending_timestamp;
			midi_message.type = get_type_from_status_byte(pending_message[0]);
			midi_message.channel = get_channel_from_status_byte(pending_message[0]);
			midi_message.data1 = pending_message[1];

			/* Save data2 only if applicable */
			if (pending_message_expected_lenght == 3) {
				midi_message.data2 = pending_message[2];
				midi_message.bytes_count = 3;
			} else {
				midi_message.data2 = 0;
				midi_message.bytes_count = 2;
			}
			pending_message_index = 0;
			pending_message_expected_lenght = 0;

			updates++;
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
				 __attribute__ ((fallthrough));
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
				midi_message.bytes_count = 1;
				//midi_message.valid = true;
				return true;

				break;
				// End of Exclusive
			case 0xF7:
				if (midi_message.system_exclusive[0] == MIDI_TYPES_SYSTEM_EXCLUSIVE) {
					// Store the last byte (EOX)
					midi_message.system_exclusive[pending_message_index++] = 0xF7;
					midi_message.type = MIDI_TYPES_SYSTEM_EXCLUSIVE;
					/* Get length */
					midi_message.data1 = pending_message_index & 0xFF; // LSB
					midi_message.data2 = pending_message_index >> 8;   // MSB
					midi_message.channel = 0;
					midi_message.bytes_count = pending_message_index;

					updates++;

					reset_input();
					return true;
				} else {
					/* Error */
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
				midi_message.bytes_count = 3;
			} else {
				midi_message.data2 = 0;
				midi_message.bytes_count = 2;
			}

			// Reset local variables
			pending_message_index = 0;
			pending_message_expected_lenght = 0;

			updates++;

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

bool midi_read(void) {
	return midi_read_channel(input_channel);
}

bool midi_read_channel(uint8_t channel) {
	if (channel >= MIDI_CHANNEL_OFF)
		return false; // MIDI Input disabled.

	if (!parse())
		return false;

	handle_null_velocity_note_on_as_note_off();
	const bool channel_match = input_filter(channel);

	return channel_match;
}

void midi_send_tc(const struct _midi_send_tc *tc) {
	uint8_t data[10] = {0xF0, 0x7F, 0x7F, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0xF7};

	data[5] = (((tc->nType) & 0x03) << 5) | (tc->nHours & 0x1F);
	data[6] = tc->nMinutes & 0x3F;
	data[7] = tc->nSeconds & 0x3F;
	data[8] = tc->nFrames & 0x1F;

	uart2_send(data, 10);
}

void midi_send_qf(uint8_t value) {
	uint8_t data[2];

	data[0] = 0xF1;
	data[1] = value;

	uart2_send(data, 2);
}

void midi_send_raw(const uint8_t *data, uint16_t length) {
	uart2_send(data, length);
}

static void __attribute__((interrupt("IRQ"))) irq_midi_in_handler(void) {
	dmb();

	const uint32_t timestamp = h3_hs_timer_lo_us();
	const uint32_t irq = H3_GIC_CPUIF->AIA;

	if (H3_UART2->O08.IIR & UART_IIR_IID_RD) {

		midi_rx_buffer[midi_rx_buffer_index_head].data = (H3_UART2->O00.RBR & 0xFF);
		midi_rx_buffer[midi_rx_buffer_index_head].timestamp = timestamp;
		midi_rx_buffer_index_head = (midi_rx_buffer_index_head + 1) & MIDI_RX_BUFFER_INDEX_MASK;

		H3_GIC_CPUIF->AEOI = H3_UART2_IRQn;
		gic_unpend(H3_UART2_IRQn);
	}

	if (irq == H3_TIMER0_IRQn) {
		H3_TIMER->IRQ_STA = TIMER_IRQ_PEND_TMR0;	/* Clear Timer 0 Pending bit */

		updates_per_second = updates - updates_previous;
		updates_previous = updates;

		H3_GIC_CPUIF->AEOI = H3_TIMER0_IRQn;
		gic_unpend(H3_TIMER0_IRQn);
	} else if (irq == H3_TIMER1_IRQn) {
		H3_TIMER->IRQ_STA = TIMER_IRQ_PEND_TMR1;	/* Clear Timer 1 Pending bit */

		if (midi_active_sense_state == MIDI_ACTIVE_SENSE_ENABLED) {
			midi_active_sense_timeout++;
			if (midi_active_sense_timeout > 300) { /* > 300 ms */
				/* Turn All Notes Off */
				dmb();
				midi_active_sense_state = MIDI_ACTIVE_SENSE_FAILED;
			}
		}

		H3_GIC_CPUIF->AEOI = H3_TIMER1_IRQn;
		gic_unpend(H3_TIMER1_IRQn);
	}

	dmb();
}

void uart2_send(const uint8_t *data, uint16_t length) {
	const uint8_t *p = data;

	while (length > 0) {
		uint32_t available = 64 - H3_UART2->TFL;

		while ((length > 0) && (available > 0)) {
			H3_UART2->O00.THR = (uint32_t) *p;
			length--;
			available--;
			p++;
		}
	}
}

static void uart2_init(void) {
	uint32_t value = H3_PIO_PORTA->CFG0;
	// PA0, TX
	value &= (uint32_t) ~(GPIO_SELECT_MASK << PA0_SELECT_CFG0_SHIFT);
	value |= H3_PA0_SELECT_UART2_TX << PA0_SELECT_CFG0_SHIFT;
	// PA1, RX
	value &= (uint32_t) ~(GPIO_SELECT_MASK << PA1_SELECT_CFG0_SHIFT);
	value |= H3_PA1_SELECT_UART2_RX << PA1_SELECT_CFG0_SHIFT;
	H3_PIO_PORTA->CFG0 = value;

	H3_CCU->BUS_SOFT_RESET4 |= CCU_BUS_SOFT_RESET4_UART2;
	H3_CCU->BUS_CLK_GATING3 |= CCU_BUS_CLK_GATING3_UART2;

	H3_UART2->O08.FCR = 0;
	H3_UART2->LCR = UART_LCR_DLAB;

	if (midi_baudrate == 0) {
		H3_UART2->O00.DLL = BAUD_31250_L;
	} else {
		H3_UART2->O00.DLL = (24000000 / 16) / midi_baudrate;
	}

	H3_UART2->O04.DLH = 0;
	H3_UART2->O04.IER = 0;
	H3_UART2->LCR = UART_LCR_8_N_1;

	dmb();

	while ((H3_UART2->USR & UART_USR_BUSY) == UART_USR_BUSY) {
		(void) H3_UART2->O00.RBR;
	}

	if ((midi_direction & MIDI_DIRECTION_INPUT) == MIDI_DIRECTION_INPUT) {
		H3_UART2->O08.FCR = 0;
		H3_UART2->O04.IER = UART_IER_ERBFI;

		gic_irq_config(H3_UART2_IRQn, GIC_CORE0);

		__enable_irq();
		dmb();
	}

	if ((midi_direction & MIDI_DIRECTION_OUTPUT) == MIDI_DIRECTION_OUTPUT) {
		H3_UART2->O08.FCR = UART_FCR_EFIFO | UART_FCR_TRESET;
		H3_UART2->O04.IER = 0;
	}
}

void midi_init(_midi_direction dir) {
	uint32_t i;

	midi_direction = dir;

	if ((dir & MIDI_DIRECTION_INPUT) == MIDI_DIRECTION_INPUT) {
		for (i = 0; i < (uint32_t) MIDI_RX_BUFFER_INDEX_ENTRIES; i++) {
			midi_rx_buffer[i].data = 0;
			midi_rx_buffer[i].timestamp = 0;
		}

		midi_rx_buffer_index_head = 0;
		midi_rx_buffer_index_tail = 0;

		/*
		 * Statistics
		 */
		gic_irq_config(H3_TIMER0_IRQn, GIC_CORE0);

		H3_TIMER->TMR0_CTRL = 0x14;						/* Select continuous mode, 24MHz clock source, 2 pre-scale */
		H3_TIMER->TMR0_INTV = 0xB71B00; 				/* 1 second */
		H3_TIMER->TMR0_CTRL &= ~(TIMER_CTRL_SINGLE_MODE);
		H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);
		H3_TIMER->IRQ_EN |= TIMER_IRQ_EN_TMR0;			/* Enable Timer 0 Interrupts */

		/*
		 * Active Sense
		 */
		if (midi_active_sense) {
			gic_irq_config(H3_TIMER1_IRQn, GIC_CORE0);

			H3_TIMER->TMR1_CTRL = 0x14;					/* Select continuous mode, 24MHz clock source, 2 pre-scale */
			H3_TIMER->TMR1_INTV = 12000; 				/*  1 ms TODO 10ms */
			H3_TIMER->TMR1_CTRL &= ~(TIMER_CTRL_SINGLE_MODE);
			H3_TIMER->TMR1_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);
			H3_TIMER->IRQ_EN |= TIMER_IRQ_EN_TMR1;		/* Enable Timer 1 Interrupts */
		}

		arm_install_handler((unsigned) irq_midi_in_handler, ARM_VECTOR(ARM_VECTOR_IRQ));

		reset_input();
	}

	if ((dir & MIDI_DIRECTION_OUTPUT) == MIDI_DIRECTION_OUTPUT) {
		midi_active_sense = false;	//TODO Implement output active sense
	}

	uart2_init();
}
