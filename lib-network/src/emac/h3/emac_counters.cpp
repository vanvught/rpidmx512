/*
 * emac_counters.cpp
 */

#include <cstring>

#include "emac/network.h"

namespace network::iface {
void GetCounters(Counters& out)
{
	memset(&out, 0, sizeof(Counters));
}
}
