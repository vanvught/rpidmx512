#include <stdint.h>
#include <assert.h>
#include <circle/string.h>
#include <circle/logger.h>
#include <circle/util.h>
#include <circle/net/netsubsystem.h>
#include <circle/net/ipaddress.h>
#include <circle/net/socket.h>
#include <circle/net/in.h>

#include "oscutil.h"
#include "udpclient.h"

UDPClient::UDPClient(CSocket *pSocket, CIPAddress *pAddress, u16 port) :
		m_pSocket(pSocket), m_pAddress(pAddress), m_Port(port)
{

}

UDPClient::~UDPClient(void)
{

}

int UDPClient::Send(const char *buf, int len)
{
	int nFlag = 0;

	int ret = m_pSocket->SendTo((const void *)buf, (unsigned)len, nFlag, *m_pAddress, (u16)m_Port);

	if (ret != len)
	{
		return -1;
	}

	return len;
}
