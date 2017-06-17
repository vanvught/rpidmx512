
//#define DEBUG

#include <stdint.h>

#include "rdm_e120.h"

#include "artnetnode.h"
#include "artnetdiscovery.h"

#include "rdmmessage.h"
#include "rdmdiscovery.h"
#include "rdmdevicecontroller.h"

#include "util.h"

/**
 *
 */
ArtNetDiscovery::ArtNetDiscovery(void) : m_pRdmCommand(0){
	m_Controller.ReadConfigFile();
	m_Discovery.SetUid(m_Controller.GetUID());

	m_pRdmCommand = new struct _rdm_command;

	if (m_pRdmCommand != 0) {
		m_pRdmCommand->start_code = E120_SC_RDM;
	}
}

/**
 *
 */
ArtNetDiscovery::~ArtNetDiscovery(void) {
	m_Discovery.Reset();
}

/**
 *
 */
void ArtNetDiscovery::Full(void) {
	m_Discovery.Full();
}

/**
 *
 * @return
 */
const uint8_t ArtNetDiscovery::GetUidCount(void) {
	return m_Discovery.GetUidCount();
}

/**
 *
 * @param tod
 */
void ArtNetDiscovery::Copy(unsigned char *tod) {
	m_Discovery.Copy(tod);
}

void ArtNetDiscovery::DumpTod(void) {
	m_Discovery.Dump();
}

const uint8_t *ArtNetDiscovery::Handler(const uint8_t *rdm_data) {

	if (rdm_data == 0) {
		return 0;
	}

	if (m_pRdmCommand != 0) {

		while (0 != RDMMessage::Receive()) {
			// Discard late responses
		}

		TRdmMessage *p = (TRdmMessage *) (rdm_data);
		uint8_t *c = (uint8_t *) m_pRdmCommand;

		memcpy(&c[1], rdm_data, p->message_length + 2);
#ifdef DEBUG
		RDMMessage::Print((const uint8_t *) c);
#endif
		RDMMessage::SendRaw(c, p->message_length + 2);

		return RDMMessage::ReceiveTimeOut(20000);
	}

	return 0;
}
