#include <assert.h>

#include "led.h"
#include "blinktask.h"

CBlinkTask::CBlinkTask(void) {

}

/**
 *
 */
CBlinkTask::~CBlinkTask(void) {

}

/**
 *
 * @param nFreqHz
 */
void CBlinkTask::SetFrequency(unsigned nFreqHz) {
	if (nFreqHz == 0) {
		led_set_ticks_per_second(0);
	} else {
		led_set_ticks_per_second(1000000 / nFreqHz);
	}
}

