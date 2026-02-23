#include <cstdio>

#include "tftpfileserver.h"

TFTPFileServer::TFTPFileServer() {
}

TFTPFileServer::~TFTPFileServer() {
}

bool TFTPFileServer::FileOpen(const char *pFileName, TFTPMode tMode) {
	file_ = fopen(pFileName, "r");
	return (file_ != nullptr);
}

bool TFTPFileServer::FileCreate(const char *pFileName, TFTPMode tMode) {
	file_ = fopen(pFileName, "wb");
	return (file_ != nullptr);
}

bool TFTPFileServer::FileClose() {
	if (file_ != nullptr) {
		static_cast<void>(fclose(file_));
		file_ = nullptr;
	}
	return true;
}

int TFTPFileServer::FileRead(void *pBuffer, unsigned nCount) {
	return fread(pBuffer, 1, nCount, file_);
}

int TFTPFileServer::FileWrite(const void *pBuffer, unsigned nCount) {
	return fwrite(pBuffer, 1, nCount, file_);
}
