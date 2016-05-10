//
// oscserver.h
//
#ifndef oscserver_h
#define oscserver_h

#include <circle/net/socket.h>
#include <circle/sched/task.h>
#include <circle/net/netsubsystem.h>

class OSCServer : public CTask
{
public:
	OSCServer (CNetSubSystem *pNet, unsigned nPort);
	virtual ~OSCServer (void);

	void Run (void);

private:
	virtual void MessageReceived (u8 *, int, CIPAddress *) = 0;

private:
	CNetSubSystem *m_pNet;
	unsigned m_nPort;

protected:
	CSocket m_Socket;

};

#endif
