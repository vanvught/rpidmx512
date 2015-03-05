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
#include <stdio.h>

#include "sys_time.h"
#include "console.h"
#include "hardware.h"

#include "dmx.h"
#include "rdm.h"
#include "rdm_send.h"
#include "rdm_e120.h"

#include "device_info.h"

typedef enum {
	FALSE = 0,
	TRUE = 1
} _boolean;

static const uint8_t SUPPORTED_LANGUAGE[] = "en";
static const uint8_t SUPPORTED_LANGUAGE_LENGTH = (sizeof(SUPPORTED_LANGUAGE) / sizeof(uint8_t)) - 1;
static const uint8_t DEVICE_MANUFACTURER_NAME[] = "AvV";
static const uint8_t DEVICE_MANUFACTURER_NAME_LENGTH = (sizeof(DEVICE_MANUFACTURER_NAME) / sizeof(uint8_t)) - 1;
static const uint8_t SOFTWARE_VERSION[] = "1.0";
static const uint8_t SOFTWARE_VERSION_LENGTH = (sizeof(SOFTWARE_VERSION) / sizeof(uint8_t)) - 1;
static const uint8_t DEVICE_NAME[] = "Raspberry Pi";
static const uint8_t DEVICE_NAME_LENGTH = (sizeof(DEVICE_NAME) / sizeof(uint8_t)) - 1;

extern uint8_t rdm_data[512];
static uint8_t rdm_is_mute = FALSE;
static uint8_t rdm_identify_mode_enabled = FALSE;

// Unique identifier (UID) which consists of a 2 byte ESTA manufacturer ID, and a 4 byte device ID.
static const uint8_t UID_ALL[UID_SIZE] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

static void rdm_get_supported_parameters(void);
static void rdm_get_device_info(void);
static void rdm_get_manufacturer_label(void);
static void rdm_get_device_label(void);
static void rdm_get_language(void);
static void rdm_get_software_version_label(void);
static void rdm_get_personality(void);
static void rdm_get_personality_description(void);
static void rdm_get_dmx_start_address(void);
static void rdm_get_identify_device(void);
static void rdm_get_real_time_clock(void);

static void rdm_set_identify_device(uint8_t was_broadcast, uint16_t sub_device);
static void rdm_set_reset_device(uint8_t was_broadcast, uint16_t sub_device);
static void rdm_set_dmx_start_address(uint8_t was_broadcast, uint16_t sub_device);
static void rdm_set_language(uint8_t was_broadcast, uint16_t sub_device);
static void rdm_set_personality(uint8_t was_broadcast, uint16_t sub_device);

uint8_t rdm_is_mute_get(void)
{
	return rdm_is_mute;
}

struct _pid_definition
{
	uint16_t pid;
	void (*get_handler)(void);
	void (*set_handler)(uint8_t was_broadcast, uint16_t sub_device);
	uint8_t get_argument_size;
	uint8_t include_in_supported_params;
} const PID_DEFINITIONS[] = {
		{E120_SUPPORTED_PARAMETERS,        &rdm_get_supported_parameters,    NULL,                       0, FALSE},
		{E120_DEVICE_INFO,                 &rdm_get_device_info,             NULL,                       0, FALSE},
		{E120_MANUFACTURER_LABEL,          &rdm_get_manufacturer_label,      NULL,                       0, TRUE },
		{E120_DEVICE_LABEL,                &rdm_get_device_label,            NULL,                       0, TRUE },
		{E120_LANGUAGE_CAPABILITIES,       &rdm_get_language,			     NULL,                       0, TRUE },
		{E120_LANGUAGE,				       &rdm_get_language,			     &rdm_set_language,          0, TRUE },
		{E120_SOFTWARE_VERSION_LABEL,      &rdm_get_software_version_label,  NULL,                       0, FALSE},
		{E120_DMX_PERSONALITY,		       &rdm_get_personality,             &rdm_set_personality,       0, TRUE },
		{E120_DMX_PERSONALITY_DESCRIPTION, &rdm_get_personality_description, NULL,                       1, TRUE },
		{E120_DMX_START_ADDRESS,           &rdm_get_dmx_start_address,       &rdm_set_dmx_start_address, 0, FALSE},
		{E120_REAL_TIME_CLOCK,		       &rdm_get_real_time_clock,         NULL,                       0, TRUE },
		{E120_IDENTIFY_DEVICE,		       &rdm_get_identify_device,		 &rdm_set_identify_device,   0, FALSE},
		{E120_RESET_DEVICE,			       NULL,                             &rdm_set_reset_device,      1, TRUE },
		};

static void rdm_get_supported_parameters(void)
{
	uint8_t supported_params = 0;

	uint8_t i;
	for (i = 0;	i < sizeof(PID_DEFINITIONS) / sizeof(struct _pid_definition); i++)
	{
		if (PID_DEFINITIONS[i].include_in_supported_params)
			supported_params++;
	}

	struct _rdm_command *rdm_response = (struct _rdm_command *)rdm_data;

	rdm_response->param_data_length = (2 * supported_params);
	rdm_response->message_length = rdm_response->message_length + (2 * supported_params);

	int j = 0;
	for (i = 0;	i < sizeof(PID_DEFINITIONS) / sizeof(struct _pid_definition); i++)
	{
		if (PID_DEFINITIONS[i].include_in_supported_params)
		{
			rdm_response->param_data[j+j] = (uint8_t)(PID_DEFINITIONS[i].pid >> 8);
			rdm_response->param_data[j+j+1] = (uint8_t)PID_DEFINITIONS[i].pid;
			j++;
		}
	}

	rdm_send_respond_message_ack();

}

static void rdm_get_device_info(void)
{
	struct _rdm_command *rdm_response = (struct _rdm_command *)rdm_data;

	struct _rdm_device_info *device_info = (struct _rdm_device_info *)rdm_response->param_data;

	rdm_response->param_data_length = sizeof(struct _rdm_device_info);
	rdm_response->message_length = rdm_response->message_length + rdm_response->param_data_length;

	uint16_t dmx_footprint = device_info_dmx_footprint_get();
	uint16_t dmx_start_address = device_info_dmx_start_address_get();
	uint8_t current_personality = device_info_current_personality_get();
	uint8_t personality_count = device_info_personality_count_get();

	device_info->protocol_major = E120_PROTOCOL_VERSION >> 8;
	device_info->protocol_minor = (uint8_t)E120_PROTOCOL_VERSION;
	device_info->device_model[0] = 1 >> 8;
	device_info->device_model[1] = (uint8_t)1;
	device_info->product_category[0] = E120_PRODUCT_DETAIL_OTHER >> 8;
	device_info->product_category[1] = (uint8_t)E120_PRODUCT_DETAIL_OTHER;
	device_info->dmx_footprint[0] = (uint8_t)(dmx_footprint >> 8);
	device_info->dmx_footprint[1] = (uint8_t)dmx_footprint;
	device_info->current_personality = current_personality;
	device_info->personality_count = personality_count;
	device_info->dmx_start_address[0] = (uint8_t)(dmx_start_address >> 8);
	device_info->dmx_start_address[1] = (uint8_t)dmx_start_address;
	device_info->sub_device_count[0] = 0;
	device_info->sub_device_count[1] = 0;
	device_info->sensor_count = 0;

	rdm_send_respond_message_ack();
}

inline static void handle_string(const uint8_t *name, const uint8_t lenght)
{
	struct _rdm_command *rdm_response = (struct _rdm_command *) rdm_data;

	rdm_response->param_data_length = lenght;
	rdm_response->message_length = RDM_MESSAGE_MINIMUM_SIZE + lenght;

	uint8_t i;
	for (i = 0; i < lenght; i++)
		rdm_response->param_data[i] = name[i];
}

static void rdm_get_manufacturer_label(void)
{
	handle_string(DEVICE_MANUFACTURER_NAME, DEVICE_MANUFACTURER_NAME_LENGTH);
	rdm_send_respond_message_ack();
}


static void rdm_get_device_label(void)
{
	handle_string(DEVICE_NAME, DEVICE_NAME_LENGTH);
	rdm_send_respond_message_ack();
}

static void rdm_get_language(void)
{
	handle_string(SUPPORTED_LANGUAGE, SUPPORTED_LANGUAGE_LENGTH);
	rdm_send_respond_message_ack();
}

static void rdm_get_software_version_label(void)
{
	handle_string(SOFTWARE_VERSION, SOFTWARE_VERSION_LENGTH);
	rdm_send_respond_message_ack();
}

static void rdm_get_personality(void)
{
	struct _rdm_command *rdm_response = (struct _rdm_command *)rdm_data;
	uint8_t current_personality = device_info_current_personality_get();
	uint8_t personality_count = device_info_personality_count_get();
	rdm_response->param_data_length = 2;
	rdm_response->message_length = rdm_response->message_length + 2;
	rdm_response->param_data[0] = current_personality;
	rdm_response->param_data[1] = personality_count;

	rdm_send_respond_message_ack();
}

static void rdm_set_personality(uint8_t was_broadcast, uint16_t sub_device)
{
	struct _rdm_command *rdm_command = (struct _rdm_command *)rdm_data;

	if (rdm_command->param_data_length != 1)
	{
		rdm_send_respond_message_nack(E120_NR_FORMAT_ERROR);
		return;
	}

	uint8_t personality = rdm_command->param_data[0];
	uint8_t max_personalities = device_info_personality_count_get();

	if ((personality == 0) || (personality > max_personalities))
	{
		rdm_send_respond_message_nack(E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	device_info_current_personality_set(personality);

	rdm_command->param_data_length = 0;
	rdm_command->message_length = RDM_MESSAGE_MINIMUM_SIZE;

	rdm_send_respond_message_ack();
}

static void rdm_get_personality_description(void)
{
	struct _rdm_command *rdm_command = (struct _rdm_command *)rdm_data;

	uint8_t personality = rdm_command->param_data[0];
	uint8_t max_personalities = device_info_personality_count_get();

	if ((personality == 0) || (personality > max_personalities))
	{
		rdm_send_respond_message_nack(E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	uint16_t slots = device_info_personality_slots_get(personality);

	console_clear_line(24);
	printf("%d : slots %d\n", personality, slots);

	const char *description = device_info_personality_description_get(personality);
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

	rdm_send_respond_message_ack();
}

static void rdm_get_dmx_start_address(void)
{
	struct _rdm_command *rdm_response = (struct _rdm_command *)rdm_data;
	uint16_t dmx_start_address = device_info_dmx_start_address_get();
	rdm_response->param_data_length = 2;
	rdm_response->message_length = rdm_response->message_length + 2;
	rdm_response->param_data[0] = (uint8_t)(dmx_start_address >> 8);
	rdm_response->param_data[1] = (uint8_t)dmx_start_address;

	rdm_send_respond_message_ack();
}

static void rdm_get_identify_device(void)
{
	struct _rdm_command *rdm_response = (struct _rdm_command *)rdm_data;
	rdm_response->param_data_length = 1;
	rdm_response->message_length = rdm_response->message_length + 1;
	rdm_response->param_data[0] = rdm_identify_mode_enabled;

	rdm_send_respond_message_ack();
}

static void rdm_get_real_time_clock(void)
{
	time_t ltime = 0;
	struct tm *local_time = NULL;

	ltime = sys_time(NULL);
    local_time = localtime(&ltime);

	struct _rdm_command *rdm_response = (struct _rdm_command *)rdm_data;

	rdm_response->param_data_length = 7;
	rdm_response->message_length = rdm_response->message_length + 7;

	uint16_t year = local_time->tm_year + 2000;

	rdm_response->param_data[0] = (uint8_t)(year >> 8);
	rdm_response->param_data[1] = (uint8_t)(year);
	rdm_response->param_data[2] = (uint8_t)(local_time->tm_mon);
	rdm_response->param_data[3] = (uint8_t)(local_time->tm_mday);
	rdm_response->param_data[4] = (uint8_t)(local_time->tm_hour);
	rdm_response->param_data[5] = (uint8_t)(local_time->tm_min);
	rdm_response->param_data[6] = (uint8_t)(local_time->tm_sec);

	rdm_send_respond_message_ack();
}

static void rdm_set_identify_device(uint8_t was_broadcast, uint16_t sub_device)
{
	struct _rdm_command *rdm_command = (struct _rdm_command *)rdm_data;

	if (rdm_command->param_data_length != 1)
	{
		rdm_send_respond_message_nack(E120_NR_FORMAT_ERROR);
		return;
	}

	if ((rdm_command->param_data[0] != 0) && (rdm_command->param_data[0] != 1))
	{
		rdm_send_respond_message_nack(E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	rdm_identify_mode_enabled = rdm_command->param_data[0];

	//TODO Do something here Flash led?

	if(was_broadcast)
		return;

	rdm_command->param_data_length = 0;
	rdm_send_respond_message_ack();
}


static void rdm_set_reset_device(uint8_t was_broadcast, uint16_t sub_device)
{
	struct _rdm_command *rdm_response = (struct _rdm_command *)rdm_data;
	rdm_response->param_data_length = 0;

	if(was_broadcast == FALSE)
		rdm_send_respond_message_ack();

	hardware_reboot();
}

static void rdm_set_dmx_start_address(uint8_t was_broadcast, uint16_t sub_device)
{
	struct _rdm_command *rdm_command = (struct _rdm_command *) rdm_data;

	if (rdm_command->param_data_length != 2)
	{
		rdm_send_respond_message_nack(E120_NR_FORMAT_ERROR);
		return;
	}

	uint16_t dmx_start_address = (rdm_command->param_data[0] << 8) + rdm_command->param_data[1];

	if ((dmx_start_address == 0) || (dmx_start_address > 512))
	{
		rdm_send_respond_message_nack(E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	device_info_dmx_start_address_set(dmx_start_address);

	if(was_broadcast)
		return;

	rdm_command->param_data_length = 0;
	rdm_send_respond_message_ack();
}

static void rdm_set_language(uint8_t was_broadcast, uint16_t sub_device)
{
	struct _rdm_command *rdm_command = (struct _rdm_command *) rdm_data;

	if (rdm_command->param_data_length != 2)
	{
		rdm_send_respond_message_nack(E120_NR_FORMAT_ERROR);
		return;
	}

	if ((rdm_command->param_data[0] != SUPPORTED_LANGUAGE[0]) || (rdm_command->param_data[1] != SUPPORTED_LANGUAGE[1]))
	{
		rdm_send_respond_message_nack(E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	if(was_broadcast)
		return;

	rdm_command->param_data_length = 0;
	rdm_send_respond_message_ack();
}

static void process_rdm_message(const uint8_t is_broadcast, const uint8_t command_class, const uint16_t param_id, const uint8_t param_data_length, const uint16_t sub_device)
{
	struct _pid_definition const *pid_handler = NULL;

	if (command_class != E120_GET_COMMAND && command_class != E120_SET_COMMAND)
	{
		rdm_send_respond_message_nack(E120_NR_UNSUPPORTED_COMMAND_CLASS);
		return;
	}

	if (sub_device != 0 && sub_device != 0xffff)
	{
		rdm_send_respond_message_nack(E120_NR_SUB_DEVICE_OUT_OF_RANGE);
		return;
	}

	uint8_t i;
	for (i = 0; i < sizeof(PID_DEFINITIONS) / sizeof(struct _pid_definition);
			++i)
	{
		if (PID_DEFINITIONS[i].pid == param_id)
			pid_handler = &PID_DEFINITIONS[i];
	}

	if (!pid_handler)
	{
		rdm_send_respond_message_nack(E120_NR_UNKNOWN_PID);
		return;
	}

	if (command_class == E120_GET_COMMAND)
	{
		if (!pid_handler->get_handler)
		{
			rdm_send_respond_message_nack(E120_NR_UNSUPPORTED_COMMAND_CLASS);
			return;
		}

		if(is_broadcast)
		{
			return;
		}

		if (sub_device)
		{
			rdm_send_respond_message_nack(E120_NR_SUB_DEVICE_OUT_OF_RANGE);
			return;
		}

		if (param_data_length != pid_handler->get_argument_size)
		{
			rdm_send_respond_message_nack(E120_NR_FORMAT_ERROR);
			return;
		}

		pid_handler->get_handler();
	} else {
		if (!pid_handler->set_handler)
		{
			rdm_send_respond_message_nack(E120_NR_UNSUPPORTED_COMMAND_CLASS);
			return;
		}

		pid_handler->set_handler(is_broadcast, sub_device);
	}
}

/**
 *
 */
void rdm_handle_data(void)
{
	if(rdm_available_get() == FALSE)
		return;

	rdm_available_set(FALSE);

	uint8_t rdm_packet_is_for_me = FALSE;
	uint8_t rdm_packet_is_broadcast = FALSE;
	uint8_t rdm_packet_is_vendorcast = FALSE;

	struct _rdm_command *rdm_cmd = (struct _rdm_command *)rdm_data;

	uint8_t command_class = rdm_cmd->command_class;
	uint16_t param_id = (rdm_cmd->param_id[0] << 8) + rdm_cmd->param_id[1];

	console_clear_line(23);
	printf("handle_rdm_data, param_id [%.2x%.2x]:%d\n", rdm_cmd->param_id[0], rdm_cmd->param_id[1], param_id);

	const uint8_t *uid_device = device_info_uuid_get();

	if (memcmp(rdm_cmd->destination_uid, UID_ALL, UID_SIZE) == 0)
	{
		rdm_packet_is_broadcast = TRUE;
	}

	if ((memcmp(rdm_cmd->destination_uid, uid_device, 2) == 0) && (memcmp(&rdm_cmd->destination_uid[2], UID_ALL, UID_SIZE-2) == 0))
	{
		rdm_packet_is_vendorcast = TRUE;
		rdm_packet_is_for_me = TRUE;
	}
	else if (memcmp(rdm_cmd->destination_uid, uid_device, UID_SIZE) == 0)
	{
		rdm_packet_is_for_me = TRUE;
	}

	if ((rdm_packet_is_for_me == FALSE) && (rdm_packet_is_broadcast == FALSE))
	{
		// Ignore RDM packet
	}
	else if (command_class == E120_DISCOVERY_COMMAND)
	{
		if (param_id == E120_DISC_UNIQUE_BRANCH)
		{
			if (rdm_is_mute == FALSE)
			{
				if ((memcmp(rdm_cmd->param_data, uid_device, UID_SIZE) <= 0)
						&& (memcmp(uid_device, rdm_cmd->param_data + 6,	UID_SIZE) <= 0))
				{
					console_clear_line(24);
					printf("E120_DISC_UNIQUE_BRANCH\n");

					struct _rdm_discovery_msg *p = (struct _rdm_discovery_msg *) (rdm_data);
					uint16_t rdm_checksum = 6 * 0xFF;

					uint8_t i = 0;
					for (i = 0; i < 7; i++)
					{
						p->header_FE[i] = 0xFE;
					}
					p->header_AA = 0xAA;

					for (i = 0; i < 6; i++)
					{
						p->masked_device_id[i + i] = uid_device[i] | 0xAA;
						p->masked_device_id[i + i + 1] = uid_device[i] | 0x55;
						rdm_checksum += uid_device[i];
					}

					p->checksum[0] = (rdm_checksum >> 8) | 0xAA;
					p->checksum[1] = (rdm_checksum >> 8) | 0x55;
					p->checksum[2] = (rdm_checksum & 0xFF) | 0xAA;
					p->checksum[3] = (rdm_checksum & 0xFF) | 0x55;

					rdm_send_discovery_msg(rdm_data, sizeof(struct _rdm_discovery_msg));
				}
			}
		} else if (param_id == E120_DISC_UN_MUTE)
		{
			if (rdm_cmd->param_data_length != 0) {
				rdm_send_respond_message_nack(E120_NR_FORMAT_ERROR);
				return;
			}
			rdm_is_mute = FALSE;
			rdm_cmd->message_length = RDM_MESSAGE_MINIMUM_SIZE + 2;
			rdm_cmd->param_data_length = 2;
			rdm_cmd->param_data[0] = 0x00;	// Control Field
			rdm_cmd->param_data[1] = 0x00;	// Control Field
			rdm_send_respond_message_ack();
		} else if (param_id == E120_DISC_MUTE)
		{
			if (rdm_cmd->param_data_length != 0) {
				rdm_send_respond_message_nack(E120_NR_FORMAT_ERROR);
				return;
			}
			rdm_is_mute = TRUE;
			rdm_cmd->message_length = RDM_MESSAGE_MINIMUM_SIZE + 2;
			rdm_cmd->param_data_length = 2;
			rdm_cmd->param_data[0] = 0x00;	// Control Field
			rdm_cmd->param_data[1] = 0x00;	// Control Field
			rdm_send_respond_message_ack();
		}
	}
	else
	{
		uint16_t sub_device = (rdm_cmd->sub_device[0] << 8) + rdm_cmd->sub_device[1];
		process_rdm_message(rdm_packet_is_broadcast || rdm_packet_is_vendorcast, command_class, param_id, rdm_cmd->param_data_length, sub_device);
	}
}
