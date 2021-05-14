/**
 * @file dmx.cpp
 *
 */
/* Copyright (C) 2018-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cstring>
#include <algorithm>
#include <cassert>

#include "dmx.h"
#include "rdm.h"
#include "rdm_e120.h"

#include "arm/arm.h"
#include "arm/synchronize.h"
#include "arm/gic.h"
#include "uart.h"
#include "irq_timer.h"

#include "h3_gpio.h"
#include "h3_ccu.h"
#include "h3_timer.h"
#include "h3_hs_timer.h"
#include "h3_board.h"

#include "debug.h"

extern "C" {
void console_error(const char*);
}

#if (GPIO_DMX_DATA_DIRECTION != GPIO_EXT_12)
# error GPIO_DMX_DATA_DIRECTION
#endif

#if (EXT_UART_NUMBER == 1)
# define UART_IRQN 		H3_UART1_IRQn
#elif (EXT_UART_NUMBER == 3)
# define UART_IRQN 		H3_UART3_IRQn
#else
# error Unsupported UART device configured
#endif

#ifndef ALIGNED
# define ALIGNED __attribute__ ((aligned (4)))
#endif

typedef enum {
	IDLE = 0,
	PRE_BREAK,
	BREAK,
	MAB,
	DMXDATA,
	RDMDATA,
	CHECKSUMH,
	CHECKSUML,
	RDMDISCFE,
	RDMDISCEUID,
	RDMDISCECS,
	DMXINTER
} _dmx_state;

using namespace dmxsingle;
using namespace dmx;

static PortDirection dmx_port_direction = dmx::PortDirection::INP;

// DMX

static volatile uint32_t dmx_data_buffer_index_head ;
static volatile uint32_t dmx_data_buffer_index_tail;
static struct Data dmx_data[buffer::INDEX_ENTRIES] ALIGNED;
static uint8_t dmx_data_previous[buffer::SIZE] ALIGNED;
static volatile _dmx_state dmx_receive_state = IDLE;
static volatile uint32_t dmx_data_index;

static uint32_t dmx_output_break_time_intv = (transmit::BREAK_TIME_MIN * 12);
static uint32_t dmx_output_mab_time_intv = (transmit::MAB_TIME_MIN * 12);
static uint32_t dmx_output_period_intv = (transmit::PERIOD_DEFAULT * 12) - (transmit::MAB_TIME_MIN * 12) - (transmit::BREAK_TIME_MIN * 12);

static uint32_t dmx_send_data_length = (max::CHANNELS + 1);		///< SC + UNIVERSE SIZE
static volatile uint32_t dmx_fiq_micros_current;
static volatile uint32_t dmx_fiq_micros_previous;
static volatile bool dmx_is_previous_break_dmx = false;
static volatile uint32_t dmx_break_to_break_latest;
static volatile uint32_t dmx_break_to_break_previous;
static volatile uint32_t dmx_slots_in_packet_previous;
static volatile _dmx_state dmx_send_state = IDLE;
static volatile bool dmx_send_always = false;
static volatile uint32_t dmx_send_break_micros;
static volatile uint32_t dmx_send_current_slot;
static bool is_stopped = true;

static volatile uint32_t dmx_updates_per_seconde;
static uint32_t dmx_packets_previous;
static volatile struct TotalStatistics total_statistics ALIGNED;

// RDM

static volatile uint32_t rdm_data_buffer_index_head;
static volatile uint32_t rdm_data_buffer_index_tail;
static uint8_t rdm_data_buffer[RDM_DATA_BUFFER_INDEX_ENTRIES][RDM_DATA_BUFFER_SIZE] ALIGNED;
static volatile uint16_t rdm_checksum;							///< This must be uint16_t
static volatile uint32_t rdm_data_receive_end;
static volatile uint32_t rdm_disc_index;

/**
 * Timer 0 interrupt DMX Receiver
 * Slot time-out
 */
static void irq_timer0_dmx_receive(uint32_t clo) {
	dmb();
	if (dmx_receive_state == DMXDATA) {
		if (clo - dmx_fiq_micros_current > dmx_data[0].Statistics.nSlotToSlot) {
			dmb();
			dmx_receive_state = IDLE;
			dmx_data[dmx_data_buffer_index_head].Statistics.nSlotsInPacket = dmx_data_index - 1;
			dmx_data_buffer_index_head = (dmx_data_buffer_index_head + 1) & buffer::INDEX_MASK;
		} else {
			H3_TIMER->TMR0_INTV = dmx_data[dmx_data_buffer_index_head].Statistics.nSlotToSlot * 12;
			H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD); // 0x3;
		}
	}
}

/**
 * Timer 0 interrupt DMX Sender
 */
static void irq_timer0_dmx_sender(uint32_t clo) {
	switch (dmx_send_state) {
	case IDLE:
	case DMXINTER:
		H3_TIMER->TMR0_INTV = dmx_output_break_time_intv;
		H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD); // 0x3;

		EXT_UART->LCR = UART_LCR_8_N_2 | UART_LCR_BC;
		dmx_send_break_micros = clo;
		dmb();
		dmx_send_state = BREAK;
		break;
	case BREAK:
		H3_TIMER->TMR0_INTV = dmx_output_mab_time_intv;
		H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD); // 0x3;

		EXT_UART->LCR = UART_LCR_8_N_2;
		dmb();
		dmx_send_state = MAB;
		break;
	case MAB: {
		H3_TIMER->TMR0_INTV = dmx_output_period_intv;
		H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD); // 0x3;

		uint32_t fifo_cnt = 16;

		for (dmx_send_current_slot = 0; fifo_cnt-- > 0; dmx_send_current_slot++) {
			if (dmx_send_current_slot >= dmx_send_data_length) {
				break;
			}

			EXT_UART->O00.THR = dmx_data[0].Data[dmx_send_current_slot];
		}

		if (dmx_send_current_slot < dmx_send_data_length) {
			dmb();
			dmx_send_state = DMXDATA;
			EXT_UART->O04.IER = UART_IER_ETBEI;
		} else {
			dmb();
			dmx_send_state = DMXINTER;
		}
	}
		break;
	case DMXDATA:
		printf("Output period too short (dlen %d, slot %d)\n", dmx_send_data_length, dmx_send_current_slot);
		assert(0);
		break;
	default:
		assert(0);
		break;
	}
}

/**
 * Timer 1 interrupt DMX Receiver
 * Statistics
 */
static void irq_timer1_dmx_receive(__attribute__((unused)) uint32_t clo) {
	dmb();
	dmx_updates_per_seconde = total_statistics.nDmxPackets - dmx_packets_previous;
	dmx_packets_previous = total_statistics.nDmxPackets;
}

/**
 * Interrupt handler for continues receiving DMX512 data.
 */
static void fiq_dmx_in_handler(void) {
	dmx_fiq_micros_current = h3_hs_timer_lo_us();

	if (EXT_UART->LSR & UART_LSR_BI) {
		dmx_receive_state = PRE_BREAK;
		dmx_break_to_break_latest = dmx_fiq_micros_current;
	} else if (EXT_UART->O08.IIR & UART_IIR_IID_RD) {
		const auto data = static_cast<uint8_t>(EXT_UART->O00.RBR);

		switch (dmx_receive_state) {
		case IDLE:
			if (data == 0xFE) {
				dmx_receive_state = RDMDISCFE;
				rdm_data_buffer[rdm_data_buffer_index_head][0] = 0xFE;
				dmx_data_index = 1;
			}
			break;
		case PRE_BREAK:
			dmx_receive_state = BREAK;
			break;
		case BREAK:
			switch (data) {
			case START_CODE:
				dmx_receive_state = DMXDATA;
				dmx_data[dmx_data_buffer_index_head].Data[0] = START_CODE;
				dmx_data_index = 1;
				total_statistics.nDmxPackets = total_statistics.nDmxPackets + 1;

				if (dmx_is_previous_break_dmx) {
					dmx_data[dmx_data_buffer_index_head].Statistics.nBreakToBreak = dmx_break_to_break_latest - dmx_break_to_break_previous;
					dmx_break_to_break_previous = dmx_break_to_break_latest;

				} else {
					dmx_is_previous_break_dmx = true;
					dmx_break_to_break_previous = dmx_break_to_break_latest;
				}
				break;
			case E120_SC_RDM:
				dmx_receive_state = RDMDATA;
				rdm_data_buffer[rdm_data_buffer_index_head][0] = E120_SC_RDM;
				rdm_checksum = E120_SC_RDM;
				dmx_data_index = 1;
				total_statistics.nRdmPackets = total_statistics.nRdmPackets + 1;
				dmx_is_previous_break_dmx = false;
				break;
			default:
				dmx_receive_state = IDLE;
				dmx_is_previous_break_dmx = false;
				break;
			}
			break;
		case DMXDATA:
			dmx_data[dmx_data_buffer_index_head].Statistics.nSlotToSlot = dmx_fiq_micros_current - dmx_fiq_micros_previous;
			dmx_data[dmx_data_buffer_index_head].Data[dmx_data_index++] = data;

			H3_TIMER->TMR0_INTV = (dmx_data[0].Statistics.nSlotToSlot + 12) * 12;
			H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD); // 0x3;

			if (dmx_data_index > max::CHANNELS) {
				dmx_receive_state = IDLE;
				dmx_data[dmx_data_buffer_index_head].Statistics.nSlotsInPacket = max::CHANNELS;
				dmx_data_buffer_index_head = (dmx_data_buffer_index_head + 1) & buffer::INDEX_MASK;
				dmb();
			}
			break;
		case RDMDATA:
			if (dmx_data_index > RDM_DATA_BUFFER_SIZE) {
				dmx_receive_state = IDLE;
			} else {
				rdm_data_buffer[rdm_data_buffer_index_head][dmx_data_index++] = data;
				rdm_checksum += data;

				const auto *p = reinterpret_cast<struct _rdm_command *>(&rdm_data_buffer[rdm_data_buffer_index_head][0]);
				if (dmx_data_index == p->message_length) {
					dmx_receive_state = CHECKSUMH;
				}
			}
			break;
		case CHECKSUMH:
			rdm_data_buffer[rdm_data_buffer_index_head][dmx_data_index++] =	data;
			rdm_checksum -= static_cast<uint16_t>(data << 8);
			dmx_receive_state = CHECKSUML;
			break;
		case CHECKSUML: {
			rdm_data_buffer[rdm_data_buffer_index_head][dmx_data_index++] = data;
			rdm_checksum -= data;
			const auto *p = reinterpret_cast<struct _rdm_command *>(&rdm_data_buffer[rdm_data_buffer_index_head][0]);

			if ((rdm_checksum == 0) && (p->sub_start_code == E120_SC_SUB_MESSAGE)) {
				rdm_data_buffer_index_head = (rdm_data_buffer_index_head + 1) & RDM_DATA_BUFFER_INDEX_MASK;
				rdm_data_receive_end = h3_hs_timer_lo_us();;
				dmb();
			}
			dmx_receive_state = IDLE;
		}
			break;
		case RDMDISCFE:
			rdm_data_buffer[rdm_data_buffer_index_head][dmx_data_index++] = data;

			if ((data == 0xAA) || (dmx_data_index == 9 )) {
				dmx_receive_state = RDMDISCEUID;
				rdm_disc_index = 0;
			}
			break;
		case RDMDISCEUID:
			rdm_data_buffer[rdm_data_buffer_index_head][dmx_data_index++] = data;
			rdm_disc_index++;

			if (rdm_disc_index == 2 * RDM_UID_SIZE) {
				dmx_receive_state = RDMDISCECS;
				rdm_disc_index = 0;
			}
			break;
		case RDMDISCECS:
			rdm_data_buffer[rdm_data_buffer_index_head][dmx_data_index++] = data;
			rdm_disc_index++;

			if (rdm_disc_index == 4) {
				rdm_data_buffer_index_head = (rdm_data_buffer_index_head + 1) & RDM_DATA_BUFFER_INDEX_MASK;
				dmx_receive_state = IDLE;
				rdm_data_receive_end = h3_hs_timer_lo_us();;
				dmb();
			}

			break;
		default:
			dmx_receive_state = IDLE;
			dmx_is_previous_break_dmx = false;
			break;
		}
	} else {
	}

	dmx_fiq_micros_previous = dmx_fiq_micros_current;
}

/**
 * EXT_UART TX interrupt
 */
static void fiq_dmx_out_handler(void) {
	uint32_t fifo_cnt = 16;

	for (; fifo_cnt-- > 0; dmx_send_current_slot++) {
		if (dmx_send_current_slot >= dmx_send_data_length) {
			break;
		}

		EXT_UART->O00.THR = dmx_data[0].Data[dmx_send_current_slot];
	}

	if (dmx_send_current_slot >= dmx_send_data_length) {
		EXT_UART->O04.IER &= static_cast<uint32_t>(~UART_IER_ETBEI);
		dmb();
		dmx_send_state = DMXINTER;
	}
}

static void __attribute__((interrupt("FIQ"))) fiq_dmx(void) {
	dmb();

	if (gic_get_active_fiq() == UART_IRQN) {
		const uint32_t iir = EXT_UART->O08.IIR ;

		if (dmx_port_direction == PortDirection::INP) {
			fiq_dmx_in_handler();
		} else {
			if ((iir & 0xF) == UART_IIR_IID_THRE) {
				fiq_dmx_out_handler();
			} else {
				(void) EXT_UART->USR;
			}
		}

		H3_GIC_CPUIF->EOI = UART_IRQN;
		gic_unpend(UART_IRQN);
	} else {
		console_error("spurious interrupt\n");
	}

	dmb();
}

Dmx *Dmx::s_pThis = nullptr;

Dmx::Dmx(uint8_t nGpioPin, bool DoInit) {
	DEBUG_PRINTF("m_IsInitDone=%d", DoInit);

	assert(s_pThis == nullptr);
	s_pThis = this;

	m_nDataDirectionGpio = nGpioPin;

	if (DoInit) {
		Init();
	}
}

void  Dmx::UartInit() {
#if (EXT_UART_NUMBER == 1)
	uint32_t value = H3_PIO_PORTG->CFG0;
	// PG6, TX
	value &= static_cast<uint32_t> (~(GPIO_SELECT_MASK << PG6_SELECT_CFG0_SHIFT));
	value |= H3_PG6_SELECT_UART1_TX << PG6_SELECT_CFG0_SHIFT;
	// PG7, RX
	value &= static_cast<uint32_t> (~(GPIO_SELECT_MASK << PG7_SELECT_CFG0_SHIFT));
	value |= H3_PG7_SELECT_UART1_RX << PG7_SELECT_CFG0_SHIFT;
	H3_PIO_PORTG->CFG0 = value;

	H3_CCU->BUS_CLK_GATING3 |= CCU_BUS_CLK_GATING3_UART1;
	H3_CCU->BUS_SOFT_RESET4 |= CCU_BUS_SOFT_RESET4_UART1;
#elif (EXT_UART_NUMBER == 3)
	uint32_t value = H3_PIO_PORTA->CFG1;
	// PA13, TX
	value &= static_cast<uint32_t>(~(GPIO_SELECT_MASK << PA13_SELECT_CFG1_SHIFT));
	value |= H3_PA13_SELECT_UART3_TX << PA13_SELECT_CFG1_SHIFT;
	// PA14, RX
	value &= static_cast<uint32_t> (~(GPIO_SELECT_MASK << PA14_SELECT_CFG1_SHIFT));
	value |= H3_PA14_SELECT_UART3_RX << PA14_SELECT_CFG1_SHIFT;
	H3_PIO_PORTA->CFG1 = value;

	H3_CCU->BUS_CLK_GATING3 |= CCU_BUS_CLK_GATING3_UART3;
	H3_CCU->BUS_SOFT_RESET4 |= CCU_BUS_SOFT_RESET4_UART3;
#else
# error Unsupported UART device configured
#endif

	EXT_UART->O08.FCR = 0;

	EXT_UART->LCR = UART_LCR_DLAB;
	EXT_UART->O00.DLL = BAUD_250000_L;
	EXT_UART->O04.DLH = BAUD_250000_H;
	EXT_UART->LCR = UART_LCR_8_N_2;

	isb();
}

void Dmx::Init() {
	assert(!m_IsInitDone);

	if (m_IsInitDone) {
		return;
	}

	m_IsInitDone = true;

	h3_gpio_fsel(m_nDataDirectionGpio, GPIO_FSEL_OUTPUT);
	h3_gpio_clr(m_nDataDirectionGpio);	// 0 = input, 1 = output

#ifdef LOGIC_ANALYZER
	h3_gpio_fsel(GPIO_ANALYZER_CH1, GPIO_FSEL_OUTPUT);	///<
	h3_gpio_clr(GPIO_ANALYZER_CH1);
	h3_gpio_fsel(GPIO_ANALYZER_CH2, GPIO_FSEL_OUTPUT);	///<
	h3_gpio_clr(GPIO_ANALYZER_CH2);
	h3_gpio_fsel(GPIO_ANALYZER_CH3, GPIO_FSEL_OUTPUT);	///<
	h3_gpio_clr(GPIO_ANALYZER_CH3);
	h3_gpio_fsel(GPIO_ANALYZER_CH4, GPIO_FSEL_OUTPUT);	///<
	h3_gpio_clr(GPIO_ANALYZER_CH4);
	h3_gpio_fsel(GPIO_ANALYZER_CH5, GPIO_FSEL_OUTPUT);	///<
	h3_gpio_clr(GPIO_ANALYZER_CH5);
	h3_gpio_fsel(GPIO_ANALYZER_CH6, GPIO_FSEL_OUTPUT);	///<
	h3_gpio_clr(GPIO_ANALYZER_CH6);
	h3_gpio_fsel(GPIO_ANALYZER_CH7, GPIO_FSEL_OUTPUT);	///<
	h3_gpio_clr(GPIO_ANALYZER_CH7);
#endif

	ClearData();

	dmx_data_buffer_index_head = 0;
	dmx_data_buffer_index_tail = 0;

	rdm_data_buffer_index_head = 0;
	rdm_data_buffer_index_tail = 0;

	dmx_receive_state = IDLE;

	dmx_send_state = IDLE;
	dmx_send_always = false;

	irq_timer_init();

	irq_timer_set(IRQ_TIMER_1, irq_timer1_dmx_receive);
	H3_TIMER->TMR1_INTV = 0xB71B00; // 1 second
	H3_TIMER->TMR1_CTRL &= ~(TIMER_CTRL_SINGLE_MODE);
	H3_TIMER->TMR1_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD); // 0x3;

	gic_fiq_config(UART_IRQN, GIC_CORE0);

	UartInit();

	__disable_fiq();
	arm_install_handler(reinterpret_cast<unsigned>(fiq_dmx), ARM_VECTOR(ARM_VECTOR_FIQ));
}

void Dmx::UartEnableFifo() {	// DMX Output
	EXT_UART->O08.FCR = UART_FCR_EFIFO | UART_FCR_TRESET;
	EXT_UART->O04.IER = 0;
	isb();
}

void Dmx::UartDisableFifo() {	// DMX Input
	EXT_UART->O08.FCR = 0;
	EXT_UART->O04.IER = UART_IER_ERBFI;
	isb();
}

void Dmx::StartData() {
	switch (dmx_port_direction) {
	case PortDirection::OUTP: {
		dmx_send_always = true;
		dmx_send_state = IDLE;

		UartEnableFifo();
		__enable_fiq();

		irq_timer_set(IRQ_TIMER_0, irq_timer0_dmx_sender);

		const auto clo = h3_hs_timer_lo_us();

		if (clo - dmx_send_break_micros > m_nDmxTransmitPeriod) {
			H3_TIMER->TMR0_CTRL |= TIMER_CTRL_SINGLE_MODE;
			H3_TIMER->TMR0_INTV = 4 * 12;
			H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD); // 0x3;
		} else {
			H3_TIMER->TMR0_CTRL |= TIMER_CTRL_SINGLE_MODE;
			H3_TIMER->TMR0_INTV = (m_nDmxTransmitPeriod + 4) * 12;
			H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD); // 0x3;
		}

		isb();
	}
		break;
	case PortDirection::INP:
		dmx_receive_state = IDLE;

		irq_timer_set(IRQ_TIMER_0, irq_timer0_dmx_receive);
		H3_TIMER->TMR0_CTRL |= TIMER_CTRL_SINGLE_MODE;

		while ((EXT_UART->USR & UART_USR_BUSY) == UART_USR_BUSY) {
			(void) EXT_UART->O00.RBR;
		}

		UartDisableFifo();
		__enable_fiq();

		isb();

		break;
	default:
		break;
	}

	is_stopped = false;
}

void Dmx::StopData() {
	uint32_t i;

	if (is_stopped) {
		return;
	}

	if (dmx_send_always) {
		do {
			dmb();
			if (dmx_send_state == DMXINTER) {
				while (!(EXT_UART->USR & UART_USR_TFE))
					;
				dmx_send_state = IDLE;
			}
			dmb();
		} while (dmx_send_state != IDLE);
	}

	irq_timer_set(IRQ_TIMER_0, NULL);

	__disable_fiq();
	isb();

	dmx_send_always = false;
	dmx_receive_state = IDLE;

	for (i = 0; i < buffer::INDEX_ENTRIES; i++) {
		dmx_data[i].Statistics.nSlotsInPacket = 0;
	}

	is_stopped = true;
}

void Dmx::SetPortDirection(__attribute__((unused)) uint32_t nPort, PortDirection tPortDirection, bool bEnableData) {
	DEBUG_PRINTF("tPortDirection=%d, bEnableData=%d", static_cast<int>(tPortDirection), bEnableData);

	assert(nPort == 0);

		if (tPortDirection != dmx_port_direction) {
			StopData();

			switch (tPortDirection) {
			case  PortDirection::OUTP:
				h3_gpio_set(m_nDataDirectionGpio);// 0 = input, 1 = output
				dmx_port_direction = PortDirection::OUTP;
				break;
			case  PortDirection::INP:
			default:
				h3_gpio_clr(m_nDataDirectionGpio);// 0 = input, 1 = output
				dmx_port_direction =  PortDirection::INP;
				break;
			}
		} else if (!bEnableData) {
			StopData();
		}

		if (bEnableData) {
			StartData();
		}
}

dmx::PortDirection Dmx::GetPortDirection() {
	return dmx_port_direction;
}

// DMX

void Dmx::SetDmxBreakTime(uint32_t nBreakTime) {
	m_nDmxTransmitBreakTime = std::max(transmit::BREAK_TIME_MIN, nBreakTime);
	dmx_output_break_time_intv = m_nDmxTransmitBreakTime * 12;

	SetDmxPeriodTime(m_nDmxTransmitPeriodRequested);
}

void Dmx::SetDmxMabTime(uint32_t nMabTime) {
	m_nDmxTransmitMabTime = std::max(transmit::MAB_TIME_MIN, nMabTime);
	dmx_output_mab_time_intv = m_nDmxTransmitMabTime * 12;

	SetDmxPeriodTime(m_nDmxTransmitPeriodRequested);
}

void Dmx::SetDmxPeriodTime(uint32_t nPeriodTime) {
	const auto package_length_us = m_nDmxTransmitBreakTime + m_nDmxTransmitMabTime + (dmx_send_data_length * 44);

	m_nDmxTransmitPeriodRequested = nPeriodTime;

	if (nPeriodTime != 0) {
		if (nPeriodTime < package_length_us) {
			m_nDmxTransmitPeriod =  std::max(transmit::BREAK_TO_BREAK_TIME_MIN, package_length_us + 44);
		} else {
			m_nDmxTransmitPeriod = nPeriodTime;
		}
	} else {
		m_nDmxTransmitPeriod =  std::max(transmit::BREAK_TO_BREAK_TIME_MIN, package_length_us + 44);
	}

	dmx_output_period_intv = (m_nDmxTransmitPeriod * 12) - dmx_output_break_time_intv - dmx_output_mab_time_intv;
}

const uint8_t* Dmx::GetDmxCurrentData() {
	return dmx_data[dmx_data_buffer_index_tail].Data;
}

const uint8_t* Dmx::GetDmxAvailable() {
	dmb();
	if (dmx_data_buffer_index_head == dmx_data_buffer_index_tail) {
		return nullptr;
	} else {
		const auto *p = dmx_data[dmx_data_buffer_index_tail].Data;
		dmx_data_buffer_index_tail = (dmx_data_buffer_index_tail + 1) & buffer::INDEX_MASK;
		return p;
	}
}

const uint8_t* Dmx::GetDmxChanged() {
	const auto *p = GetDmxAvailable();
	auto *src = reinterpret_cast<const uint32_t *>(p);

	if (src == nullptr) {
		return nullptr;
	}

	auto *dst = reinterpret_cast<uint32_t *>(dmx_data_previous);
	const auto *dmx_statistics = reinterpret_cast<const struct Data *>(p);

	if (dmx_statistics->Statistics.nSlotsInPacket != dmx_slots_in_packet_previous) {
		dmx_slots_in_packet_previous = dmx_statistics->Statistics.nSlotsInPacket;
		for (uint32_t i = 0; i < buffer::SIZE / 4; i++) {
			*dst= *src;
			dst++;
			src++;
		}
		return p;
	}

	auto is_changed = false;

	for (uint32_t i = 0; i < buffer::SIZE / 4; i++) {
		if (*dst != *src) {
			*dst = *src;
			is_changed = true;
		}
		dst++;
		src++;
	}

	return (is_changed ? p : nullptr);
}

void Dmx::SetSendDataLength(uint16_t nLength) {
	dmx_send_data_length = nLength;
	SetDmxPeriodTime(m_nDmxTransmitPeriodRequested);
}

void Dmx::SetSendData(const uint8_t *pData, uint16_t nLength) {
	do {
		dmb();
	} while (dmx_send_state != IDLE && dmx_send_state != DMXINTER);

	__builtin_prefetch(pData);
	memcpy(dmx_data[0].Data, pData, nLength);

	SetSendDataLength(nLength);
}

void Dmx::SetSendDataWithoutSC(const uint8_t *data, uint16_t nLength) {
	do {
		dmb();
	} while (dmx_send_state != IDLE && dmx_send_state != DMXINTER);

	dmx_data[0].Data[0] = START_CODE;

	__builtin_prefetch(data);
	memcpy(&dmx_data[0].Data[1], data, nLength);

	SetSendDataLength(nLength + 1);
}

uint32_t Dmx::GetUpdatesPerSecond() {
	dmb();
	return dmx_updates_per_seconde;
}

void Dmx::ClearData() {
	auto i = sizeof(dmx_data) / sizeof(uint32_t);
	auto *p = reinterpret_cast<uint32_t *>(dmx_data);

	while (i-- != 0) {
		*p++ = 0;
	}
}

// RDM

uint32_t Dmx::RdmGetDateReceivedEnd() {
	return rdm_data_receive_end;
}

const uint8_t *Dmx::RdmReceive(__attribute__((unused)) uint32_t nPort) {
	assert(nPort == 0);

	dmb();
	if (rdm_data_buffer_index_head == rdm_data_buffer_index_tail) {
		return nullptr;
	} else {
		const auto *p = &rdm_data_buffer[rdm_data_buffer_index_tail][0];
		rdm_data_buffer_index_tail = (rdm_data_buffer_index_tail + 1) & RDM_DATA_BUFFER_INDEX_MASK;
		return p;
	}
}

const uint8_t* Dmx::RdmReceiveTimeOut(uint32_t nPort, uint32_t nTimeOut) {
	assert(nPort == 0);

	uint8_t *p = nullptr;
	const auto nMicros = H3_TIMER->AVS_CNT1;

	do {
		if ((p = const_cast<uint8_t*>(RdmReceive(nPort))) != nullptr) {
			return p;
		}
	} while ((H3_TIMER->AVS_CNT1 - nMicros) < nTimeOut);

	return p;
}

void Dmx::RdmSendRaw(__attribute__((unused)) uint32_t nPort, const uint8_t *pRdmData, uint16_t nLength) {
	assert(nPort == 0);

	while (!(EXT_UART->LSR & UART_LSR_TEMT))
		;

	EXT_UART->LCR = UART_LCR_8_N_2 | UART_LCR_BC;
	udelay(RDM_TRANSMIT_BREAK_TIME);

	EXT_UART->LCR = UART_LCR_8_N_2;
	udelay(RDM_TRANSMIT_MAB_TIME);

	for (uint16_t i = 0; i < nLength; i++) {
		while (!(EXT_UART->LSR & UART_LSR_THRE))
			;
		EXT_UART->O00.THR = pRdmData[i];
	}

	while ((EXT_UART->USR & UART_USR_BUSY) == UART_USR_BUSY) {
		(void) EXT_UART->O00.RBR;
	}
}
