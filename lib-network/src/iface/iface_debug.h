/**
 * @file iface_debug.h
 *
 */

#ifndef IFACE_DEBUG_H_
#define IFACE_DEBUG_H_

#include "firmware/debug/debug_debug.h"

#if defined(DEBUG_NETWORK_IFACE)
#define NETWORK_IFACE_DEBUG_ENTRY() DEBUG_ENTRY()
#define NETWORK_IFACE_DEBUG_EXIT() DEBUG_EXIT()
#define NETWORK_IFACE_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__)
#define NETWORK_IFACE_DEBUG_PUTS(...) DEBUG_PUTS(__VA_ARGS__)
#else
#define NETWORK_IFACE_DEBUG_ENTRY() \
    do {                            \
    } while (false)
#define NETWORK_IFACE_DEBUG_EXIT() \
    do {                           \
    } while (false)
#define NETWORK_IFACE_DEBUG_PRINTF(...) \
    do {                                \
    } while (false)
#define NETWORK_IFACE_DEBUG_PUTS(...) \
    do {                              \
    } while (false)
#endif

#endif // IFACE_DEBUG_H_
