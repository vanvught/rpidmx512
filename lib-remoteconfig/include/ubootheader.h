#ifndef UBOOTHEADER_H_
#define UBOOTHEADER_H_

#include <stdint.h>
#include <stdbool.h>

class UBootHeader {
public:
	UBootHeader(uint8_t *pHeader);
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
