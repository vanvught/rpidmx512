//
// oscserver.h
//
#ifndef oscserver_h
#define oscserver_h

#include <circle/sched/task.h>

class OSCServer : public CTask
{
public:
	OSCServer (unsigned nPort);
	virtual ~OSCServer (void);

	void Run (void);

private:
	virtual void MessageReceived (u8 *, int, u32) = 0;

private:
	unsigned m_nPort;
};

#endif
