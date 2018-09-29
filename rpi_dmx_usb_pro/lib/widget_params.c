/**
 * @file widget_params.c
 */
/* Copyright (C) 2015, 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "ff.h"
#include "util.h"
#include "sscan.h"

#include "widget.h"
#include "widget_params.h"

#include "dmx.h"

///<
static const uint8_t DEVICE_TYPE_ID[DEVICE_TYPE_ID_LENGTH] __attribute__((aligned(4))) = { (uint8_t) 1, (uint8_t) 0 };

///<
static struct _widget_params dmx_usb_pro_params __attribute__((aligned(4))) = {
		(uint8_t) WIDGET_DEFAULT_FIRMWARE_LSB, (uint8_t) FIRMWARE_RDM,
		(uint8_t) WIDGET_DEFAULT_BREAK_TIME, (uint8_t) WIDGET_DEFAULT_MAB_TIME,
		(uint8_t) WIDGET_DEFAULT_REFRESH_RATE };

static uint8_t dmx_send_to_host_throttle = 0;										///<

static const TCHAR FILE_NAME_PARAMS[] = "params.txt";								///< Parameters file name
#ifdef UPDATE_CONFIG_FILE
static const TCHAR FILE_NAME_PARAMS_BAK[] = "params.bak";							///<
static const TCHAR FILE_NAME_UPDATES[] = "updates.txt";								///<
#endif

static const char DMXUSBPRO_PARAMS_BREAK_TIME[] = "dmxusbpro_break_time";			///<
static const char DMXUSBPRO_PARAMS_MAB_TIME[] = "dmxusbpro_mab_time";				///<
static const char DMXUSBPRO_PARAMS_REFRESH_RATE[] = "dmxusbpro_refresh_rate";		///<

static const char PARAMS_WIDGET_MODE[] = "widget_mode";								///<
static const char PARAMS_DMX_SEND_TO_HOST_THROTTLE[] = "dmx_send_to_host_throttle";	///<

typedef enum {
	AI_BREAK_TIME = 0,
	AI_MAB_TIME,
	AI_REFRESH_RATE
} _array_index;

static bool needs_update[3] = { false, false, false };

#ifdef UPDATE_CONFIG_FILE
static char *uint8_toa(uint8_t i) {
	/* Room for 3 digits and '\0' */
	static char buffer[4];
	char *p = &buffer[3]; /* points to terminating '\0' */

	*p = (char) '\0';

	do {
		*--p = (char) '0' + (char) (i % 10);
		i /= 10;
	} while (i != 0);

	return p;
}

static void sprintf_name_value(char *buffer, const char *name, const uint8_t value) {
	char *dst = buffer;
	const char *src = name;

	while (*src != (char) '\0') {
		*dst++ = *src++;
	}

	*dst++ = (char) '=';

	char *p = uint8_toa(value);

	while (*p != (char) '\0') {
		*dst++ = *p++;
	}

	*dst++ = (char) '\n';
	*dst = (char) '\0';
}

static void process_line_update(const char *line, FIL *file_object_wr) {
	TCHAR buffer[128];
	uint8_t _value;
	int i;

	if (needs_update[AI_BREAK_TIME]) {
		if (sscan_uint8_t(line, DMXUSBPRO_PARAMS_BREAK_TIME, &_value) == 2) {
			sprintf_name_value(buffer, DMXUSBPRO_PARAMS_BREAK_TIME, dmx_usb_pro_params.break_time);
			(void) f_puts(buffer, file_object_wr);
			needs_update[AI_BREAK_TIME] = false;
			return;
		}
	}

	if (needs_update[AI_MAB_TIME]) {
		if (sscan_uint8_t(line, DMXUSBPRO_PARAMS_MAB_TIME, &_value) == 2) {
			sprintf_name_value(buffer, DMXUSBPRO_PARAMS_MAB_TIME, dmx_usb_pro_params.mab_time);
			(void) f_puts(buffer, file_object_wr);
			needs_update[AI_MAB_TIME] = false;
			return;
		}
	}

	if (needs_update[AI_REFRESH_RATE]) {
		if (sscan_uint8_t(line, DMXUSBPRO_PARAMS_REFRESH_RATE, &_value) == 2) {
			sprintf_name_value(buffer, DMXUSBPRO_PARAMS_REFRESH_RATE, dmx_usb_pro_params.refresh_rate);
			(void) f_puts(buffer, file_object_wr);
			needs_update[AI_REFRESH_RATE] = false;
			return;
		}
	}

	(void) f_puts(line, file_object_wr);
	i = (int) strlen(line) - 1;
	if (line[i] != (char) '\n') {
		(void) f_putc((TCHAR) '\n', file_object_wr);
	}

}

static void update_config_file(void) {
	TCHAR buffer[128];
	FIL file_object_rd;
	FIL file_object_wr;
	FRESULT rc_rd = FR_DISK_ERR;
	FRESULT rc_wr = FR_DISK_ERR;

	rc_rd = f_open(&file_object_rd, FILE_NAME_PARAMS, (BYTE) FA_READ);
	rc_wr = f_open(&file_object_wr, FILE_NAME_UPDATES, (BYTE) (FA_WRITE | FA_CREATE_ALWAYS));

	if (rc_wr == FR_OK) {

		if (rc_rd == FR_OK) {
			for (;;) {
				if (f_gets(buffer, (int) sizeof(buffer), &file_object_rd) == NULL) {
					break; // Error or end of file
				}
				process_line_update((const char *) buffer, &file_object_wr);
			}
		}

		if (needs_update[AI_BREAK_TIME]) {
			sprintf_name_value(buffer, DMXUSBPRO_PARAMS_BREAK_TIME, dmx_usb_pro_params.break_time);
			(void) f_puts(buffer, &file_object_wr);
			needs_update[AI_BREAK_TIME] = false;
		}

		if (needs_update[AI_MAB_TIME]) {
			sprintf_name_value(buffer, DMXUSBPRO_PARAMS_MAB_TIME, dmx_usb_pro_params.mab_time);
			(void) f_puts(buffer, &file_object_wr);
			needs_update[AI_MAB_TIME] = false;
		}

		if (needs_update[AI_REFRESH_RATE]) {
			sprintf_name_value(buffer, DMXUSBPRO_PARAMS_REFRESH_RATE, dmx_usb_pro_params.refresh_rate);
			(void) f_puts(buffer, &file_object_wr);
			needs_update[AI_REFRESH_RATE] = false;
		}

		(void) f_close(&file_object_wr);
		if (rc_rd == FR_OK) {
			(void) f_close(&file_object_rd);
		}
		(void) f_unlink(FILE_NAME_PARAMS_BAK);
		(void) f_rename(FILE_NAME_PARAMS, FILE_NAME_PARAMS_BAK);
		(void) f_rename(FILE_NAME_UPDATES, FILE_NAME_PARAMS);
	}
}
#else
inline static void update_config_file(void) { }
#endif

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
}

static void read_config_file(void) {
	FRESULT rc = FR_DISK_ERR;
	FIL file_object;

	rc = f_open(&file_object, FILE_NAME_PARAMS, (BYTE) FA_READ);

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

void widget_params_get(struct _widget_params *widget_params) {
	widget_params->break_time = dmx_usb_pro_params.break_time;
	widget_params->firmware_lsb = dmx_usb_pro_params.firmware_lsb;
	widget_params->firmware_msb = dmx_usb_pro_params.firmware_msb;
	widget_params->mab_time = dmx_usb_pro_params.mab_time;
	widget_params->refresh_rate = dmx_usb_pro_params.refresh_rate;
}

void widget_params_set(const struct _widget_params *widget_params) {
	bool call_update_config_file = false;

	if (widget_params->break_time != dmx_usb_pro_params.break_time) {
		dmx_usb_pro_params.break_time = widget_params->break_time;
		dmx_set_output_break_time((uint32_t) ((double) (dmx_usb_pro_params.break_time) * (double) (10.67)));
		needs_update[AI_BREAK_TIME] = true;
		call_update_config_file = true;
	}

	if (widget_params->mab_time != dmx_usb_pro_params.mab_time) {
		dmx_usb_pro_params.mab_time = widget_params->mab_time;
		dmx_set_output_mab_time((uint32_t) ((double) (dmx_usb_pro_params.mab_time) * (double) (10.67)));
		needs_update[AI_MAB_TIME] = true;
		call_update_config_file = true;
	}

	if (widget_params->refresh_rate != dmx_usb_pro_params.refresh_rate) {
		dmx_usb_pro_params.refresh_rate = widget_params->refresh_rate;
		dmx_set_output_period(widget_params->refresh_rate == (uint8_t) 0 ? (uint32_t) 0 : (uint32_t) (1E6 / widget_params->refresh_rate));
		needs_update[AI_REFRESH_RATE] = true;
		call_update_config_file = true;
	}

	if (call_update_config_file) {
		update_config_file();
	}
}

void widget_params_get_type_id(struct _widget_params_data *info) {
	info->data = (uint8_t *) DEVICE_TYPE_ID;
	info->length = (uint8_t) DEVICE_TYPE_ID_LENGTH;
}

const uint8_t widget_params_get_throttle(void) {
	return dmx_send_to_host_throttle;
}

void widget_params_set_throttle(const uint8_t throttle) {
	dmx_send_to_host_throttle = throttle;
}

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
