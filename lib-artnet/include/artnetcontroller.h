/*
 * artnetcontroller.h
 *
 *  Created on: Aug 11, 2017
 *      Author: pi
 */

#ifndef ARTNETCONTROLLER_H_
#define ARTNETCONTROLLER_H_

#include <time.h>

#include "packets.h"

#include "artnetpolltable.h"
#include "artnetipprog.h"

class ArtNetController: public ArtNetPollTable {
public:
	ArtNetController(void);
	~ArtNetController(void);

	void Start(void);
	void Stop(void);

	void SetPollInterval(const uint8_t);
	const uint8_t GetPollInterval(void);

	int Run(void);

	void SendIpProg(const uint32_t, const struct TArtNetIpProg *);

private:
	void SendPoll(void);
	void HandlePollReply(void);
	void SendIpProg(void);
	void HandleIpProgReply(void);

private:
	struct TArtNetPacket	*m_pArtNetPacket;
	struct TArtPoll			m_ArtNetPoll;
	struct TArtIpProg 		m_ArtIpProg;
	time_t 					m_nLastPollTime;
	uint32_t				m_IPAddressLocal;
	uint32_t				m_IPAddressBroadcast;
	uint8_t					m_nPollInterVal;
};

#endif /* ARTNETCONTROLLER_H_ */
