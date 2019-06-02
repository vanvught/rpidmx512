

#ifndef TFTPFILESERVER_H_
#define TFTPFILESERVER_H_

#include <stdio.h>

#include "tftpdaemon.h"

class TFTPFileServer: public TFTPDaemon {
public:
	TFTPFileServer (void);
	~TFTPFileServer (void);

	bool FileOpen (const char *pFileName, TTFTPMode tMode);
	bool FileCreate (const char *pFileName, TTFTPMode tMode);
	bool FileClose (void);
	int FileRead (void *pBuffer, unsigned nCount);
	int FileWrite (const void *pBuffer, unsigned nCount);

private:
	FILE *m_pFile;
};

#endif /* TFTPFILESERVER_H_ */
