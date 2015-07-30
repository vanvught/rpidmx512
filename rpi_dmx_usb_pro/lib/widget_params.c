/**
 * @file widget_params.c
 */
/* Copyright (C) 2015 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
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

#include <stdint.h>
#include <string.h>

#include "ff.h"
#include "util.h"
#include "dmx.h"
#include "widget.h"
#include "widget_params.h"
#include "sscan.h"

static const uint8_t DEVICE_TYPE_ID[DEVICE_TYPE_ID_LENGTH] __attribute__((aligned(4))) = { (uint8_t) 1, (uint8_t) 0 };

static struct _widget_params dmx_usb_pro_params __attribute__((aligned(4))) = {
		(uint8_t) WIDGET_DEFAULT_FIRMWARE_LSB, (uint8_t) FIRMWARE_RDM,
		(uint8_t) WIDGET_DEFAULT_BREAK_TIME, (uint8_t) WIDGET_DEFAULT_MAB_TIME,
		(uint8_t) WIDGET_DEFAULT_REFRESH_RATE };

static uint8_t dmx_send_to_host_throttle = 0;

static const TCHAR PARAMS_FILE_NAME[] = "params.txt";								///< Parameters file name
///< entries
static const char DMXUSBPRO_PARAMS_BREAK_TIME[] = "dmxusbpro_break_time";			///<
static const char DMXUSBPRO_PARAMS_MAB_TIME[] = "dmxusbpro_mab_time";				///<
static const char DMXUSBPRO_PARAMS_REFRESH_RATE[] = "dmxusbpro_refresh_rate";		///<
///< custom entries
static const char PARAMS_WIDGET_MODE[] = "widget_mode";								///<
static const char PARAMS_DMX_SEND_TO_HOST_THROTTLE[] = "dmx_send_to_host_throttle";	///<

/**
 *
 * @param line
 * @param file_object_wr
 * @param name
 * @param value
 * @return
 */
#ifdef UPDATE_CONFIG_FILE
#include <stdio.h>
static char process_line_update(const char *line, FIL file_object_wr, const char *name, const int value) {
	char _name[64];
	int _value;

	if (sscanf(line, "%[^=]=%d", _name, &_value) == 2) {
		if (strncmp(_name, name, _strlen(name)) == 0) {
			TCHAR buffer[128];
			sprintf(buffer, "%s=%d\n", name, value);
			f_puts(buffer, &file_object_wr);
			return 1;

		} else {
			f_puts(line, &file_object_wr);
		}
	}

	return 0;
}

/**
 *
 * @param name
 * @param value
 */
static void update_config_file(const char *name, const int value) {
	int rc = -1;

	FATFS fat_fs;
	FIL file_object_rd;

	f_mount(0, &fat_fs);		// Register volume work area (never fails)

	rc = f_open(&file_object_rd, PARAMS_FILE_NAME, FA_READ);
	if (rc == FR_OK) {
		FIL file_object_wr;
		rc = f_open(&file_object_wr, "tmp.txt", FA_WRITE | FA_CREATE_ALWAYS);
		if (rc == FR_OK) {
			TCHAR buffer[128];
			char found = 0;
			for (;;) {
				if (f_gets(buffer, sizeof(buffer), &file_object_rd) == NULL)
					break; // Error or end of file

				if (!found) {
					found = process_line_update((const char *) buffer, file_object_wr, name, value);
				} else {
					f_puts(buffer, &file_object_wr);
				}
			}
			f_close(&file_object_wr);
		}
		f_close(&file_object_rd);
	}
}
#else
inline static void update_config_file(/*@unused@*/const char *name, /*@unused@*/const int value) { }
#endif

/**
 * @ingroup widget
 *
 * @param line
 */
static void process_line_read_uint8_t(const char *line) {
	uint8_t value;

	if (sscan_uint8_t(line, DMXUSBPRO_PARAMS_BREAK_TIME, &value) == 2) {
		if ((value >= (uint8_t) WIDGET_MIN_BREAK_TIME) && (value <= (uint8_t) WIDGET_MAX_BREAK_TIME)) {
			dmx_usb_pro_params.break_time = value;
		}
	} else if (sscan_uint8_t(line, DMXUSBPRO_PARAMS_MAB_TIME, &value) == 2) {
		if ((value >= (uint8_t) WIDGET_MIN_MAB_TIME) && (value <= (uint8_t) WIDGET_MAX_MAB_TIME)) {
			dmx_usb_pro_params.mab_time = value;
		}
	} else if (sscan_uint8_t(line, DMXUSBPRO_PARAMS_REFRESH_RATE, &value) == 2) {
		dmx_usb_pro_params.refresh_rate = (uint8_t) value;
	} else if (sscan_uint8_t(line, PARAMS_WIDGET_MODE, &value) == 2) {
		if ((value >= (uint8_t) MODE_DMX_RDM) && value <= (uint8_t) MODE_RDM_SNIFFER) {
			dmx_usb_pro_params.firmware_msb = value;
		}
	} else if (sscan_uint8_t(line, PARAMS_DMX_SEND_TO_HOST_THROTTLE, &value) == 2) {
		dmx_send_to_host_throttle = value;
	}

	return;
}

/**
 * @ingroup widget
 *
 */
static void read_config_file(void) {
	FRESULT rc = FR_DISK_ERR;

	FATFS fat_fs;
	FIL file_object;

	rc = f_mount((BYTE) 0, &fat_fs);// Register volume work area (never fails)

	rc = f_open(&file_object, PARAMS_FILE_NAME, (BYTE) FA_READ);

	if (rc == FR_OK) {
		TCHAR buffer[128];
		for (;;) {
			if (f_gets(buffer, (int) sizeof(buffer), &file_object) == NULL)
				break; // Error or end of file
			(void) process_line_read_uint8_t((const char *) buffer);
		}
		(void) f_close(&file_object);
	} else {
		// nothing to do here
	}
}

/**
 * @ingroup widget
 *
 * @param widget_params
 */
void widget_params_get(struct _widget_params *widget_params) {
	widget_params->break_time = dmx_usb_pro_params.break_time;
	widget_params->firmware_lsb = dmx_usb_pro_params.firmware_lsb;
	widget_params->firmware_msb = dmx_usb_pro_params.firmware_msb;
	widget_params->mab_time = dmx_usb_pro_params.mab_time;
	widget_params->refresh_rate = dmx_usb_pro_params.refresh_rate;
}

/**
 * @ingroup widget
 *
 * @param break_time
 */
void widget_params_set_break_time(const uint8_t break_time) {
	dmx_usb_pro_params.break_time = break_time;
	dmx_set_output_break_time((uint32_t) ((double) (dmx_usb_pro_params.break_time) * (double) (10.67)));
	update_config_file(DMXUSBPRO_PARAMS_BREAK_TIME, (int)break_time);
}

/**
 * @ingroup widget
 *
 * @param mab_time
 */
void widget_params_set_mab_time(const uint8_t mab_time) {
	dmx_usb_pro_params.mab_time = mab_time;
	dmx_set_output_mab_time((uint32_t) ((double) (dmx_usb_pro_params.mab_time) * (double) (10.67)));
	update_config_file(DMXUSBPRO_PARAMS_MAB_TIME, (int) mab_time);
}

/**
 * @ingroup widget
 *
 * @param refresh_rate
 */
void widget_params_set_refresh_rate(const uint8_t refresh_rate) {
	dmx_usb_pro_params.refresh_rate = refresh_rate;
	dmx_set_output_period(refresh_rate == (uint8_t) 0 ? (uint32_t) 0 : (uint32_t) (1E6 / refresh_rate));
	update_config_file(DMXUSBPRO_PARAMS_REFRESH_RATE, (int)refresh_rate);
}

/**
 * @ingroup widget
 *
 * @return
 */
const uint8_t * widget_params_get_type_id(void) {
	return DEVICE_TYPE_ID;
}

/**
 * @ingroup widget
 *
 * @return
 */
const uint8_t widget_params_get_type_id_length(void) {
	return (uint8_t) DEVICE_TYPE_ID_LENGTH;
}

/**
 * @ingroup widget
 *
 * @return
 */
const uint8_t widget_params_get_throttle(void) {
	return dmx_send_to_host_throttle;
}

/**
 *  @ingroup widget
 *
 * @param throttle
 */
void widget_params_set_throttle(const uint8_t throttle) {
	dmx_send_to_host_throttle = throttle;
}

/**
 * @ingroup widget
 *
 * Update the Widget with the settings from params.txt
 */
void widget_params_init(void) {
	uint32_t period;
	uint8_t mode;

	read_config_file();

	period = 0;

	if (dmx_usb_pro_params.refresh_rate != 0) {
		period = (uint32_t) (1E6 / dmx_usb_pro_params.refresh_rate);
	}

	dmx_set_output_period(period);

	period = 0;

	if (dmx_send_to_host_throttle != 0) {
		period = (uint32_t) (1E6 / dmx_send_to_host_throttle);
	}

	widget_set_received_dmx_packet_period(period);

	mode = dmx_usb_pro_params.firmware_msb;

	if (mode == MODE_DMX_RDM) {
		dmx_usb_pro_params.firmware_msb = FIRMWARE_RDM;
	}

	widget_set_mode(mode);

	dmx_set_output_break_time((uint32_t) ((double) (dmx_usb_pro_params.break_time) * (double) (10.67)));
	dmx_set_output_mab_time((uint32_t) ((double) (dmx_usb_pro_params.mab_time) * (double) (10.67)));
}
