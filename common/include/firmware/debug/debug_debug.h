/**
 * @file debug_debug.h
 *
 */
/* Copyright (C) 2018-2026 by Arjan van Vught mailto:info@gd32-dmx.org */

#ifndef FIRMWARE_DEBUG_DEBUG_H_
#define FIRMWARE_DEBUG_DEBUG_H_

#if !defined(NDEBUG)
#include <cstdio>
#include <source_location>

#define DEBUG_ENTRY()                                                                                      \
    do {                                                                                                   \
        const std::source_location loc = std::source_location::current();                                  \
        printf("-> %s(%u):%s\n", loc.file_name(), static_cast<unsigned>(loc.line()), loc.function_name()); \
    } while (0)

#define DEBUG_EXIT()                                                                                       \
    do {                                                                                                   \
        const std::source_location loc = std::source_location::current();                                  \
        printf("<- %s(%u):%s\n", loc.file_name(), static_cast<unsigned>(loc.line()), loc.function_name()); \
    } while (0)

#define DEBUG_PRINTF(fmt, ...)                                                                                                                   \
    do {                                                                                                                                         \
        const std::source_location loc = std::source_location::current();                                                                        \
        printf("   %s(%u):%s: " fmt "\n", loc.file_name(), static_cast<unsigned>(loc.line()), loc.function_name() __VA_OPT__(, ) __VA_ARGS__); \
    } while (0)

#define DEBUG_PUTS(msg)            \
    do {                           \
        DEBUG_PRINTF("%s", (msg)); \
    } while (0)

#else

#define DEBUG_ENTRY() \
    do {              \
    } while (0)

#define DEBUG_EXIT() \
    do {             \
    } while (0)

#define DEBUG_PRINTF(...) \
    do {                  \
    } while (0)

#define DEBUG_PUTS(...) \
    do {                \
    } while (0)

#endif

#endif // FIRMWARE_DEBUG_DEBUG_H_
