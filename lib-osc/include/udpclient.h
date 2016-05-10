#ifndef UDPCLIENT_H_
#define UDPCLIENT_H_

#include <assert.h>
#include <circle/util.h>
#include <circle/net/netsubsystem.h>
#include <circle/net/ipaddress.h>
#include <circle/net/socket.h>

#include "udpclient.h"


class UDPClient {
public:
	UDPClient(CSocket *, CIPAddress *, u16);
	~UDPClient(void);

	int Send(const char *, int);

private:
	CSocket *m_pSocket;
	CIPAddress *m_pAddress;
	u16 m_Port;
};

#endif /* UDPCLIENT_H_ */
