/**
 * @file dmx.h
 *
 */
/* Copyright (C) 2015-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef DMX_H_
#define DMX_H_

#include <stdint.h>
#include <stdbool.h>

#if defined(H3)
 #if defined(ORANGE_PI_ONE)
  #define DMX_MAX_UARTS	4
 #else
  #define DMX_MAX_UARTS	2	///< Orange Pi Zero & NanoPi NEO
 #endif
#else
 #define DMX_MAX_UARTS	1	///< All Raspberry Pi's
#endif

#define DMX_MAX_OUT		4

#define DMX_DATA_BUFFER_SIZE					516									///< including SC, aligned 4
#define DMX_DATA_BUFFER_INDEX_ENTRIES			(1 << 1)							///<
#define DMX_DATA_BUFFER_INDEX_MASK 				(DMX_DATA_BUFFER_INDEX_ENTRIES - 1)	///<

#define DMX_TRANSMIT_BREAK_TIME_MIN				92		///< 92 us
#define DMX_TRANSMIT_BREAK_TIME_TYPICAL			176		///< 176 us
#define DMX_TRANSMIT_MAB_TIME_MIN				12		///< 12 us
#define DMX_TRANSMIT_MAB_TIME_MAX				1E6		///< 1s
#define DMX_TRANSMIT_REFRESH_RATE_DEFAULT		40		///< 40 Hz
#define DMX_TRANSMIT_PERIOD_DEFAULT				(uint32_t)(1E6 / DMX_TRANSMIT_REFRESH_RATE_DEFAULT)	///< 25000 us
#define DMX_TRANSMIT_BREAK_TO_BREAK_TIME_MIN	1204	///< us

#define DMX_MIN_SLOT_VALUE 						0		///< The minimum value a DMX512 slot can take.
#define DMX_MAX_SLOT_VALUE 						255		///< The maximum value a DMX512 slot can take.
#define DMX512_START_CODE						0		///< The start code for DMX512 data. This is often referred to as NSC for "Null Start Code".

enum {
	DMX_UNIVERSE_SIZE = 512		///< The number of slots in a DMX512 universe.
};

typedef enum {
	DMX_PORT_DIRECTION_OUTP,	///< DMX output
	DMX_PORT_DIRECTION_INP		///< DMX input
} _dmx_port_direction;

struct _dmx_statistics {
	uint32_t mark_after_break;
	uint32_t slots_in_packet;
	uint32_t break_to_break;
	uint32_t slot_to_slot;
};

struct _dmx_data {
	uint8_t data[DMX_DATA_BUFFER_SIZE];
	struct _dmx_statistics statistics;
};

struct _total_statistics {
	uint32_t dmx_packets;
	uint32_t rdm_packets;
};

#ifdef __cplusplus
extern "C" {
#endif

extern void dmx_init_set_gpiopin(uint8_t);
extern void dmx_init(void);

extern void dmx_set_send_data(const uint8_t *, uint16_t);
extern void dmx_set_send_data_without_sc(const uint8_t *, uint16_t);
extern void dmx_clear_data(void);
extern void dmx_set_port_direction(_dmx_port_direction, bool);
extern _dmx_port_direction dmx_get_port_direction(void);
extern void dmx_data_send(const uint8_t *, uint16_t);
extern /*@shared@*/const /*@null@*/uint8_t *dmx_get_available(void) __attribute__((assume_aligned(4)));
extern /*@shared@*/const uint8_t *dmx_get_current_data(void) __attribute__((assume_aligned(4)));
extern /*@shared@*/const uint8_t *dmx_is_data_changed(void) __attribute__((assume_aligned(4)));
extern uint32_t dmx_get_output_break_time(void);
extern void dmx_set_output_break_time(uint32_t);
extern uint32_t dmx_get_output_mab_time(void);
extern void dmx_set_output_mab_time(uint32_t);
extern void dmx_reset_total_statistics(void);
extern /*@shared@*/const volatile struct _total_statistics *dmx_get_total_statistics(void) __attribute__((assume_aligned(4)));
extern const volatile uint32_t dmx_get_updates_per_seconde(void);
extern uint16_t dmx_get_send_data_length(void);
extern uint32_t dmx_get_output_period(void);
extern void dmx_set_output_period(uint32_t);
extern /*@shared@*/const /*@null@*/uint8_t *rdm_get_available(void) __attribute__((assume_aligned(4)));
extern /*@shared@*/const uint8_t *rdm_get_current_data(void) __attribute__((assume_aligned(4)));
extern uint32_t rdm_get_data_receive_end(void);

#ifdef __cplusplus
}
#endif

/*
 * C++ only
 */

#ifdef __cplusplus

#include "gpio.h"

enum TDmxRdmPortDirection {
	DMXRDM_PORT_DIRECTION_OUTP = DMX_PORT_DIRECTION_OUTP,
	DMXRDM_PORT_DIRECTION_INP = DMX_PORT_DIRECTION_INP
};

struct TDmxStatistics {
	uint32_t MarkAfterBreak;
	uint32_t SlotsInPacket;
	uint32_t BreakToBreak;
	uint32_t SlotToSlot;
};

struct TDmxData {
	uint8_t Data[DMX_DATA_BUFFER_SIZE];
	struct TDmxStatistics Statistics;
};

#if defined (H3)
class DmxSet {
public:
	DmxSet(void);
	virtual ~DmxSet(void);

	virtual void SetPortDirection(uint8_t nPort, TDmxRdmPortDirection tPortDirection, bool bEnableData)=0;

	virtual void RdmSendRaw(uint8_t nPort, const uint8_t *pRdmData, uint16_t nLength)=0;

	virtual const uint8_t *RdmReceive(uint8_t nPort)=0;
	virtual const uint8_t *RdmReceiveTimeOut(uint8_t nPort, uint32_t nTimeOut)=0;

public:
	inline static DmxSet* Get(void) {
		return s_pThis;
	}

private:
	static DmxSet *s_pThis;
};

class Dmx: public DmxSet {
public:
	Dmx(uint8_t nGpioPin = GPIO_DMX_DATA_DIRECTION, bool DoInit = true);
	~Dmx(void);

	void SetPortDirection(uint8_t nPort, TDmxRdmPortDirection tPortDirection, bool bEnableData = false);

	void RdmSendRaw(uint8_t nPort, const uint8_t *pRdmData, uint16_t nLength);

	const uint8_t *RdmReceive(uint8_t nPort);
	const uint8_t *RdmReceiveTimeOut(uint8_t nPort, uint32_t nTimeOut);
#else
class Dmx {
public:
	Dmx(uint8_t nGpioPin = GPIO_DMX_DATA_DIRECTION, bool DoInit = true);
	~Dmx(void);

	inline void SetPortDirection(uint8_t nPort, TDmxRdmPortDirection tPortDirection, bool bEnableData = false) {
		dmx_set_port_direction((_dmx_port_direction)tPortDirection, bEnableData);
	}
#endif
public: // DMX
	void Init(void);

	inline uint32_t GetUpdatesPerSecond(void) {
		return dmx_get_updates_per_seconde();
	}

	inline const uint8_t *GetDmxCurrentData(void) {
		return dmx_get_current_data();
	}

	inline const uint8_t *GetDmxAvailable(void) {
		return dmx_get_available();
	}

	inline void SetDmxBreakTime(uint32_t nBreakTime) {
		dmx_set_output_break_time(nBreakTime);
	}

	inline  uint32_t GetDmxBreakTime(void) {
		return dmx_get_output_break_time();
	}

	inline void SetDmxMabTime(uint32_t nMabTime) {
		dmx_set_output_mab_time(nMabTime);
	}

	inline uint32_t GetDmxMabTime(void) {
		return dmx_get_output_mab_time();
	}

	inline void SetDmxPeriodTime(uint32_t nPeriodTime) {
		dmx_set_output_period(nPeriodTime);
	}

	inline uint32_t GetDmxPeriodTime(void) {
		return dmx_get_output_period();
	}

private:
	bool m_IsInitDone;
};

#endif

#endif /* DMX_H_ */
