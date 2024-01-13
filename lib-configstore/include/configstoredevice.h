/*
 * storedevice.h
 */

#ifndef STOREDEVICE_H_
#define STOREDEVICE_H_

#include <cstdint>

namespace storedevice {
enum class result {
	OK, ERROR
};
}  // namespace storedevice

#if defined (CONFIG_STORE_USE_I2C)
# include "i2c/at24cxx.h"
class StoreDevice: AT24C32 {
#elif defined (CONFIG_STORE_USE_ROM)
# include "flashcode.h"
class StoreDevice: FlashCode {
#else
class StoreDevice {
#endif
public:
	StoreDevice();
	~StoreDevice();

	bool IsDetected() const {
		return m_IsDetected;
	}
	uint32_t GetSectorSize() const;
	uint32_t GetSize() const;

	bool Read(uint32_t nOffset, uint32_t nLength, uint8_t *pBuffer, storedevice::result& nResult);
	bool Erase(uint32_t nOffset, uint32_t nLength, storedevice::result& nResult);
	bool Write(uint32_t nOffset, uint32_t nLength, const uint8_t *pBuffer, storedevice::result& nResult);

private:
	bool m_IsDetected { false };
};

#endif /* STOREDEVICE_H_ */
