#ifndef WS28XX_H_
#define WS28XX_H_

#include <stdint.h>

#define WS2801_SPI_SPEED_MAX_HZ		25000000	///< 25 MHz
#define WS2801_SPI_SPEED_DEFAULT_HZ	4000000		///< 4 MHz

typedef enum ws28xxx_type{
	WS2801 = 0,
	WS2811,
	WS2812,
	WS2812B,
	WS2813
} _ws28xxx_type;

#ifdef __cplusplus
extern "C" {
#endif

extern void ws28xx_init(const uint16_t, const _ws28xxx_type, const uint32_t);
extern void ws28xx_set_led(const uint16_t, const uint8_t, const uint8_t, const uint8_t);
extern void ws28xx_update(void);
extern const uint16_t ws28xx_get_led_count(void);
extern const _ws28xxx_type ws28xx_get_led_type(void);

#ifdef __cplusplus
}
#endif

#endif /* WS28XX_H_ */
