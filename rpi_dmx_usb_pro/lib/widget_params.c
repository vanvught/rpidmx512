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

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "ff.h"
#include "dmx.h"
#include "widget.h"
#include "widget_params.h"

static const uint8_t DEVICE_TYPE_ID[DEVICE_TYPE_ID_LENGTH] = {1, 0};
static struct _widget_params dmx_usb_pro_params = { 4, FIRMWARE_RDM, 9, 1, 40 };
static uint8_t dmx_send_to_host_throttle = 0;

static const TCHAR PARAMS_FILE_NAME[] = "params.txt";							///< Parameters file name
///< entries
static const char DMXUSBPRO_PARAMS_BREAK_TIME[] = "dmxusbpro_break_time";		///<
static const char DMXUSBPRO_PARAMS_MAB_TIME[] = "dmxusbpro_mab_time";			///<
static const char DMXUSBPRO_PARAMS_REFRESH_RATE[] = "dmxusbpro_refresh_rate";	///<
///< custom entry
static const char PARAMS_WIDGET_MODE[] = "mode";									///<
static const char PARAMS_DMX_SEND_TO_HOST_THROTTLE[] = "dmx_send_to_host_throttle";	///<

#ifdef UPDATE_CONFIG_FILE
/**
 * @ingroup widget
 *
 * @param line
 * @param file_object_wr
 * @param name
 * @param value
 * @return
 */
static char process_line_update(const char *line, FIL file_object_wr, const char *name, const int value)
{
	char _name[64];
	int _value;

	if (sscanf(line, "%[^=]=%d", _name, &_value) == 2)
	{
		if (strncmp(_name, name, strlen(name)) == 0)
		{
			TCHAR buffer[128];
			sprintf(buffer, "%s=%d\n", name, value);
			f_puts(buffer, &file_object_wr);
			return 1;

		} else
		{
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

static void update_config_file(const char *name, const int value)
{
	int rc = -1;

	FATFS fat_fs;
	FIL file_object_rd;

	f_mount(0, &fat_fs);		// Register volume work area (never fails)

	rc = f_open(&file_object_rd, PARAMS_FILE_NAME, FA_READ);
	if (rc == FR_OK)
	{
		FIL file_object_wr;
		rc = f_open(&file_object_wr, "tmp.txt", FA_WRITE | FA_CREATE_ALWAYS);
		if (rc == FR_OK)
		{
			TCHAR buffer[128];
			char found = 0;
			for (;;)
			{
				if (f_gets(buffer, sizeof(buffer), &file_object_rd) == NULL)
					break; // Error or end of file

				if (!found)
				{
					found = process_line_update((const char *) buffer, file_object_wr, name, value);
				} else
				{
					f_puts(buffer, &file_object_wr);
				}
			}
			f_close(&file_object_wr);
		}
		f_close(&file_object_rd);
	}
}
#else
inline static void update_config_file(const char *name, const int value) { }
#endif

/**
 * @ingroup widget
 *
 * @param line
 */
static void process_line_read(const char *line)
{
	char name[64];
	int value;
	if (sscanf(line, "%[^=]=%d", name, &value) == 2)
	{
		if (strncmp(name, DMXUSBPRO_PARAMS_BREAK_TIME, sizeof(DMXUSBPRO_PARAMS_BREAK_TIME)) == 0)
		{
			if((value >= 9) && (value <= 127))		// DMX output break time in 10.67 μs units. Valid range is 9 to 127
			{
				dmx_usb_pro_params.break_time = value;
			}
		} else if  (strncmp(name, DMXUSBPRO_PARAMS_MAB_TIME, sizeof(DMXUSBPRO_PARAMS_MAB_TIME)) == 0)
		{
			if((value >= 1) && (value <= 127))		// DMX output Mark After Break time in 10.67 μs units. Valid range is 1 to 127.
			{
				dmx_usb_pro_params.mab_time = value;
			}
		} else if  (strncmp(name, DMXUSBPRO_PARAMS_REFRESH_RATE, sizeof(DMXUSBPRO_PARAMS_REFRESH_RATE)) == 0)
		{
			if((value >= 0) && (value <= 40))		// DMX output rate in packets per second. Valid range is 1 to 40.
			{
				dmx_usb_pro_params.refresh_rate = value;
			}
		} else if  (strncmp(name, PARAMS_WIDGET_MODE, sizeof(PARAMS_WIDGET_MODE)) == 0)
		{
			if((value >= MODE_DMX_RDM) && value <= MODE_RDM_SNIFFER)
			{
				dmx_usb_pro_params.firmware_msb = value;
			}
		} else if  (strncmp(name, PARAMS_DMX_SEND_TO_HOST_THROTTLE, sizeof(PARAMS_DMX_SEND_TO_HOST_THROTTLE)) == 0)
		{
			dmx_send_to_host_throttle = value;
		}
	}
}

/**
 * @ingroup widget
 *
 */
static void read_config_file(void)
{
	int rc = -1;

	FATFS fat_fs;
	FIL file_object;

	f_mount(0, &fat_fs);		// Register volume work area (never fails)

	rc = f_open(&file_object, PARAMS_FILE_NAME, FA_READ);

	if (rc == FR_OK)
	{
		TCHAR buffer[128];
		for (;;)
		{
			if (f_gets(buffer, sizeof buffer, &file_object) == NULL)
				break; // Error or end of file
			process_line_read((const char *) buffer);
		}
		f_close(&file_object);
	}
	else
	{
	}

}

/**
 * @ingroup widget
 *
 * @param widget_params
 */
void widget_params_get(struct _widget_params *widget_params)
{
	memcpy(widget_params, &dmx_usb_pro_params, sizeof(struct _widget_params));
}

/**
 * @ingroup widget
 *
 * @param break_time
 */
void widget_params_set_break_time(const uint8_t break_time)
{
	dmx_usb_pro_params.break_time = break_time;
	dmx_output_break_time_set((double)(dmx_usb_pro_params.break_time) * (double)(10.67));
	update_config_file(DMXUSBPRO_PARAMS_BREAK_TIME, break_time);
}

/**
 * @ingroup widget
 *
 * @param mab_time
 */
void widget_params_set_mab_time(const uint8_t mab_time)
{
	dmx_usb_pro_params.mab_time = mab_time;
	dmx_output_mab_time_set((double)(dmx_usb_pro_params.mab_time) * (double)(10.67));
	update_config_file(DMXUSBPRO_PARAMS_MAB_TIME, mab_time);
}

/**
 * @ingroup widget
 *
 * @param refresh_rate
 */
void widget_params_set_refresh_rate(const uint8_t refresh_rate)
{
	dmx_usb_pro_params.refresh_rate = refresh_rate;
	dmx_set_output_period(refresh_rate == 0 ? 0 : (1E6 / refresh_rate));
	update_config_file(DMXUSBPRO_PARAMS_REFRESH_RATE, refresh_rate);
}

/**
 * @ingroup widget
 *
 * @return
 */
const uint8_t * widget_params_get_type_id(void)
{
	return DEVICE_TYPE_ID;
}

/**
 * @ingroup widget
 *
 * @return
 */
const uint8_t widget_params_get_type_id_length(void)
{
	return DEVICE_TYPE_ID_LENGTH;
}

/**
 * @ingroup widget
 *
 * @return
 */
const uint8_t widget_params_get_throttle(void)
{
	return dmx_send_to_host_throttle;
}

void  widget_params_set_throttle(const uint8_t throttle)
{
	dmx_send_to_host_throttle = throttle;
}

/**
 * @ingroup widget
 *
 * Update the Widget with the settings from params.txt
 */
void widget_params_init(void)
{
	read_config_file();

	// DMX output rate in packets per second. Valid range is 1 to 40.
	uint32_t period = 0;	// us
	if (dmx_usb_pro_params.refresh_rate > 0 && dmx_usb_pro_params.refresh_rate <= 40)
	{
		period = 1E6 / dmx_usb_pro_params.refresh_rate;
	}

	dmx_set_output_period(period);

	uint8_t mode = dmx_usb_pro_params.firmware_msb;
	if (mode == MODE_DMX_RDM)
		dmx_usb_pro_params.firmware_msb = FIRMWARE_RDM;

	widget_mode_set(mode);

	dmx_output_break_time_set((double)(dmx_usb_pro_params.break_time) * (double)(10.67));
	dmx_output_mab_time_set((double)(dmx_usb_pro_params.mab_time) * (double)(10.67));
}
