/*
 * rdmslotdescription.h
 *
 *  Created on: Jan 15, 2018
 *      Author: pi
 */

#ifndef RDMSLOTINFO_H_
#define RDMSLOTINFO_H_

#include <stdint.h>

class RDMSlotInfo {
public:
	RDMSlotInfo(void);
	~RDMSlotInfo(void);

	static const char *GetTypeText(uint8_t nId, uint8_t& nLength);
	static const char *GetCategoryText(uint16_t nId, uint8_t& nLength);

private:
	static int bsearch(uint16_t nKey);
};

#endif /* RDMSLOTINFO_H_ */
