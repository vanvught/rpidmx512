#include <stdio.h>

#include "tftpfileserver.h"

TFTPFileServer::TFTPFileServer(void): m_pFile(0) {
}

TFTPFileServer::~TFTPFileServer(void) {
}

bool TFTPFileServer::FileOpen(const char* pFileName, TTFTPMode tMode) {
	m_pFile = fopen(pFileName, "r");
	return (m_pFile != NULL);
}

bool TFTPFileServer::FileCreate(const char* pFileName, TTFTPMode tMode) {
	m_pFile = fopen(pFileName, "wb");
	return (m_pFile != NULL);
}

bool TFTPFileServer::FileClose(void) {
	if (m_pFile != 0) {
		fclose(m_pFile);
		m_pFile = 0;
	}
	return true;
}

int TFTPFileServer::FileRead(void* pBuffer, unsigned nCount) {
	return fread(pBuffer, 1, nCount, m_pFile);
}

int TFTPFileServer::FileWrite(const void* pBuffer, unsigned nCount) {
	return fwrite(pBuffer, 1, nCount, m_pFile);
}
