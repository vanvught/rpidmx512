/*
 * hub75bdisplay.h
 */
/**
 * PoC
 */

#ifndef HUB75BDISPLAY_H_
#define HUB75BDISPLAY_H_

#include <stdint.h>

#include "lightset.h"

class Hub75bDisplay: public LightSet {
public:
	Hub75bDisplay(uint32_t nColumns, uint32_t nRows);

	void Start();
	void Run();

	void SetPixel(uint32_t nColumn, uint32_t nRow, uint8_t nRed, uint8_t nGreen, uint8_t nBlue);

	uint32_t GetFps();

	void Dump();

	// LightSet
	void Start(__attribute__((unused)) uint8_t nPort) {

	}
	void Stop(__attribute__((unused)) uint8_t nPort) {

	}
	void SetData(uint8_t nPort, const uint8_t *pData, uint16_t nLength);

private:
	uint32_t m_nColumns;
	uint32_t m_nRows;
	uint32_t *m_pFramebuffer{0};
	uint8_t m_TablePWM[255];
};

#endif /* HUB75BDISPLAY_H_ */
