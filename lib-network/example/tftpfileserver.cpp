#include <stdio.h>

#include "tftpfileserver.h"

TFTPFileServer::TFTPFileServer() {
}

TFTPFileServer::~TFTPFileServer() {
}

bool TFTPFileServer::FileOpen(const char *pFileName, TFTPMode tMode) {
	m_pFile = fopen(pFileName, "r");
	return (m_pFile != nullptr);
}

bool TFTPFileServer::FileCreate(const char *pFileName, TFTPMode tMode) {
	m_pFile = fopen(pFileName, "wb");
	return (m_pFile != nullptr);
}

bool TFTPFileServer::FileClose() {
	if (m_pFile != nullptr) {
		static_cast<void>(fclose(m_pFile));
		m_pFile = nullptr;
	}
	return true;
}

int TFTPFileServer::FileRead(void *pBuffer, unsigned nCount) {
	return fread(pBuffer, 1, nCount, m_pFile);
}

int TFTPFileServer::FileWrite(const void *pBuffer, unsigned nCount) {
	return fwrite(pBuffer, 1, nCount, m_pFile);
}
