#ifndef TFTPFILESERVER_H_
#define TFTPFILESERVER_H_

#include <stdio.h>

#include "net/apps/tftpdaemon.h"

class TFTPFileServer final: public TFTPDaemon {
public:
	TFTPFileServer();
	~TFTPFileServer() override;

	bool FileOpen(const char *pFileName, TFTPMode tMode) override;
	bool FileCreate(const char *pFileName, TFTPMode tMode) override;
	bool FileClose() override;
	int FileRead(void *pBuffer, unsigned nCount);
	int FileWrite(const void *pBuffer, unsigned nCount);

private:
	FILE *m_pFile{nullptr};
};

#endif /* TFTPFILESERVER_H_ */
