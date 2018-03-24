
#ifndef ARTNETDISCOVERY_H_
#define ARTNETDISCOVERY_H_

#include <stdint.h>

#include "artnetrdm.h"

#include "rdmdiscovery.h"
#include "rdmdevicecontroller.h"

#include "rdm.h"

class ArtNetRdmResponder: public ArtNetRdm {
public:
	ArtNetRdmResponder(void);
	~ArtNetRdmResponder(void);

	void Full(void);
	const uint8_t GetUidCount(void);
	void Copy(uint8_t *);
	const uint8_t *Handler(const uint8_t *);

	void DumpTod(void);

private:
	RDMDiscovery m_Discovery;
	RDMDeviceController m_Controller;
	struct TRdmMessage *m_pRdmCommand;
};

#endif /* ARTNETDISCOVERY_H_ */
