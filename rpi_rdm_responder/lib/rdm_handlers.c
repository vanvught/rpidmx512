/**
 * @file rdm_handlers.c
 *
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

#include "sys_time.h"
#include "hardware.h"
#include "util.h"
#include "dmx.h"
#include "rdm.h"
#include "rdm_send.h"
#include "rdm_sub_devices.h"
#include "rdm_e120.h"
#include "rdm_device_info.h"
#include "rdm_identify.h"

static uint8_t *rdm_handlers_rdm_data = NULL;

//static void rdm_get_queued_message(void);
static void rdm_get_supported_parameters(uint16_t);
static void rdm_get_device_info(uint16_t);
static void rdm_get_device_model_description(uint16_t);
static void rdm_get_manufacturer_label(uint16_t);
static void rdm_get_device_label(uint16_t);
static void rdm_set_device_label(uint8_t , uint16_t);
static void rdm_get_factory_defaults(uint16_t);
static void rdm_set_factory_defaults(uint8_t , uint16_t);
static void rdm_get_language(uint16_t);
static void rdm_set_language(uint8_t , uint16_t);
static void rdm_get_software_version_label(uint16_t);
static void rdm_get_boot_software_version_id(uint16_t);
static void rdm_get_boot_software_version_label(uint16_t);
static void rdm_get_personality(uint16_t);
static void rdm_set_personality(uint8_t , uint16_t);
static void rdm_get_personality_description(uint16_t);
static void rdm_get_dmx_start_address(uint16_t);
static void rdm_set_dmx_start_address(uint8_t , uint16_t);
static void rdm_get_device_hours(uint16_t);
static void rdm_set_device_hours(uint8_t , uint16_t);
static void rdm_get_identify_device(uint16_t);
static void rdm_set_identify_device(uint8_t , uint16_t);
static void rdm_get_real_time_clock(uint16_t);
static void rdm_set_real_time_clock(uint8_t , uint16_t);
static void rdm_set_reset_device(uint8_t , uint16_t);

struct _pid_definition
{
	const uint16_t pid;
	void (*get_handler)(uint16_t sub_device);
	void (*set_handler)(uint8_t was_broadcast, uint16_t sub_device);
	const uint8_t get_argument_size;
	const uint8_t include_in_supported_params;
};

static const struct _pid_definition PID_DEFINITIONS[] = {
//		{E120_QUEUED_MESSAGE,              &rdm_get_queued_message,             NULL,                        1, TRUE },
		{E120_SUPPORTED_PARAMETERS,        &rdm_get_supported_parameters,       NULL,                        0, FALSE},
		{E120_DEVICE_INFO,                 &rdm_get_device_info,                NULL,                        0, FALSE},
		{E120_DEVICE_MODEL_DESCRIPTION,    &rdm_get_device_model_description,   NULL,                        0, TRUE },
		{E120_MANUFACTURER_LABEL,          &rdm_get_manufacturer_label,         NULL,                        0, TRUE },
		{E120_DEVICE_LABEL,                &rdm_get_device_label,               &rdm_set_device_label,       0, TRUE },
		{E120_FACTORY_DEFAULTS,            &rdm_get_factory_defaults,           &rdm_set_factory_defaults,   0, TRUE },
		{E120_LANGUAGE_CAPABILITIES,       &rdm_get_language,			        NULL,                        0, TRUE },
		{E120_LANGUAGE,				       &rdm_get_language,			        &rdm_set_language,           0, TRUE },
		{E120_SOFTWARE_VERSION_LABEL,      &rdm_get_software_version_label,     NULL,                        0, FALSE},
		{E120_BOOT_SOFTWARE_VERSION_ID,    &rdm_get_boot_software_version_id,   NULL,                        0, TRUE },
		{E120_BOOT_SOFTWARE_VERSION_LABEL, &rdm_get_boot_software_version_label,NULL,                        0, TRUE },
		{E120_DMX_PERSONALITY,		       &rdm_get_personality,                &rdm_set_personality,        0, TRUE },
		{E120_DMX_PERSONALITY_DESCRIPTION, &rdm_get_personality_description,    NULL,                        1, TRUE },
		{E120_DMX_START_ADDRESS,           &rdm_get_dmx_start_address,          &rdm_set_dmx_start_address,  0, FALSE},
		{E120_DEVICE_HOURS,                &rdm_get_device_hours,    	        &rdm_set_device_hours,       0, TRUE },
		{E120_REAL_TIME_CLOCK,		       &rdm_get_real_time_clock,            &rdm_set_real_time_clock,    0, TRUE },
		{E120_IDENTIFY_DEVICE,		       &rdm_get_identify_device,		    &rdm_set_identify_device,    0, FALSE},
		{E120_RESET_DEVICE,			       NULL,                                &rdm_set_reset_device,       0, TRUE }
};

static const struct _pid_definition PID_DEFINITIONS_SUB_DEVICES[] = {
		{E120_SUPPORTED_PARAMETERS,        &rdm_get_supported_parameters,       NULL,                        0, TRUE },
		{E120_DEVICE_INFO,                 &rdm_get_device_info,                NULL,                        0, TRUE },
		{E120_SOFTWARE_VERSION_LABEL,      &rdm_get_software_version_label,     NULL,                        0, TRUE },
		{E120_DMX_PERSONALITY,		       &rdm_get_personality,                &rdm_set_personality,        0, TRUE },
		{E120_DMX_PERSONALITY_DESCRIPTION, &rdm_get_personality_description,    NULL,                        1, TRUE },
		{E120_DMX_START_ADDRESS,           &rdm_get_dmx_start_address,          &rdm_set_dmx_start_address,  0, TRUE },
		{E120_IDENTIFY_DEVICE,		       &rdm_get_identify_device,		    &rdm_set_identify_device,    0, TRUE }
};

/**
 * @ingroup rdm_handlers
 *
 * @param sub_device
 */
static void rdm_get_supported_parameters(uint16_t sub_device)
{
	uint8_t supported_params = 0;

	const struct _pid_definition *pid_definitions;
	int table_size;

	if (sub_device)
	{
		pid_definitions = &PID_DEFINITIONS_SUB_DEVICES[0];
		table_size = sizeof(PID_DEFINITIONS_SUB_DEVICES) / sizeof(struct _pid_definition);
	} else
	{
		pid_definitions = &PID_DEFINITIONS[0];
		table_size = sizeof(PID_DEFINITIONS) / sizeof(struct _pid_definition);
	}

	uint8_t i;
	for (i = 0;	i < table_size; i++)
	{
		if (pid_definitions[i].include_in_supported_params)
			supported_params++;
	}

	struct _rdm_command *rdm_response = (struct _rdm_command *)rdm_handlers_rdm_data;

	rdm_response->param_data_length = (2 * supported_params);
	rdm_response->message_length = rdm_response->message_length + (2 * supported_params);

	int j = 0;
	for (i = 0;	i < table_size; i++)
	{
		if (pid_definitions[i].include_in_supported_params)
		{
			rdm_response->param_data[j+j] = (uint8_t)(pid_definitions[i].pid >> 8);
			rdm_response->param_data[j+j+1] = (uint8_t)pid_definitions[i].pid;
			j++;
		}
	}

	rdm_send_respond_message_ack(rdm_handlers_rdm_data);
}

#if 0
static void rdm_get_queued_message(void)
{
	struct _rdm_command *rdm_response = (struct _rdm_command *) rdm_handlers_rdm_data;
	rdm_response->port_id = E120_STATUS_MESSAGES;
	rdm_response->param_data_length = 0;

	rdm_send_respond_message_ack(rdm_handlers_rdm_data);
}
#endif

/**
 * @ingroup rdm_handlers
 *
 * @param name
 * @param lenght
 */
inline static void handle_string(const char *name, const uint8_t lenght)
{
	struct _rdm_command *rdm_response = (struct _rdm_command *) rdm_handlers_rdm_data;

	rdm_response->param_data_length = lenght;
	rdm_response->message_length = RDM_MESSAGE_MINIMUM_SIZE + lenght;

	uint8_t i;
	for (i = 0; i < lenght; i++)
		rdm_response->param_data[i] = name[i];
}

/**
 * @ingroup rdm_handlers
 *
 * @param sub_device
 */
static void rdm_get_device_info(uint16_t sub_device)
{
	struct _rdm_command *rdm_response = (struct _rdm_command *)rdm_handlers_rdm_data;
	const struct _rdm_device_info *rdm_device_info = rdm_device_info_get(sub_device);
	struct _rdm_device_info *device_info = (struct _rdm_device_info *)rdm_response->param_data;

	rdm_response->param_data_length = sizeof(struct _rdm_device_info);
	rdm_response->message_length = RDM_MESSAGE_MINIMUM_SIZE + rdm_response->param_data_length;

	memcpy(device_info, rdm_device_info, sizeof(struct _rdm_device_info));

	rdm_send_respond_message_ack(rdm_handlers_rdm_data);
}

/**
 * @ingroup rdm_handlers
 *
 * @param sub_device
 */
static void rdm_get_device_model_description(uint16_t sub_device)
{
	const char *board_model = hardware_get_board_model();
	const uint8_t board_model_length = hardware_get_board_model_length();

	handle_string(board_model, board_model_length);
	rdm_send_respond_message_ack(rdm_handlers_rdm_data);
}

/**
 * @ingroup rdm_handlers
 *
 * @param sub_device
 */
static void rdm_get_manufacturer_label(uint16_t sub_device)
{
	const char *manufacturer_name = rdm_device_info_get_manufacturer_name();
	const uint8_t manufacturer_name_length = rdm_device_info_get_manufacturer_name_length();

	handle_string(manufacturer_name, manufacturer_name_length);
	rdm_send_respond_message_ack(rdm_handlers_rdm_data);
}


/**
 * @ingroup rdm_handlers
 *
 * @param sub_device
 */
static void rdm_get_device_label(uint16_t sub_device)
{
	const char *device_name = rdm_device_info_get_label(sub_device);
	const uint8_t device_name_length = rdm_device_info_get_label_length(sub_device);

	handle_string(device_name, device_name_length);
	rdm_send_respond_message_ack(rdm_handlers_rdm_data);
}

/**
 * @ingroup rdm_handlers
 *
 * @param was_broadcast
 * @param sub_device
 */
static void rdm_set_device_label(uint8_t was_broadcast, uint16_t sub_device)
{
	struct _rdm_command *rdm_command = (struct _rdm_command *)rdm_handlers_rdm_data;

	if (rdm_command->param_data_length > 32)
	{
		rdm_send_respond_message_nack(rdm_handlers_rdm_data, E120_NR_FORMAT_ERROR);
		return;
	}

	const uint8_t device_label_length = rdm_command->param_data_length;
	const uint8_t *device_label= &rdm_command->param_data[0];

	rdm_device_info_set_label(sub_device, device_label, device_label_length);

	if(was_broadcast)
		return;

	rdm_command->param_data_length = 0;
	rdm_command->message_length = RDM_MESSAGE_MINIMUM_SIZE;

	rdm_send_respond_message_ack(rdm_handlers_rdm_data);
}

/**
 * @ingroup rdm_handlers
 *
 * @param sub_device
 */
static void rdm_get_factory_defaults(uint16_t sub_device)
{
	struct _rdm_command *rdm_command = (struct _rdm_command *)rdm_handlers_rdm_data;

	if (rdm_command->param_data_length != 0)
	{
		rdm_send_respond_message_nack(rdm_handlers_rdm_data, E120_NR_FORMAT_ERROR);
		return;
	}

	rdm_command->param_data_length = 1;
	rdm_command->message_length = RDM_MESSAGE_MINIMUM_SIZE + 1;
	rdm_command->param_data[0] = rdm_device_info_get_is_factory_defaults();

	rdm_send_respond_message_ack(rdm_handlers_rdm_data);
}

/**
 * @ingroup rdm_handlers
 *
 * @param was_broadcast
 * @param sub_device
 */
static void rdm_set_factory_defaults(uint8_t was_broadcast, uint16_t sub_device)
{
	struct _rdm_command *rdm_command = (struct _rdm_command *)rdm_handlers_rdm_data;

	if (rdm_command->param_data_length != 0)
	{
		rdm_send_respond_message_nack(rdm_handlers_rdm_data, E120_NR_FORMAT_ERROR);
		return;
	}

	rdm_device_info_init();

	if(was_broadcast)
		return;

	rdm_send_respond_message_ack(rdm_handlers_rdm_data);
}

/**
 * @ingroup rdm_handlers
 *
 * @param sub_device
 */
static void rdm_get_language(uint16_t sub_device)
{
	const char *supported_language = rdm_device_info_get_supported_language();
	const uint8_t supported_language_length = rdm_device_info_get_supported_language_length();

	handle_string(supported_language, supported_language_length);
	rdm_send_respond_message_ack(rdm_handlers_rdm_data);
}

/**
 * @ingroup rdm_handlers
 *
 * @param sub_device
 */
static void rdm_get_software_version_label(uint16_t sub_device)
{
	const char *software_version = rdm_device_info_get_software_version();
	const uint8_t software_version_length = rdm_device_info_get_software_version_length();

	handle_string(software_version, software_version_length);
	rdm_send_respond_message_ack(rdm_handlers_rdm_data);
}

/**
 * @ingroup rdm_handlers
 *
 * @param sub_device
 */
static void rdm_get_boot_software_version_id(uint16_t sub_device)
{
	struct _rdm_command *rdm_command = (struct _rdm_command *)rdm_handlers_rdm_data;

	if (rdm_command->param_data_length != 0)
	{
		rdm_send_respond_message_nack(rdm_handlers_rdm_data, E120_NR_FORMAT_ERROR);
		return;
	}

	uint32_t boot_software_version_id = hardware_get_firmware_revision();

	rdm_command->param_data_length = 4;
	rdm_command->message_length = RDM_MESSAGE_MINIMUM_SIZE + 4;
	rdm_command->param_data[0] = (uint8_t)(boot_software_version_id >> 24);
	rdm_command->param_data[1] = (uint8_t)(boot_software_version_id >> 16);
	rdm_command->param_data[2] = (uint8_t)(boot_software_version_id >> 8);
	rdm_command->param_data[3] = (uint8_t)boot_software_version_id;

	rdm_send_respond_message_ack(rdm_handlers_rdm_data);
}

/**
 * @ingroup rdm_handlers
 *
 * @param sub_device
 */
static void rdm_get_boot_software_version_label(uint16_t sub_device)
{
	struct _rdm_command *rdm_command = (struct _rdm_command *)rdm_handlers_rdm_data;

	if (rdm_command->param_data_length != 0)
	{
		rdm_send_respond_message_nack(rdm_handlers_rdm_data, E120_NR_FORMAT_ERROR);
		return;
	}

	const char *firmware_copyright = hardware_get_firmware_copyright();
	const uint8_t firmware_copyright_length = hardware_get_firmware_copyright_length();

	handle_string(firmware_copyright, firmware_copyright_length);
	rdm_send_respond_message_ack(rdm_handlers_rdm_data);
}

/**
 * @ingroup rdm_handlers
 *
 * @param sub_device
 */
static void rdm_get_personality(uint16_t sub_device)
{
	struct _rdm_command *rdm_response = (struct _rdm_command *)rdm_handlers_rdm_data;
	const uint8_t current_personality = rdm_device_info_get_personality_current(sub_device);
	const uint8_t personality_count = rdm_device_info_get_personality_count(sub_device);

	rdm_response->param_data_length = 2;
	rdm_response->message_length = rdm_response->message_length + 2;
	rdm_response->param_data[0] = current_personality;
	rdm_response->param_data[1] = personality_count;

	rdm_send_respond_message_ack(rdm_handlers_rdm_data);
}

/**
 * @ingroup rdm_handlers
 *
 * @param was_broadcast
 * @param sub_device
 */
static void rdm_set_personality(uint8_t was_broadcast, uint16_t sub_device)
{
	struct _rdm_command *rdm_command = (struct _rdm_command *)rdm_handlers_rdm_data;

	if (rdm_command->param_data_length != 1)
	{
		rdm_send_respond_message_nack(rdm_handlers_rdm_data, E120_NR_FORMAT_ERROR);
		return;
	}

	const uint8_t personality = rdm_command->param_data[0];
	const uint8_t max_personalities = rdm_device_info_get_personality_count(sub_device);

	if ((personality == 0) || (personality > max_personalities))
	{
		rdm_send_respond_message_nack(rdm_handlers_rdm_data, E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	rdm_device_info_set_personality_current(sub_device, personality);

	rdm_command->param_data_length = 0;
	rdm_command->message_length = RDM_MESSAGE_MINIMUM_SIZE;

	rdm_send_respond_message_ack(rdm_handlers_rdm_data);
}

/**
 * @ingroup rdm_handlers
 *
 * @param sub_device
 */
static void rdm_get_personality_description(uint16_t sub_device)
{
	struct _rdm_command *rdm_command = (struct _rdm_command *)rdm_handlers_rdm_data;
	const uint8_t personality = rdm_command->param_data[0];
	const uint8_t max_personalities = rdm_device_info_get_personality_count(sub_device);

	if ((personality == 0) || (personality > max_personalities))
	{
		rdm_send_respond_message_nack(rdm_handlers_rdm_data, E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	const uint16_t slots = rdm_device_info_get_personality_slots(sub_device, personality);

	const char *description = rdm_device_info_get_personality_description(sub_device, personality);
	uint8_t length = strlen(description);
	length = length > 32 ? 32 : length;

	rdm_command->param_data[1] = (uint8_t)(slots >> 8);
	rdm_command->param_data[2] = (uint8_t)(slots);

	uint8_t i = 0;
	for (i = 0; i < length; i++)
	{
		rdm_command->param_data[3 + i] = description[i];
	}

	rdm_command->param_data_length = 3 + length;
	rdm_command->message_length = RDM_MESSAGE_MINIMUM_SIZE + 3 + length;

	rdm_send_respond_message_ack(rdm_handlers_rdm_data);
}

/**
 * @ingroup rdm_handlers
 *
 * @param sub_device
 */
static void rdm_get_dmx_start_address(uint16_t sub_device)
{
	struct _rdm_command *rdm_response = (struct _rdm_command *)rdm_handlers_rdm_data;
	const uint16_t dmx_start_address = rdm_device_info_get_dmx_start_address(sub_device);

	rdm_response->param_data_length = 2;
	rdm_response->message_length = RDM_MESSAGE_MINIMUM_SIZE + 2;
	rdm_response->param_data[0] = (uint8_t)(dmx_start_address >> 8);
	rdm_response->param_data[1] = (uint8_t)dmx_start_address;

	rdm_send_respond_message_ack(rdm_handlers_rdm_data);
}

/**
 * @ingroup rdm_handlers
 *
 * @param sub_device
 */
static void rdm_get_device_hours(uint16_t sub_device)
{
	struct _rdm_command *rdm_response = (struct _rdm_command *)rdm_handlers_rdm_data;
	const uint64_t device_hours = hardware_uptime_seconds() / 3600;

	rdm_response->param_data_length = 4;
	rdm_response->message_length = RDM_MESSAGE_MINIMUM_SIZE + 4;
	rdm_response->param_data[0] = (uint8_t)(device_hours >> 24);
	rdm_response->param_data[1] = (uint8_t)(device_hours >> 16);
	rdm_response->param_data[2] = (uint8_t)(device_hours >> 8);
	rdm_response->param_data[3] = (uint8_t)device_hours;

	rdm_send_respond_message_ack(rdm_handlers_rdm_data);
}

/**
 * @ingroup rdm_handlers
 *
 * @param was_broadcast
 * @param sub_device
 */
static void rdm_set_device_hours(uint8_t was_broadcast, uint16_t sub_device)
{
	struct _rdm_command *rdm_command = (struct _rdm_command *)rdm_handlers_rdm_data;

	rdm_command->param_data_length = 0;
	rdm_command->message_length = RDM_MESSAGE_MINIMUM_SIZE;

	rdm_send_respond_message_nack(rdm_handlers_rdm_data, E120_NR_WRITE_PROTECT);
}

/**
 * @ingroup rdm_handlers
 *
 * @param sub_device
 */
static void rdm_get_identify_device(uint16_t sub_device)
{
	struct _rdm_command *rdm_response = (struct _rdm_command *)rdm_handlers_rdm_data;

	rdm_response->param_data_length = 1;
	rdm_response->message_length = RDM_MESSAGE_MINIMUM_SIZE + 1;
	rdm_response->param_data[0] = rdm_identify_is_enabled();

	rdm_send_respond_message_ack(rdm_handlers_rdm_data);
}

/**
 * @ingroup rdm_handlers
 *
 * @param sub_device
 */
static void rdm_get_real_time_clock(uint16_t sub_device)
{
	time_t ltime = 0;
	struct tm *local_time = NULL;

	ltime = sys_time(NULL);
    local_time = localtime(&ltime);

    struct _rdm_command *rdm_response = (struct _rdm_command *)rdm_handlers_rdm_data;

	rdm_response->param_data_length = 7;
	rdm_response->message_length = RDM_MESSAGE_MINIMUM_SIZE + 7;

	const uint16_t year = local_time->tm_year + 2000;

	rdm_response->param_data[0] = (uint8_t)(year >> 8);
	rdm_response->param_data[1] = (uint8_t)(year);
	rdm_response->param_data[2] = (uint8_t)(local_time->tm_mon);
	rdm_response->param_data[3] = (uint8_t)(local_time->tm_mday);
	rdm_response->param_data[4] = (uint8_t)(local_time->tm_hour);
	rdm_response->param_data[5] = (uint8_t)(local_time->tm_min);
	rdm_response->param_data[6] = (uint8_t)(local_time->tm_sec);

	rdm_send_respond_message_ack(rdm_handlers_rdm_data);
}

/**
 * @ingroup rdm_handlers
 *
 * @param was_broadcast
 * @param sub_device
 */
static void rdm_set_real_time_clock(uint8_t was_broadcast, uint16_t sub_device)
{
	struct _rdm_command *rdm_command = (struct _rdm_command *)rdm_handlers_rdm_data;

	if (rdm_command->param_data_length != 7)
	{
		rdm_send_respond_message_nack(rdm_handlers_rdm_data, E120_NR_FORMAT_ERROR);
		return;
	}

	struct hardware_time tm_hw;

	tm_hw.year = (rdm_command->param_data[0] << 8) + rdm_command->param_data[1];
	tm_hw.month = rdm_command->param_data[2];
	tm_hw.day = rdm_command->param_data[3];
	tm_hw.hour = rdm_command->param_data[4];
	tm_hw.minute = rdm_command->param_data[5];
	tm_hw.second = rdm_command->param_data[6];

	hardware_rtc_set(&tm_hw);

	if(was_broadcast)
		return;

	rdm_command->param_data_length = 0;
	rdm_send_respond_message_ack(rdm_handlers_rdm_data);
}

/**
 * @ingroup rdm_handlers
 *
 * @param was_broadcast
 * @param sub_device
 */
static void rdm_set_identify_device(uint8_t was_broadcast, uint16_t sub_device)
{
	struct _rdm_command *rdm_command = (struct _rdm_command *)rdm_handlers_rdm_data;

	if (rdm_command->param_data_length != 1)
	{
		rdm_send_respond_message_nack(rdm_handlers_rdm_data, E120_NR_FORMAT_ERROR);
		return;
	}

	if ((rdm_command->param_data[0] != 0) && (rdm_command->param_data[0] != 1))
	{
		rdm_send_respond_message_nack(rdm_handlers_rdm_data, E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	if (rdm_command->param_data[0] == 0)
		rdm_identify_off();
	else
		rdm_identify_on();

	if(was_broadcast)
		return;

	rdm_command->param_data_length = 0;
	rdm_send_respond_message_ack(rdm_handlers_rdm_data);
}

/**
 * @ingroup rdm_handlers
 *
 * @param was_broadcast
 * @param sub_device
 */
static void rdm_set_reset_device(uint8_t was_broadcast, uint16_t sub_device)
{
	struct _rdm_command *rdm_response = (struct _rdm_command *)rdm_handlers_rdm_data;
	rdm_response->param_data_length = 0;

	if(was_broadcast == FALSE)
		rdm_send_respond_message_ack(rdm_handlers_rdm_data);

	hardware_reboot();
}

/**
 * @ingroup rdm_handlers
 *
 * @param was_broadcast
 * @param sub_device
 */
static void rdm_set_dmx_start_address(uint8_t was_broadcast, uint16_t sub_device)
{
	struct _rdm_command *rdm_command = (struct _rdm_command *) rdm_handlers_rdm_data;

	if (rdm_command->param_data_length != 2)
	{
		rdm_send_respond_message_nack(rdm_handlers_rdm_data, E120_NR_FORMAT_ERROR);
		return;
	}

	const uint16_t dmx_start_address = (rdm_command->param_data[0] << 8) + rdm_command->param_data[1];

	if ((dmx_start_address == 0) || (dmx_start_address > DMX_UNIVERSE_SIZE))
	{
		rdm_send_respond_message_nack(rdm_handlers_rdm_data, E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	rdm_device_info_set_dmx_start_address(sub_device, dmx_start_address);

	if(was_broadcast)
		return;

	rdm_command->param_data_length = 0;
	rdm_send_respond_message_ack(rdm_handlers_rdm_data);
}

/**
 * @ingroup rdm_handlers
 *
 * @param was_broadcast
 * @param sub_device
 */
static void rdm_set_language(uint8_t was_broadcast, uint16_t sub_device)
{
	struct _rdm_command *rdm_command = (struct _rdm_command *) rdm_handlers_rdm_data;

	if (rdm_command->param_data_length != 2)
	{
		rdm_send_respond_message_nack(rdm_handlers_rdm_data, E120_NR_FORMAT_ERROR);
		return;
	}

	const char *supported_language = rdm_device_info_get_supported_language();

	if ((rdm_command->param_data[0] != supported_language[0]) || (rdm_command->param_data[1] != supported_language[1]))
	{
		rdm_send_respond_message_nack(rdm_handlers_rdm_data, E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	if(was_broadcast)
		return;

	rdm_command->param_data_length = 0;
	rdm_send_respond_message_ack(rdm_handlers_rdm_data);
}

/**
 * @ingroup rdm_handlers
 *
 * @param is_broadcast
 * @param command_class
 * @param param_id
 * @param param_data_length
 * @param sub_device
 */
void rdm_handlers(uint8_t *rdm_data, const uint8_t is_broadcast, const uint8_t command_class, const uint16_t param_id, const uint8_t param_data_length, const uint16_t sub_device)
{
	struct _pid_definition const *pid_handler = NULL;

	rdm_handlers_rdm_data = rdm_data;

	if (command_class != E120_GET_COMMAND && command_class != E120_SET_COMMAND)
	{
		rdm_send_respond_message_nack(rdm_handlers_rdm_data, E120_NR_UNSUPPORTED_COMMAND_CLASS);
		return;
	}

	const uint16_t sub_device_count = rdm_sub_devices_get();

	if ((sub_device >  sub_device_count) && (sub_device != 0xffff))
	{
		rdm_send_respond_message_nack(rdm_handlers_rdm_data, E120_NR_SUB_DEVICE_OUT_OF_RANGE);
		return;
	}

	uint8_t i;
	for (i = 0; i < sizeof(PID_DEFINITIONS) / sizeof(struct _pid_definition); ++i)
	{
		if (PID_DEFINITIONS[i].pid == param_id)
			pid_handler = &PID_DEFINITIONS[i];
	}

	if (!pid_handler)
	{
		rdm_send_respond_message_nack(rdm_handlers_rdm_data, E120_NR_UNKNOWN_PID);
		return;
	}

	if (command_class == E120_GET_COMMAND)
	{
		if(is_broadcast)
		{
			return;
		}

		if (sub_device == 0xffff)
		{
			rdm_send_respond_message_nack(rdm_handlers_rdm_data, E120_NR_SUB_DEVICE_OUT_OF_RANGE);
			return;
		}

		if (!pid_handler->get_handler)
		{
			rdm_send_respond_message_nack(rdm_handlers_rdm_data, E120_NR_UNSUPPORTED_COMMAND_CLASS);
			return;
		}

		if (param_data_length != pid_handler->get_argument_size)
		{
			rdm_send_respond_message_nack(rdm_handlers_rdm_data, E120_NR_FORMAT_ERROR);
			return;
		}

		pid_handler->get_handler(sub_device);
	}
	else
	{
		if (!pid_handler->set_handler)
		{
			rdm_send_respond_message_nack(rdm_handlers_rdm_data, E120_NR_UNSUPPORTED_COMMAND_CLASS);
			return;
		}

		pid_handler->set_handler(is_broadcast, sub_device);
	}
}
