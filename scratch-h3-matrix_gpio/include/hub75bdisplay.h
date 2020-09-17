/*
 * hub75bdisplay.h
 */
/**
 * PoC
 */

#include <stdint.h>

#ifndef HUB75BDISPLAY_H_
#define HUB75BDISPLAY_H_


class Hub75bDisplay {
public:
	Hub75bDisplay(uint32_t nColumns, uint32_t nRows);

	void Run();

private:
	uint32_t m_nColumns;
	uint32_t m_nRows;
};

#endif /* HUB75BDISPLAY_H_ */
