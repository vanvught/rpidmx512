#ifndef UBOOTHEADER_H_
#define UBOOTHEADER_H_

#include <stdint.h>
#include <stdbool.h>

enum TImageHeaderCompression {
	IH_COMP_NONE = 0, 	/*  No	 Compression Used	*/
	IH_COMP_GZIP		/* gzip	 Compression Used	*/
};

class UBootHeader {
public:
	UBootHeader(uint8_t *pHeader, TImageHeaderCompression tImageHeaderCompression = IH_COMP_NONE);
	~UBootHeader(void);

	bool IsValid(void) {
		return m_bIsValid;
	}

	void Dump(void);

private:
	uint8_t *m_pHeader;
	bool m_bIsValid;
};

#endif
