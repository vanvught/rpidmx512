/*
 * hal_statusled.cpp
 */

#if defined (DEBUG_HAL)
# undef NDEBUG
#endif

#include <cstdint>

#include "hal_statusled.h"

#include "debug.h"

namespace hal {
namespace global {
hal::StatusLedMode g_statusLedMode;
}  // namespace global

static bool s_doLock;

enum class ModeToFrequency {
	OFF_OFF = 0, NORMAL = 1, DATA = 3, FAST = 5, REBOOT = 8, OFF_ON = 255
};

#if !defined (CONFIG_HAL_USE_MINIMUM)

void __attribute__((weak)) display_statusled([[maybe_unused]] const hal::StatusLedMode mode) {
	DEBUG_PRINTF("mode=%u", static_cast<uint32_t>(mode));
}

void statusled_set_mode_with_lock(const StatusLedMode mode, const bool doLock) {
	s_doLock = false;
	statusled_set_mode(mode);
	s_doLock = doLock;
}

void statusled_set_mode(const hal::StatusLedMode mode) {
	if (s_doLock || (global::g_statusLedMode == mode)) {
		return;
	}

	global::g_statusLedMode = mode;

	switch (global::g_statusLedMode) {
	case hal::StatusLedMode::OFF_OFF:
		statusled_set_frequency(static_cast<uint32_t>(ModeToFrequency::OFF_OFF));
		break;
	case hal::StatusLedMode::OFF_ON:
		statusled_set_frequency(static_cast<uint32_t>(ModeToFrequency::OFF_ON));
		break;
	case hal::StatusLedMode::NORMAL:
		statusled_set_frequency(static_cast<uint32_t>(ModeToFrequency::NORMAL));
		break;
	case hal::StatusLedMode::DATA:
		statusled_set_frequency(static_cast<uint32_t>(ModeToFrequency::DATA));
		break;
	case hal::StatusLedMode::FAST:
		statusled_set_frequency(static_cast<uint32_t>(ModeToFrequency::FAST));
		break;
	case hal::StatusLedMode::REBOOT:
		statusled_set_frequency(static_cast<uint32_t>(ModeToFrequency::REBOOT));
		break;
	default:
		statusled_set_frequency(static_cast<uint32_t>(ModeToFrequency::OFF_OFF));
		break;
	}

	hal::display_statusled(global::g_statusLedMode);

	DEBUG_PRINTF("StatusLedMode=%u", static_cast<uint32_t>(global::g_statusLedMode));
}
#endif
}  // namespace hal
