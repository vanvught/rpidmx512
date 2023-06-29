/*
 * config_delay.cpp
 */

#include "configstore.h"

namespace configstore {
	void delay() {
		ConfigStore::Get()->Delay();
	}
}  // namespace configstore


