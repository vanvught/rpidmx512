
#ifndef ARTNETDISCOVERY_H_
#define ARTNETDISCOVERY_H_

#include <stdint.h>

#include "artnetrdm.h"

#include "rdmdiscovery.h"
#include "rdmdevicecontroller.h"

#include "dmx_uarts.h"
#include "rdm.h"

class ArtNetRdmController: public RDMDeviceController, public ArtNetRdm {
public:
	ArtNetRdmController();
	~ArtNetRdmController() override;

	void Print();

	void Full(uint8_t nPort = 0) override;
	uint8_t GetUidCount(uint8_t nPort = 0) override;
	void Copy(uint8_t nPort, uint8_t *pTod) override;
	const uint8_t *Handler(uint8_t nPort, const uint8_t *pRdmData) override;

	void DumpTod(uint8_t nPort = 0);

private:
	RDMDiscovery *m_Discovery[DMX_MAX_UARTS];
	struct TRdmMessage *m_pRdmCommand{nullptr};
};

#endif /* ARTNETDISCOVERY_H_ */
