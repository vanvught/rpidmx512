/**
 * @file midi.cpp
 *
 */
/* Copyright (C) 2016-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#pragma GCC push_options
#pragma GCC optimize ("O2")

#include <cstdint>
#include <cstring>
#include <cassert>

#include "midi.h"

#include "hal_uart.h"

#include "platform_midi.h"

#include "logic_analyzer.h"

namespace midi {
/**
 * Setting this to true will make midi_read parse only one byte of data for each
 * call when data is available. This can speed up your application if receiving
 * a lot of traffic, but might induce MIDI Thru and treatment latency.
 */
static constexpr auto USE_1_BYTE_PARSING = true;

static constexpr uint32_t RX_BUFFER_INDEX_ENTRIES	= (1U << 6);
static constexpr uint32_t RX_BUFFER_INDEX_MASK 		= (RX_BUFFER_INDEX_ENTRIES - 1);

struct MidiReceive {
	uint32_t nTimestamp;
	uint8_t nData;
} __attribute__ ((aligned (4)));

struct RxData {
	MidiReceive midi[midi::RX_BUFFER_INDEX_ENTRIES] __attribute__ ((aligned (4)));
	uint32_t nIndexHead;
	uint32_t nIndexTail;
};
}  // namespace midi

static volatile midi::RxData sv_RxBuffer __attribute__ ((aligned (4)));

static volatile uint32_t sv_nTick100ms;
static volatile uint32_t sv_nUpdatesPerSecond;
static volatile uint32_t sv_nUpdatesPrevious;
static volatile uint32_t sv_nUpdates;
static volatile uint32_t sv_nActiveSenseTimeout;
static volatile midi::ActiveSenseState sv_ActiveSenseState = midi::ActiveSenseState::NOT_ENABLED;

#if defined (H3)
#if __GNUC__ > 8
# pragma GCC target ("general-regs-only")
#endif

static volatile midi::thunk_irq_timer1_t irq_handler_timer1_func;

void Midi::SetIrqTimer1(midi::thunk_irq_timer1_t pFunc) {
	if (pFunc != nullptr) {
		irq_handler_timer1_func = pFunc;
		gic_irq_config(H3_TIMER1_IRQn, GIC_CORE0);

		H3_TIMER->TMR1_CTRL = 0x14;	// Select continuous mode, 24MHz clock source, 2 pre-scale
		H3_TIMER->IRQ_EN |= TIMER_IRQ_EN_TMR1;
	} else {
		H3_TIMER->IRQ_EN &= ~TIMER_IRQ_EN_TMR1;
	}
}

void TIMER0_IRQHandler() {	// 100ms Tick
	H3_TIMER->IRQ_STA = TIMER_IRQ_PEND_TMR0;	/* Clear Timer 0 Pending bit */

	if (sv_nTick100ms == 10) {
		sv_nTick100ms = 0;
		sv_nUpdatesPerSecond = sv_nUpdates - sv_nUpdatesPrevious;
		sv_nUpdatesPrevious = sv_nUpdates;
	} else {
		sv_nTick100ms = sv_nTick100ms + 1;
	}

	if (sv_ActiveSenseState == midi::ActiveSenseState::ENABLED) {
		sv_nActiveSenseTimeout = sv_nActiveSenseTimeout + 1;
		if (sv_nActiveSenseTimeout > 3) { // > 300 ms
			sv_ActiveSenseState = midi::ActiveSenseState::FAILED;	// Turn All Notes Off
		}
	}
}

void TIMER1_IRQHandler() {
	H3_TIMER->IRQ_STA = TIMER_IRQ_PEND_TMR1;	/* Clear Timer 1 Pending bit */

	assert(irq_handler_timer1_func != nullptr);

	if (__builtin_expect((irq_handler_timer1_func != nullptr), 1)) {
		irq_handler_timer1_func();
	}
}

void USART2_IRQHandler() {
	if (H3_UART2->O08.IIR & UART_IIR_IID_RD) {
		sv_RxBuffer.midi[sv_RxBuffer.nIndexHead].nData = (H3_UART2->O00.RBR & 0xFF);
		sv_RxBuffer.midi[sv_RxBuffer.nIndexHead].nTimestamp = H3_TIMER->AVS_CNT0;
		sv_RxBuffer.nIndexHead = (sv_RxBuffer.nIndexHead + 1) & midi::RX_BUFFER_INDEX_MASK;
	}
}
#elif defined (GD32)
#if defined(GD32H7XX)
# define TIMERx			TIMER14
# define RCU_TIMERx		RCU_TIMER14
# define TIMERx_IRQn	TIMER14_IRQn
#else
# if defined (GD32F10X) || defined (GD32F30X)
#  define TIMERx		TIMER3
#  define RCU_TIMERx	RCU_TIMER3
#  define TIMERx_IRQn	TIMER3_IRQn
# else
#  define TIMERx		TIMER8
#  define RCU_TIMERx	RCU_TIMER8
#  define TIMERx_IRQn	TIMER0_BRK_TIMER8_IRQn
# endif
#endif

extern "C" {
#if defined(GD32H7XX)
void TIMER14_IRQHandler() {
#elif defined (GD32F10X) || defined (GD32F30X)
void TIMER3_IRQHandler() {
#else
void TIMER0_BRK_TIMER8_IRQHandler() {
#endif
	const auto nIntFlag = TIMER_INTF(TIMERx);

	if ((nIntFlag & TIMER_INT_FLAG_UP) == TIMER_INT_FLAG_UP) {
		if (sv_nTick100ms == 10) {
			sv_nTick100ms = 0;
			sv_nUpdatesPerSecond = sv_nUpdates - sv_nUpdatesPrevious;
			sv_nUpdatesPrevious = sv_nUpdates;
		} else {
			sv_nTick100ms = sv_nTick100ms + 1;
		}

		if (sv_ActiveSenseState == midi::ActiveSenseState::ENABLED) {
			sv_nActiveSenseTimeout = sv_nActiveSenseTimeout + 1;
			if (sv_nActiveSenseTimeout > 3) { // > 300 ms
				sv_ActiveSenseState = midi::ActiveSenseState::FAILED;	// Turn All Notes Off
			}
		}
	}

	TIMER_INTF(TIMERx) = static_cast<uint32_t>(~nIntFlag);
}

void USART5_IRQHandler() {
	if (RESET != usart_interrupt_flag_get(USART5, USART_INT_FLAG_RBNE)) {
		sv_RxBuffer.midi[sv_RxBuffer.nIndexHead].nData = gd32_uart_get_rx_data(USART5);
//		sv_RxBuffer[sv_RxBuffer.nIndexHead].nTimestamp =
		sv_RxBuffer.nIndexHead = (sv_RxBuffer.nIndexHead + 1) & midi::RX_BUFFER_INDEX_MASK;
	}
}

#if defined(GD32H7XX)
static void timer14_config() {
#elif defined (GD32F10X) || defined (GD32F30X)
static void timer3_config() {
#else
static void timer8_config() {
#endif
	rcu_periph_clock_enable(RCU_TIMERx);
	timer_deinit(TIMERx);

	timer_parameter_struct timer_initpara;
	timer_struct_para_init(&timer_initpara);

	timer_initpara.prescaler = TIMER_PSC_10KHZ;
	timer_initpara.alignedmode = TIMER_COUNTER_EDGE;
	timer_initpara.counterdirection = TIMER_COUNTER_UP;
	timer_initpara.period = (1000 - 1);		// 10Hz / 100 ms
	timer_init(TIMERx, &timer_initpara);

	timer_interrupt_flag_clear(TIMERx, ~0);

	timer_interrupt_enable(TIMERx, TIMER_INT_UP);

	NVIC_SetPriority(TIMERx_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL); // Lowest priority
	NVIC_EnableIRQ(TIMERx_IRQn);

	timer_enable(TIMERx);
}
}
#endif

void Midi::Init(midi::Direction direction) {
	m_Direction = direction;

	FUNC_PREFIX (uart_begin(EXT_MIDI_UART_BASE, m_nBaudrate == 0 ? 31250 : m_nBaudrate, hal::uart::BITS_8, hal::uart::PARITY_NONE, hal::uart::STOP_1BIT));

	if ((static_cast<uint32_t>(direction) & static_cast<uint32_t>(midi::Direction::INPUT)) == static_cast<uint32_t>(midi::Direction::INPUT)) {
		ResetInput();

		auto nSize = sizeof(sv_RxBuffer);
		auto *pDst = reinterpret_cast<volatile uint8_t *>(&sv_RxBuffer);
		while (nSize--) {
			*pDst++ = 0;
		}

#if defined (H3)
		H3_TIMER->TMR0_CTRL = 0x14;					/* Select continuous mode, 24MHz clock source, 2 pre-scale */
		H3_TIMER->TMR0_INTV = 1200000; 				/* 100ms second */
		H3_TIMER->TMR0_CTRL &= ~(TIMER_CTRL_SINGLE_MODE);
		H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);
		H3_TIMER->IRQ_EN |= TIMER_IRQ_EN_TMR0;

		H3_UART2->O08.FCR = 0;
		H3_UART2->O04.IER = UART_IER_ERBFI;

		IRQ_SetHandler(H3_TIMER0_IRQn, TIMER0_IRQHandler);
		gic_irq_config(H3_TIMER0_IRQn, GIC_CORE0);

		IRQ_SetHandler(H3_TIMER1_IRQn, TIMER1_IRQHandler);
		gic_irq_config(H3_TIMER1_IRQn, GIC_CORE0);

		IRQ_SetHandler(H3_UART2_IRQn, USART2_IRQHandler);
		gic_irq_config(H3_UART2_IRQn, GIC_CORE0);

		__enable_irq();
		__DMB();
#elif defined (GD32)
# if defined(GD32H7XX)
		void timer14_config();
# elif defined (GD32F10X) || defined (GD32F30X)
		void timer3_config();
# else
		void timer8_config();
# endif
		usart_interrupt_flag_clear(USART5, USART_INT_FLAG_RBNE);
		usart_interrupt_enable(USART5, USART_INT_RBNE);

		NVIC_EnableIRQ(USART5_IRQn);
#endif
	}

	if ((static_cast<uint32_t>(m_Direction) & static_cast<uint32_t>(midi::Direction::OUTPUT)) == static_cast<uint32_t>(midi::Direction::OUTPUT)) {
#if defined (H3)
		H3_UART2->O08.FCR = UART_FCR_EFIFO | UART_FCR_TRESET;
		H3_UART2->O04.IER = 0;
		IRQ_SetHandler(H3_UART2_IRQn, nullptr);
#elif defined (GD32)
		NVIC_DisableIRQ(USART5_IRQn);
#endif
	}
}

/**
 * Common
 */

uint32_t Midi::GetUpdatesPerSecond() {
	__DMB();
	return sv_nUpdatesPerSecond;
}

bool Midi::ReadRaw(uint8_t *pByte, uint32_t *pTimestamp) {
	__DMB();
	if (sv_RxBuffer.nIndexHead != sv_RxBuffer.nIndexTail) {
		*pByte = sv_RxBuffer.midi[sv_RxBuffer.nIndexTail].nData;
		*pTimestamp = sv_RxBuffer.midi[sv_RxBuffer.nIndexTail].nTimestamp;
		sv_RxBuffer.nIndexTail = (sv_RxBuffer.nIndexTail + 1) & midi::RX_BUFFER_INDEX_MASK;
		return true;
	} else {
		return false;
	}
}

midi::ActiveSenseState Midi::GetActiveSenseState() {
	__DMB();
	return sv_ActiveSenseState;
}

bool Midi::Parse() {
	uint8_t nSerialData;
	uint32_t nTimestamp;
	uint32_t nPendingTimestamp;

    if (!ReadRaw(&nSerialData, &nTimestamp)) {
        return false;
	}

    sv_nActiveSenseTimeout = 0;

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
			// We received another status byte,
			// so the running status does not apply here.
			// It will be updated upon completion of this message.
		}

		switch (GetTypeFromStatusByte(m_aPendingMessage[0])) {
		// 1 byte messages
		case midi::Types::ACTIVE_SENSING:
			sv_ActiveSenseState = midi::ActiveSenseState::ENABLED;
			 __attribute__ ((fallthrough));
			/* no break */
		case midi::Types::START:
		case midi::Types::CONTINUE:
		case midi::Types::STOP:
		case midi::Types::CLOCK:
		case midi::Types::SYSTEM_RESET:
		case midi::Types::TUNE_REQUEST:
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
			sv_nUpdates = sv_nUpdates + 1;
			return true;
			__builtin_unreachable();
			break;
		// 2 bytes messages
		case midi::Types::PROGRAM_CHANGE:
		case midi::Types::AFTER_TOUCH_CHANNEL:
		case midi::Types::TIME_CODE_QUARTER_FRAME:
		case midi::Types::SONG_SELECT:
			m_nPendingMessageExpectedLenght = 2;
			break;
		// 3 bytes messages
		case midi::Types::NOTE_ON:
		case midi::Types::NOTE_OFF:
		case midi::Types::CONTROL_CHANGE:
		case midi::Types::PITCH_BEND:
		case midi::Types::AFTER_TOUCH_POLY:
		case midi::Types::SONG_POSITION:
			m_nPendingMessageExpectedLenght = 3;
			break;
		case midi::Types::SYSTEM_EXCLUSIVE:
			// The message can be any length
			// between 3 and MIDI_SYSTEM_EXCLUSIVE_INDEX_ENTRIES
			m_nPendingMessageExpectedLenght = MIDI_SYSTEM_EXCLUSIVE_INDEX_ENTRIES;
			m_nRunningStatusRx = static_cast<uint8_t>(midi::Types::INVALIDE_TYPE);
			m_Message.aSystemExclusive[0] = static_cast<uint8_t>(midi::Types::SYSTEM_EXCLUSIVE);
			m_Message.nChannel = 0;
			m_Message.nTimestamp = nPendingTimestamp;
			break;
		case midi::Types::INVALIDE_TYPE:
		default:
			// This is obviously wrong. Let's get the hell out'a here.
			ResetInput();
			return false;
			__builtin_unreachable();
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

			sv_nUpdates = sv_nUpdates + 1;
			return true;
		} else {
			// Waiting for more data
			m_nPendingMessageIndex++;
		}

		if (midi::USE_1_BYTE_PARSING) {
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
			case static_cast<uint8_t>(midi::Types::ACTIVE_SENSING):
				sv_ActiveSenseState = midi::ActiveSenseState::ENABLED;
				 __attribute__ ((fallthrough));
				/* no break */
			case static_cast<uint8_t>(midi::Types::CLOCK):
			case static_cast<uint8_t>(midi::Types::START):
			case static_cast<uint8_t>(midi::Types::CONTINUE):
			case static_cast<uint8_t>(midi::Types::STOP):
			case static_cast<uint8_t>(midi::Types::SYSTEM_RESET):
				// Here we will have to extract the one-byte message,
				// pass it to the structure for being read outside
				// the MIDI class, and recompose the message it was
				// interleaved into. Oh, and without killing the running status..
				// This is done by leaving the pending message as is,
				// it will be completed on next calls.
				m_Message.nTimestamp = nTimestamp;
				m_Message.tType = static_cast<midi::Types>(nSerialData);
				m_Message.nData1 = 0;
				m_Message.nData2 = 0;
				m_Message.nChannel = 0;
				m_Message.nBytesCount = 1;
				//midi_message.valid = true;
				return true;
				__builtin_unreachable();
				break;
				// End of Exclusive
			case 0xF7:
				if (m_Message.aSystemExclusive[0] == static_cast<uint8_t>(midi::Types::SYSTEM_EXCLUSIVE)) {
					// Store the last byte (EOX)
					m_Message.aSystemExclusive[m_nPendingMessageIndex++] = 0xF7;
					m_Message.tType = midi::Types::SYSTEM_EXCLUSIVE;
					/* Get length */
					m_Message.nData1 = m_nPendingMessageIndex & 0xFF; // LSB
					m_Message.nData2 = static_cast<uint8_t>(m_nPendingMessageIndex >> 8);   // MSB
					m_Message.nChannel = 0;
					m_Message.nBytesCount = m_nPendingMessageIndex;

					sv_nUpdates = sv_nUpdates + 1;

					ResetInput();
					return true;
				} else {
					/* Error */
					ResetInput();
					return false;
				}
				__builtin_unreachable();
				break;
			default:
				break;
			}
		}

		// Add extracted data byte to pending message
		if (m_aPendingMessage[0] == static_cast<uint8_t>(midi::Types::SYSTEM_EXCLUSIVE)) {
			m_Message.aSystemExclusive[m_nPendingMessageIndex] = nSerialData;
		} else {
			m_aPendingMessage[m_nPendingMessageIndex] = nSerialData;
		}

		// Now we are going to check if we have reached the end of the message
		if (m_nPendingMessageIndex >= (m_nPendingMessageExpectedLenght - 1)) {
			// "FML" case: fall down here with an overflown SysEx..
			// This means we received the last possible data byte that can fit
			// the buffer. If this happens, try increasing MidiMessage::sSysExMaxSize.
			if (m_aPendingMessage[0] == static_cast<uint8_t>(midi::Types::SYSTEM_EXCLUSIVE)) {
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

			sv_nUpdates = sv_nUpdates + 1;

			// Activate running status (if enabled for the received type)
			switch (m_Message.tType) {
			case midi::Types::NOTE_OFF:
			case midi::Types::NOTE_ON:
			case midi::Types::AFTER_TOUCH_POLY:
			case midi::Types::CONTROL_CHANGE:
			case midi::Types::PROGRAM_CHANGE:
			case midi::Types::AFTER_TOUCH_CHANNEL:
			case midi::Types::PITCH_BEND:
				// Running status enabled: store it from received message
				m_nRunningStatusRx = m_aPendingMessage[0];
				break;

			default:
				// No running status
				m_nRunningStatusRx = static_cast<uint8_t>(midi::Types::INVALIDE_TYPE);
				break;
			}
			return true;
		} else {
			// Then update the index of the pending message.
			m_nPendingMessageIndex++;

			if (midi::USE_1_BYTE_PARSING) {
				// Message is not complete.
				return false;
			} else {
				// Call the parser recursively to parse the rest of the message.
				return Parse();
			}
		}
	}
}
