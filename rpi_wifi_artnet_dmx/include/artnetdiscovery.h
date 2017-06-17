
#ifndef ARTNETDISCOVERY_H_
#define ARTNETDISCOVERY_H_

#include <artnetrdm.h>
#include <stdint.h>

#include "rdmdiscovery.h"
#include "rdmdevicecontroller.h"

#include "rdm.h"

#if  ! defined (PACKED)
#define PACKED __attribute__((packed))
#endif

struct TRdmMessage {
	uint8_t sub_start_code;					///< 2	SC_SUB_MESSAGE
	uint8_t message_length;					///< 3	Range 24 to 255
	uint8_t destination_uid[RDM_UID_SIZE];	///< 4,5,6,7,8,9
	uint8_t source_uid[RDM_UID_SIZE];		///< 10,11,12,13,14,15
	uint8_t transaction_number;				///< 16
	union {
		uint8_t port_id;					///< 17
		uint8_t response_type;				///< 17
	} slot16;
	uint8_t message_count;					///< 18
	uint8_t sub_device[2];					///< 19, 20
	uint8_t command_class;					///< 21
	uint8_t param_id[2];					///< 22, 23
	uint8_t param_data_length;				///< 24	PDL	Range 0 to 231
	uint8_t param_data[231];				///< 25,,,,	PD	6.2.3 Message Length
}PACKED;

class ArtNetDiscovery: public ArtNetRdm {
public:
	ArtNetDiscovery(void);
	~ArtNetDiscovery(void);

	void Full(void);
	const uint8_t GetUidCount(void);
	void Copy(uint8_t *);
	const uint8_t *Handler(const uint8_t *);

	void DumpTod(void);

private:
	RDMDiscovery m_Discovery;
	RDMDeviceController m_Controller;
	struct _rdm_command *m_pRdmCommand;
};

#endif /* ARTNETDISCOVERY_H_ */
