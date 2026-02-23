/**
 * @file debug_debug.h
 *
 */
/* Copyright (C) 2018-2025 by Arjan van Vught mailto:info@gd32-dmx.org */

#ifndef FIRMWARE_DEBUG_DEBUG_H_
#define FIRMWARE_DEBUG_DEBUG_H_

#if !defined(NDEBUG)
#include <cstdio>

#define DEBUG_ENTRY()                                          \
    do                                                         \
    {                                                          \
        printf("-> %s:%s:%d\n", __FILE__, __func__, __LINE__); \
    } while (0)

#define DEBUG_EXIT()                                           \
    do                                                         \
    {                                                          \
        printf("<- %s:%s:%d\n", __FILE__, __func__, __LINE__); \
    } while (0)

#define DEBUG_PRINTF(fmt, ...)                                                                    \
    do                                                                                            \
    {                                                                                             \
        printf("%s() %s:%d: " fmt "\n", __func__, __FILE__, __LINE__ __VA_OPT__(, ) __VA_ARGS__); \
    } while (0)

#define DEBUG_PUTS(msg)            \
    do                             \
    {                              \
        DEBUG_PRINTF("%s", (msg)); \
    } while (0)

#else

#define DEBUG_ENTRY() \
    do                \
    {                 \
    } while (0)

#define DEBUG_EXIT() \
    do               \
    {                \
    } while (0)

#define DEBUG_PRINTF(...) \
    do                    \
    {                     \
    } while (0)

#define DEBUG_PUTS(...) \
    do                  \
    {                   \
    } while (0)

#endif

#endif // FIRMWARE_DEBUG_DEBUG_H_
