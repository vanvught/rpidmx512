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
#include <errno.h>

#include "ff.h"
#include "dmx.h"
#include "widget.h"
#include "widget_params.h"

static const uint8_t DEVICE_TYPE_ID[DEVICE_TYPE_ID_LENGTH] = {(uint8_t)1, (uint8_t)0};
static struct _widget_params dmx_usb_pro_params = { (uint8_t)4, (uint8_t)FIRMWARE_RDM, (uint8_t)9, (uint8_t)1, (uint8_t)40 };
static uint8_t dmx_send_to_host_throttle = 0;

static const TCHAR PARAMS_FILE_NAME[] = "params.txt";							///< Parameters file name
///< entries
static const char DMXUSBPRO_PARAMS_BREAK_TIME[] = "dmxusbpro_break_time";		///<
static const char DMXUSBPRO_PARAMS_MAB_TIME[] = "dmxusbpro_mab_time";			///<
static const char DMXUSBPRO_PARAMS_REFRESH_RATE[] = "dmxusbpro_refresh_rate";	///<
///< custom entries
static const char PARAMS_WIDGET_MODE[] = "widget_mode";								///<
static const char PARAMS_DMX_SEND_TO_HOST_THROTTLE[] = "dmx_send_to_host_throttle";	///<

/**
 * @ingroup widget
 *
 * @param line
 */
static int process_line_read_unsigned_int(const char *line)
{
	char name[64];
	unsigned int value;

	errno = 0;

	if (sscanf(line, "%48[^=]=%u", name, &value) == 2)
	{
		if (strncmp(name, DMXUSBPRO_PARAMS_BREAK_TIME, sizeof(DMXUSBPRO_PARAMS_BREAK_TIME)) == 0)
		{
			if((value >= (unsigned int)9) && (value <= (unsigned int)127))
			{
				dmx_usb_pro_params.break_time = (uint8_t)value;		// DMX output break time in 10.67 μs units. Valid range is 9 to 127
			}
		} else if  (strncmp(name, DMXUSBPRO_PARAMS_MAB_TIME, sizeof(DMXUSBPRO_PARAMS_MAB_TIME)) == 0)
		{
			if((value >= (unsigned int)1) && (value <= (unsigned int)127))
			{
				dmx_usb_pro_params.mab_time = (uint8_t)value;		// DMX output Mark After Break time in 10.67 μs units. Valid range is 1 to 127.
			}
		} else if  (strncmp(name, DMXUSBPRO_PARAMS_REFRESH_RATE, sizeof(DMXUSBPRO_PARAMS_REFRESH_RATE)) == 0)
		{
			dmx_usb_pro_params.refresh_rate = (uint8_t)value;		// DMX output rate in packets per second. 0 is maximum possible
		} else if  (strncmp(name, PARAMS_WIDGET_MODE, sizeof(PARAMS_WIDGET_MODE)) == 0)
		{
			if((value >= (unsigned int)MODE_DMX_RDM) && value <= (unsigned int)MODE_RDM_SNIFFER)
			{
				dmx_usb_pro_params.firmware_msb = (uint8_t)value;
			}
		} else if  (strncmp(name, PARAMS_DMX_SEND_TO_HOST_THROTTLE, sizeof(PARAMS_DMX_SEND_TO_HOST_THROTTLE)) == 0)
		{
			dmx_send_to_host_throttle = (uint8_t)value;
		} else
		{
			// nothing to do here
		}
	}

	return errno;
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
			(void) process_line_read_unsigned_int((const char *) buffer);
		}
		(void) f_close(&file_object);
	} else {
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
	dmx_set_output_break_time((uint32_t)((double)(dmx_usb_pro_params.break_time) * (double)(10.67)));
}

/**
 * @ingroup widget
 *
 * @param mab_time
 */
void widget_params_set_mab_time(const uint8_t mab_time)
{
	dmx_usb_pro_params.mab_time = mab_time;
	dmx_set_output_mab_time((uint32_t)((double)(dmx_usb_pro_params.mab_time) * (double)(10.67)));
}

/**
 * @ingroup widget
 *
 * @param refresh_rate
 */
void widget_params_set_refresh_rate(const uint8_t refresh_rate)
{
	dmx_usb_pro_params.refresh_rate = refresh_rate;
	dmx_set_output_period(refresh_rate == (uint8_t)0 ? (uint8_t)0 : (uint8_t)(1E6 / refresh_rate));
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
	return (uint8_t)DEVICE_TYPE_ID_LENGTH;
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
void widget_params_init(void) {
	uint32_t period = 0;

	read_config_file();

	if (dmx_usb_pro_params.refresh_rate != 0) {
		period = (uint32_t)(1E6 / dmx_usb_pro_params.refresh_rate);
	}

	dmx_set_output_period(period);

	period = 0;

	if (dmx_send_to_host_throttle != 0) {
		period = (uint32_t)(1E6 / dmx_send_to_host_throttle);
	}

	widget_set_received_dmx_packet_period(period);

	uint8_t mode = dmx_usb_pro_params.firmware_msb;

	if (mode == MODE_DMX_RDM) {
		dmx_usb_pro_params.firmware_msb = FIRMWARE_RDM;
	}

	widget_set_mode(mode);

	dmx_set_output_break_time((double)(dmx_usb_pro_params.break_time) * (double)(10.67));
	dmx_set_output_mab_time((double)(dmx_usb_pro_params.mab_time) * (double)(10.67));
}
