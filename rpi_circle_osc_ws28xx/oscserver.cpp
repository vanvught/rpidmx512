//
// oscserver.cpp
//
#include <assert.h>

#include <circle/net/socket.h>
#include <circle/sched/scheduler.h>
#include <circle/logger.h>

#include "oscserver.h"

#include "network.h"

#define STACK_SIZE	TASK_STACK_SIZE

static const char FromOSCServer[] = "oscserv";

OSCServer::OSCServer (unsigned nPort) :	CTask (STACK_SIZE),
	m_nPort (nPort)
{
}

OSCServer::~OSCServer (void)
{
}

void OSCServer::Run (void)
{
	network_begin(m_nPort);

	while (1)
	{
		u8 Buffer[FRAME_BUFFER_SIZE];
		u32 ForeignIP;
		u16 nForeignPort;

		int nBytesReceived = network_recvfrom(Buffer, sizeof Buffer, &ForeignIP, &nForeignPort);

		if (nBytesReceived < 0)
		{
			CLogger::Get ()->Write (FromOSCServer, LogError, "Cannot receive");
			return;
		}

		if (nBytesReceived > 0)
		{
			MessageReceived (Buffer, nBytesReceived, ForeignIP);
		}

		CScheduler::Get ()->Yield ();
	}
}
