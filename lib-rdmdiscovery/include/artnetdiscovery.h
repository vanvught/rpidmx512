
#ifndef ARTNETDISCOVERY_H_
#define ARTNETDISCOVERY_H_

#include <stdint.h>

#include "artnetrdm.h"

#include "rdmdiscovery.h"
#include "rdmdevicecontroller.h"

#include "dmx.h"
#include "rdm.h"

class ArtNetRdmController: public ArtNetRdm {
public:
	ArtNetRdmController(void);
	~ArtNetRdmController(void);

	void Full(uint8_t nPort = 0);
	const uint8_t GetUidCount(uint8_t nPort = 0);
	void Copy(uint8_t nPort, uint8_t *);
	const uint8_t *Handler(uint8_t nPort, const uint8_t *);

	void DumpTod(uint8_t nPort = 0);

private:
	RDMDiscovery *m_Discovery[DMX_MAX_UARTS];
	RDMDeviceController m_Controller;
	struct TRdmMessage *m_pRdmCommand;
};

#endif /* ARTNETDISCOVERY_H_ */
