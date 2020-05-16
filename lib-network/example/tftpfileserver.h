

#ifndef TFTPFILESERVER_H_
#define TFTPFILESERVER_H_

#include <stdio.h>

#include "tftpdaemon.h"

class TFTPFileServer: public TFTPDaemon {
public:
	TFTPFileServer (void);
	~TFTPFileServer (void);

	bool FileOpen (const char *pFileName, TFTPMode tMode);
	bool FileCreate (const char *pFileName, TFTPMode tMode);
	bool FileClose (void);
	int FileRead (void *pBuffer, unsigned nCount);
	int FileWrite (const void *pBuffer, unsigned nCount);

private:
	FILE *m_pFile;
};

#endif /* TFTPFILESERVER_H_ */
