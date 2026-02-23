/**
 * @file firmware.h
 *
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef FIRMWARE_H_
#define FIRMWARE_H_

#include <cstdint>

namespace firmware {
#if defined(__linux__) || defined (__APPLE__)
static constexpr char FILE_NAME[] = "dummy.bin";
#else
# if defined (H3)
#  if defined(ORANGE_PI)
static constexpr char FILE_NAME[] = "orangepi_zero.uImage";
#  else
static constexpr char FILE_NAME[] = "orangepi_one.uImage";
#  endif
# elif defined (GD32)
#  if defined (GD32F10X)
static constexpr char FILE_NAME[] = "gd32f107.bin";
#  elif defined (GD32F20X)
static constexpr char FILE_NAME[] = "gd32f207.bin";
#  elif defined (GD32F4XX)
static constexpr char FILE_NAME[] = "gd32f4xx.bin";
#  elif defined (GD32H7XX)
static constexpr char FILE_NAME[] = "gd32h7xx.bin";
#  else
#   error FAMILY is not defined
#  endif
# endif
#endif

static constexpr auto FILE_NAME_LENGTH = sizeof(FILE_NAME) - 1;

#if defined (H3)
// nuc-i5:~/uboot-spi/u-boot$ grep CONFIG_BOOTCOMMAND include/configs/sunxi-common.h
// #define CONFIG_BOOTCOMMAND "sf probe; sf read 48000000 180000 22000; bootm 48000000"
# define FIRMWARE_MAX_SIZE	0x22000			// 136K
# define OFFSET_UBOOT_SPI	0x000000
# define OFFSET_UIMAGE		0x180000
# define IH_LOAD			0x40000000
# define IH_EP				0x40000000
#elif defined (GD32)
# define IH_LOAD			0x08008000
# define IH_EP				0x08008000
# if defined (BOARD_GD32F107RC)
#  define OFFSET_UIMAGE		0x007000		// 28K
#  define FIRMWARE_MAX_SIZE (76 * 1024)		// 76K
# elif defined (BOARD_GD32F207RG)
#  define OFFSET_UIMAGE		0x008000		// 32K
#  define FIRMWARE_MAX_SIZE (234 * 1024)	// 234K
# elif defined (BOARD_GD32F207VC_2)
#  define OFFSET_UIMAGE		0x008000		// 32K
#  define FIRMWARE_MAX_SIZE (106 * 1024)	// 106K
# elif defined (BOARD_GD32F207VC_4)
#  define OFFSET_UIMAGE		0x008000		// 32K
#  define FIRMWARE_MAX_SIZE (106 * 1024)	// 106K
# elif defined (BOARD_GD32F207C_EVAL)
#  define OFFSET_UIMAGE		0x008000		// 32K
#  define FIRMWARE_MAX_SIZE (106 * 1024)	// 106K
# elif defined (BOARD_GD32F407RE)
#  define OFFSET_UIMAGE		0x008000		// 32K
#  define FIRMWARE_MAX_SIZE (116 * 1024)	// 116K
# elif defined (BOARD_BW_OPIDMX4)
#  define OFFSET_UIMAGE		0x008000		// 32K
#  define FIRMWARE_MAX_SIZE (116 * 1024)	// 116K
# elif defined (BOARD_DMX3)
#  define OFFSET_UIMAGE		0x008000		// 32K
#  define FIRMWARE_MAX_SIZE (116 * 1024)	// 116K
# elif defined (BOARD_DMX4)
#  define OFFSET_UIMAGE		0x008000		// 32K
#  define FIRMWARE_MAX_SIZE (116 * 1024)	// 116K
# elif defined (BOARD_GD32F450VE)
#  define OFFSET_UIMAGE		0x008000		// 32K
#  define FIRMWARE_MAX_SIZE (180 * 1024)	// 180K
# elif defined (BOARD_GD32F450VI)
#  define OFFSET_UIMAGE		0x008000		// 32K
#  define FIRMWARE_MAX_SIZE (234 * 1024)	// 234K
# elif defined (BOARD_16X4U_PIXEL)
#  define OFFSET_UIMAGE		0x008000		// 32K
#  define FIRMWARE_MAX_SIZE (234 * 1024)	// 234K
# elif defined (BOARD_GD32F470VG)
#  define OFFSET_UIMAGE		0x008000		// 32K
#  define FIRMWARE_MAX_SIZE (234 * 1024)	// 234K
# elif defined (BOARD_GD32F470Z_EVAL)
#  define OFFSET_UIMAGE		0x008000		// 32K
#  define FIRMWARE_MAX_SIZE (175 * 1024)	// 175K
# elif defined (BOARD_GD32H759I_EVAL)
#  define OFFSET_UIMAGE		0x008000		// 32K
#  define FIRMWARE_MAX_SIZE (300 * 1024)	// 300K
# else
#  error Board is not supported
# endif
#else
# define IH_LOAD			0x40000000
# define IH_EP				0x40000000
# define OFFSET_UIMAGE		0x0
# define FIRMWARE_MAX_SIZE  4096	// for dummy.bin
#endif

bool firmware_install_start(const uint8_t *buffer,  uint32_t buffer_size);
bool firmware_install_continue(const uint8_t *buffer,  uint32_t buffer_size);
bool firmware_install_end(const uint8_t *buffer, uint32_t buffer_size);
}  // namespace firmware

#endif  // FIRMWARE_H_
