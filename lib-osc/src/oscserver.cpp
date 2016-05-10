//
// oscserver.cpp
//
#include "oscserver.h"
#include <circle/net/socket.h>
#include <circle/net/in.h>
#include <circle/sched/scheduler.h>
#include <circle/logger.h>
#include <assert.h>

#define STACK_SIZE	TASK_STACK_SIZE

static const char FromOSCServer[] = "oscserv";

OSCServer::OSCServer (CNetSubSystem *pNet, unsigned nPort)
:	CTask (STACK_SIZE),
	m_pNet (pNet),
	m_nPort (nPort),
	m_Socket (m_pNet, IPPROTO_UDP)
{
}

OSCServer::~OSCServer (void)
{
	m_pNet = 0;
}

void OSCServer::Run (void)
{
	assert (m_pNet != 0);
	//
	if (m_Socket.Bind (m_nPort) < 0)
	{
		CLogger::Get ()->Write (FromOSCServer, LogError, "Cannot bind socket (port %u)", m_nPort);

		return;
	}

	while (1)
	{
		u8 Buffer[FRAME_BUFFER_SIZE];
		CIPAddress ForeignIP;
		u16 nForeignPort;
		int nBytesReceived = m_Socket.ReceiveFrom (Buffer, sizeof Buffer, MSG_DONTWAIT, &ForeignIP, &nForeignPort);
		if (nBytesReceived < 0)
		{
			CLogger::Get ()->Write (FromOSCServer, LogError, "Cannot receive");

			return;
		}

		if (nBytesReceived > 0)
		{
			MessageReceived (Buffer, nBytesReceived, &ForeignIP);
		}

		CScheduler::Get ()->Yield ();
	}
}
