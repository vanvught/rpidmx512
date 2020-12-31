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

/**
 * NoteOn with 0 velocity should be handled as NoteOf.
 * Set to true  to get NoteOff events when receiving null-velocity NoteOn messages.
 * Set to false to get NoteOn  events when receiving null-velocity NoteOn messages.
 */
#define HANDLE_NULL_VELOCITY_NOTE_ON_AS_NOTE_OFF	true
/**
 * Setting this to true will make midi_read parse only one byte of data for each
 * call when data is available. This can speed up your application if receiving
 * a lot of traffic, but might induce MIDI Thru and treatment latency.
 */
#define USE_1_BYTE_PARSING							true

using namespace midi;

#define MIDI_RX_BUFFER_INDEX_ENTRIES	(1U << 6)
#define MIDI_RX_BUFFER_INDEX_MASK 		(MIDI_RX_BUFFER_INDEX_ENTRIES - 1)

/*
 * IRQ UART2
 */
struct MidiReceive {
	uint32_t nTimestamp;
	uint8_t nData;
};

static volatile struct MidiReceive midi_rx_buffer[MIDI_RX_BUFFER_INDEX_ENTRIES] __attribute__ ((aligned (4)));
static volatile uint32_t midi_rx_buffer_index_head = 0;
static volatile uint32_t midi_rx_buffer_index_tail = 0;
/*
 * IRQ Timer0
 */
static volatile uint32_t nUpdatesPerSecond = 0;
static volatile uint32_t nUpdatesPrevious = 0;
static volatile uint32_t nUpdates = 0;
/*
 * IRQ Timer1
 */
static volatile uint16_t nActiveSenseTimeout = 0;
static volatile ActiveSenseState midi_active_sense_state = ActiveSenseState::NOT_ENABLED;

uint32_t Midi::GetUpdatesPerSeconde(void) {
	dmb();
	return nUpdatesPerSecond;
}

void Midi::ResetInput(void) {
	m_nPendingMessageIndex = 0;
	m_nPendingMessageExpectedLenght = 0;
	m_nRunningStatusRx = static_cast<uint8_t>(Types::INVALIDE_TYPE);
}

bool Midi::ReadRaw(uint8_t *pByte, uint32_t *pTimestamp) {
	dmb();
	if (midi_rx_buffer_index_head != midi_rx_buffer_index_tail) {
		*pByte = midi_rx_buffer[midi_rx_buffer_index_tail].nData;
		*pTimestamp = midi_rx_buffer[midi_rx_buffer_index_tail].nTimestamp;
		midi_rx_buffer_index_tail = (midi_rx_buffer_index_tail + 1) & MIDI_RX_BUFFER_INDEX_MASK;
		return true;
	} else {
		return false;
	}
}

ActiveSenseState Midi::GetActiveSenseState(void) {
	dmb();
	return midi_active_sense_state;
}

Types Midi::GetTypeFromStatusByte(uint8_t nStatusByte) const {
	if ((nStatusByte < 0x80) || (nStatusByte == 0xf4) || (nStatusByte == 0xf5)
			|| (nStatusByte == 0xf9) || (nStatusByte == 0xfD)) {
		// Data bytes and undefined.
		return Types::INVALIDE_TYPE;
	}

	if (nStatusByte < 0xF0) {
		// Channel message, remove channel nibble.
		return static_cast<Types>(nStatusByte & 0xF0);
	}

	return static_cast<Types>(nStatusByte);
}

uint8_t Midi::GetChannelFromStatusByte(uint8_t nStatusByte) const {
	return (nStatusByte & 0x0F) + 1;
}

bool Midi::isChannelMessage(Types nType) const {
	return nType == Types::NOTE_OFF || nType == Types::NOTE_ON
			|| nType == Types::CONTROL_CHANGE
			|| nType == Types::AFTER_TOUCH_POLY
			|| nType == Types::AFTER_TOUCH_CHANNEL || nType == Types::PITCH_BEND
			|| nType == Types::PROGRAM_CHANGE;
}

bool Midi::InputFilter(uint8_t nChannel) const {
	if (m_Message.tType == Types::INVALIDE_TYPE)
		return false;

	// First, check if the received message is Channel
	if (m_Message.tType >= Types::NOTE_OFF && m_Message.tType <= Types::PITCH_BEND) {
		// Then we need to know if we listen to it
		if ((m_Message.nChannel == nChannel) || (nChannel == static_cast<uint8_t>(Channel::OMNI))) {
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

void Midi::HandleNullVelocityNoteOnAsNoteOff() {
	if (HANDLE_NULL_VELOCITY_NOTE_ON_AS_NOTE_OFF
			&& (m_Message.tType == Types::NOTE_ON) && (m_Message.nData2 == 0)) {
		m_Message.tType = Types::NOTE_OFF;
	}
}

bool Midi::Parse() {
	uint8_t nSerialData;
	uint32_t nTimestamp;
	uint32_t nPendingTimestamp;

    if (!ReadRaw(&nSerialData, &nTimestamp)) {
        return false;
	}

    nActiveSenseTimeout = 0;

	if (m_nPendingMessageIndex == 0) {
		// Start a new pending message
		nPendingTimestamp = nTimestamp;
		m_aPendingMessage[0] = nSerialData;

		// Check for running status first
		if (isChannelMessage(GetTypeFromStatusByte(m_nRunningStatusRx))) {
			// Only these types allow Running Status
			// If the status byte is not received, prepend it
			// to the pending message
			if (nSerialData < 0x80) {
				m_aPendingMessage[0] = m_nRunningStatusRx;
				m_aPendingMessage[1] = nSerialData;
				m_nPendingMessageIndex = 1;
			}
			// Else: well, we received another status byte,
			// so the running status does not apply here.
			// It will be updated upon completion of this message.
		}

		switch (GetTypeFromStatusByte(m_aPendingMessage[0])) {
		// 1 byte messages
		case Types::ACTIVE_SENSING:
			dmb();
			midi_active_sense_state = ActiveSenseState::ENABLED;
			 __attribute__ ((fallthrough));
			/* no break */
		case Types::START:
		case Types::CONTINUE:
		case Types::STOP:
		case Types::CLOCK:
		case Types::SYSTEM_RESET:
		case Types::TUNE_REQUEST:
			// Handle the message type directly here.
			m_Message.nTimestamp = nPendingTimestamp;
			m_Message.tType = GetTypeFromStatusByte(m_aPendingMessage[0]);
			m_Message.nChannel = 0;
			m_Message.nData1 = 0;
			m_Message.nData2 = 0;
			m_Message.nBytesCount = 1;
			//midi_message.valid = true;
			// \fix Running Status broken when receiving Clock messages.
			// Do not reset all input attributes, Running Status must remain unchanged.
			//resetInput();
			// We still need to reset these
			m_nPendingMessageIndex = 0;
			m_nPendingMessageExpectedLenght = 0;
			nUpdates++;
			return true;
			break;
		// 2 bytes messages
		case Types::PROGRAM_CHANGE:
		case Types::AFTER_TOUCH_CHANNEL:
		case Types::TIME_CODE_QUARTER_FRAME:
		case Types::SONG_SELECT:
			m_nPendingMessageExpectedLenght = 2;
			break;
		// 3 bytes messages
		case Types::NOTE_ON:
		case Types::NOTE_OFF:
		case Types::CONTROL_CHANGE:
		case Types::PITCH_BEND:
		case Types::AFTER_TOUCH_POLY:
		case Types::SONG_POSITION:
			m_nPendingMessageExpectedLenght = 3;
			break;
		case Types::SYSTEM_EXCLUSIVE:
			// The message can be any length
			// between 3 and MIDI_SYSTEM_EXCLUSIVE_INDEX_ENTRIES
			m_nPendingMessageExpectedLenght = MIDI_SYSTEM_EXCLUSIVE_INDEX_ENTRIES;
			m_nRunningStatusRx = static_cast<uint8_t>(Types::INVALIDE_TYPE);
			m_Message.aSystemExclusive[0] = static_cast<uint8_t>(Types::SYSTEM_EXCLUSIVE);
			m_Message.nChannel = 0;
			m_Message.nTimestamp = nPendingTimestamp;
			break;
		case Types::INVALIDE_TYPE:
		default:
			// This is obviously wrong. Let's get the hell out'a here.
			ResetInput();
			return false;
			break;
		}

		if (m_nPendingMessageIndex >= (m_nPendingMessageExpectedLenght - 1)) {
			// Reception complete
			m_Message.nTimestamp = nPendingTimestamp;
			m_Message.tType = GetTypeFromStatusByte(m_aPendingMessage[0]);
			m_Message.nChannel = GetChannelFromStatusByte(m_aPendingMessage[0]);
			m_Message.nData1 = m_aPendingMessage[1];

			/* Save data2 only if applicable */
			if (m_nPendingMessageExpectedLenght == 3) {
				m_Message.nData2 = m_aPendingMessage[2];
				m_Message.nBytesCount = 3;
			} else {
				m_Message.nData2 = 0;
				m_Message.nBytesCount = 2;
			}
			m_nPendingMessageIndex = 0;
			m_nPendingMessageExpectedLenght = 0;

			nUpdates++;
			return true;
		} else {
			// Waiting for more data
			m_nPendingMessageIndex++;
		}

		if (USE_1_BYTE_PARSING) {
			// Message is not complete.
			return false;
		} else {
			// Call the parser recursively
			// to parse the rest of the message.
			return Parse();
		}
	} else {
		// First, test if this is a status byte
		if (nSerialData >= 0x80) {
			// Reception of status bytes in the middle of an uncompleted message
			// are allowed only for interleaved Real Time message or EOX
			switch (nSerialData) {
			case static_cast<uint8_t>(Types::ACTIVE_SENSING):
				dmb();
				midi_active_sense_state = ActiveSenseState::ENABLED;
				 __attribute__ ((fallthrough));
				/* no break */
			case static_cast<uint8_t>(Types::CLOCK):
			case static_cast<uint8_t>(Types::START):
			case static_cast<uint8_t>(Types::CONTINUE):
			case static_cast<uint8_t>(Types::STOP):
			case static_cast<uint8_t>(Types::SYSTEM_RESET):
				// Here we will have to extract the one-byte message,
				// pass it to the structure for being read outside
				// the MIDI class, and recompose the message it was
				// interleaved into. Oh, and without killing the running status..
				// This is done by leaving the pending message as is,
				// it will be completed on next calls.
				m_Message.nTimestamp = nTimestamp;
				m_Message.tType = static_cast<Types>(nSerialData);
				m_Message.nData1 = 0;
				m_Message.nData2 = 0;
				m_Message.nChannel = 0;
				m_Message.nBytesCount = 1;
				//midi_message.valid = true;
				return true;

				break;
				// End of Exclusive
			case 0xF7:
				if (m_Message.aSystemExclusive[0] == static_cast<uint8_t>(Types::SYSTEM_EXCLUSIVE)) {
					// Store the last byte (EOX)
					m_Message.aSystemExclusive[m_nPendingMessageIndex++] = 0xF7;
					m_Message.tType = Types::SYSTEM_EXCLUSIVE;
					/* Get length */
					m_Message.nData1 = m_nPendingMessageIndex & 0xFF; // LSB
					m_Message.nData2 = m_nPendingMessageIndex >> 8;   // MSB
					m_Message.nChannel = 0;
					m_Message.nBytesCount = m_nPendingMessageIndex;

					nUpdates++;

					ResetInput();
					return true;
				} else {
					/* Error */
					ResetInput();
					return false;
				}
				break;
			default:
				break;
			}
		}

		// Add extracted data byte to pending message
		if (m_aPendingMessage[0] == static_cast<uint8_t>(Types::SYSTEM_EXCLUSIVE)) {
			m_Message.aSystemExclusive[m_nPendingMessageIndex] = nSerialData;
		} else {
			m_aPendingMessage[m_nPendingMessageIndex] = nSerialData;
		}

		// Now we are going to check if we have reached the end of the message
		if (m_nPendingMessageIndex >= (m_nPendingMessageExpectedLenght - 1)) {
			// "FML" case: fall down here with an overflown SysEx..
			// This means we received the last possible data byte that can fit
			// the buffer. If this happens, try increasing MidiMessage::sSysExMaxSize.
			if (m_aPendingMessage[0] == static_cast<uint8_t>(Types::SYSTEM_EXCLUSIVE)) {
				ResetInput();
				return false;
			}

			m_Message.nTimestamp = nTimestamp;
			m_Message.tType = GetTypeFromStatusByte(m_aPendingMessage[0]);

			if (isChannelMessage(m_Message.tType)) {
				m_Message.nChannel = GetChannelFromStatusByte(m_aPendingMessage[0]);
			} else {
				m_Message.nChannel = 0;
			}

			m_Message.nData1 = m_aPendingMessage[1];

			// Save data2 only if applicable
			if (m_nPendingMessageExpectedLenght == 3) {
				m_Message.nData2 = m_aPendingMessage[2];
				m_Message.nBytesCount = 3;
			} else {
				m_Message.nData2 = 0;
				m_Message.nBytesCount = 2;
			}

			// Reset local variables
			m_nPendingMessageIndex = 0;
			m_nPendingMessageExpectedLenght = 0;

			nUpdates++;

			// Activate running status (if enabled for the received type)
			switch (m_Message.tType) {
			case Types::NOTE_OFF:
			case Types::NOTE_ON:
			case Types::AFTER_TOUCH_POLY:
			case Types::CONTROL_CHANGE:
			case Types::PROGRAM_CHANGE:
			case Types::AFTER_TOUCH_CHANNEL:
			case Types::PITCH_BEND:
				// Running status enabled: store it from received message
				m_nRunningStatusRx = m_aPendingMessage[0];
				break;

			default:
				// No running status
				m_nRunningStatusRx = static_cast<uint8_t>(Types::INVALIDE_TYPE);
				break;
			}
			return true;
		} else {
			// Then update the index of the pending message.
			m_nPendingMessageIndex++;

			if (USE_1_BYTE_PARSING) {
				// Message is not complete.
				return false;
			} else {
				// Call the parser recursively to parse the rest of the message.
				return Parse();
			}
		}
	}
}

bool Midi::Read(uint8_t nChannel) {
	if (nChannel >= static_cast<uint8_t>(Channel::OFF)) {
		return false; // MIDI Input disabled.
	}

	if (!Parse()) {
		return false;
	}

	HandleNullVelocityNoteOnAsNoteOff();
	const auto isChannelMatch = InputFilter(nChannel);

	return isChannelMatch;
}

void Midi::SendTimeCode(const struct midi::Timecode *tc) {
	uint8_t data[10] = {0xF0, 0x7F, 0x7F, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0xF7};

	data[5] = (((tc->nType) & 0x03) << 5) | (tc->nHours & 0x1F);
	data[6] = tc->nMinutes & 0x3F;
	data[7] = tc->nSeconds & 0x3F;
	data[8] = tc->nFrames & 0x1F;

	SendUart2(data, 10);
}

void Midi::SendQf(uint8_t nValue) {
	uint8_t data[2];

	data[0] = 0xF1;
	data[1] = nValue;

	SendUart2(data, 2);
}

static void __attribute__((interrupt("IRQ"))) irq_midi_in_handler(void) {
	dmb();

	const uint32_t timestamp = h3_hs_timer_lo_us();
	const uint32_t irq = H3_GIC_CPUIF->AIA;

	if (H3_UART2->O08.IIR & UART_IIR_IID_RD) {

		midi_rx_buffer[midi_rx_buffer_index_head].nData = (H3_UART2->O00.RBR & 0xFF);
		midi_rx_buffer[midi_rx_buffer_index_head].nTimestamp = timestamp;
		midi_rx_buffer_index_head = (midi_rx_buffer_index_head + 1) & MIDI_RX_BUFFER_INDEX_MASK;

		H3_GIC_CPUIF->AEOI = H3_UART2_IRQn;
		gic_unpend(H3_UART2_IRQn);
	}

	if (irq == H3_TIMER0_IRQn) {
		H3_TIMER->IRQ_STA = TIMER_IRQ_PEND_TMR0;	/* Clear Timer 0 Pending bit */

		nUpdatesPerSecond = nUpdates - nUpdatesPrevious;
		nUpdatesPrevious = nUpdates;

		H3_GIC_CPUIF->AEOI = H3_TIMER0_IRQn;
		gic_unpend(H3_TIMER0_IRQn);
	} else if (irq == H3_TIMER1_IRQn) {
		H3_TIMER->IRQ_STA = TIMER_IRQ_PEND_TMR1;	/* Clear Timer 1 Pending bit */

		if (midi_active_sense_state == ActiveSenseState::ENABLED) {
			nActiveSenseTimeout++;
			if (nActiveSenseTimeout > 300) { /* > 300 ms */
				/* Turn All Notes Off */
				dmb();
				midi_active_sense_state = ActiveSenseState::FAILED;
			}
		}

		H3_GIC_CPUIF->AEOI = H3_TIMER1_IRQn;
		gic_unpend(H3_TIMER1_IRQn);
	}

	dmb();
}

void Midi::SendUart2(const uint8_t *pData, uint16_t nLength) {
	const uint8_t *p = pData;

	while (nLength > 0) {
		uint32_t nAvailable = 64 - H3_UART2->TFL;

		while ((nLength > 0) && (nAvailable > 0)) {
			H3_UART2->O00.THR = static_cast<uint32_t>(*p);
			nLength--;
			nAvailable--;
			p++;
		}
	}
}

void Midi::InitUart2(void) {
	uint32_t nValue = H3_PIO_PORTA->CFG0;
	// PA0, TX
	nValue &= static_cast<uint32_t>(~(GPIO_SELECT_MASK << PA0_SELECT_CFG0_SHIFT));
	nValue |= H3_PA0_SELECT_UART2_TX << PA0_SELECT_CFG0_SHIFT;
	// PA1, RX
	nValue &= static_cast<uint32_t>(~(GPIO_SELECT_MASK << PA1_SELECT_CFG0_SHIFT));
	nValue |= H3_PA1_SELECT_UART2_RX << PA1_SELECT_CFG0_SHIFT;
	H3_PIO_PORTA->CFG0 = nValue;

	H3_CCU->BUS_SOFT_RESET4 |= CCU_BUS_SOFT_RESET4_UART2;
	H3_CCU->BUS_CLK_GATING3 |= CCU_BUS_CLK_GATING3_UART2;

	H3_UART2->O08.FCR = 0;
	H3_UART2->LCR = UART_LCR_DLAB;

	if (m_nBaudrate == 0) {
		H3_UART2->O00.DLL = BAUD_31250_L;
	} else {
		H3_UART2->O00.DLL = (24000000 / 16) / m_nBaudrate;
	}

	H3_UART2->O04.DLH = 0;
	H3_UART2->O04.IER = 0;
	H3_UART2->LCR = UART_LCR_8_N_1;

	dmb();

	while ((H3_UART2->USR & UART_USR_BUSY) == UART_USR_BUSY) {
		static_cast<void>(H3_UART2->O00.RBR);
	}

	if ((static_cast<uint32_t>(m_tDirection) & static_cast<uint32_t>(Direction::INPUT)) == static_cast<uint32_t>(Direction::INPUT)) {
		H3_UART2->O08.FCR = 0;
		H3_UART2->O04.IER = UART_IER_ERBFI;

		gic_irq_config(H3_UART2_IRQn, GIC_CORE0);

		__enable_irq();
		dmb();
	}

	if ((static_cast<uint32_t>(m_tDirection) & static_cast<uint32_t>(Direction::OUTPUT)) == static_cast<uint32_t>(Direction::OUTPUT)) {
		H3_UART2->O08.FCR = UART_FCR_EFIFO | UART_FCR_TRESET;
		H3_UART2->O04.IER = 0;
	}
}

void Midi::Init(midi::Direction tDirection) {
	m_tDirection = tDirection;

	if ((static_cast<uint32_t>(tDirection) & static_cast<uint32_t>(Direction::INPUT)) == static_cast<uint32_t>(Direction::INPUT)) {
		for (uint32_t i = 0; i < MIDI_RX_BUFFER_INDEX_ENTRIES; i++) {
			midi_rx_buffer[i].nData = 0;
			midi_rx_buffer[i].nTimestamp = 0;
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
		H3_TIMER->IRQ_EN |= TIMER_IRQ_EN_TMR0;

		/*
		 * Active Sense
		 */
		if (m_bActiveSense) {
			gic_irq_config(H3_TIMER1_IRQn, GIC_CORE0);

			H3_TIMER->TMR1_CTRL = 0x14;					/* Select continuous mode, 24MHz clock source, 2 pre-scale */
			H3_TIMER->TMR1_INTV = 12000; 				/*  1 ms TODO 10ms */
			H3_TIMER->TMR1_CTRL &= ~(TIMER_CTRL_SINGLE_MODE);
			H3_TIMER->TMR1_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);
			H3_TIMER->IRQ_EN |= TIMER_IRQ_EN_TMR1;
		}

		arm_install_handler(reinterpret_cast<unsigned>(irq_midi_in_handler), ARM_VECTOR(ARM_VECTOR_IRQ));

		ResetInput();
	}

	if ((static_cast<uint32_t>(tDirection) & static_cast<uint32_t>(Direction::OUTPUT)) == static_cast<uint32_t>(Direction::OUTPUT)) {
	}

	InitUart2();
}
