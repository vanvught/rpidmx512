#ifndef BW_SPI_LCD_H_
#define BW_SPI_LCD_H_

#include <device_info.h>
#include <bw_lcd.h>

#include <stdint.h>

#define BW_LCD_SPI_BYTE_WAIT_US			8

extern int bw_spi_lcd_start (device_info_t *);
extern void bw_spi_lcd_end (void);

extern void bw_spi_lcd_reinit(device_info_t *);

extern void bw_spi_lcd_text_line_1(device_info_t *, const char *, uint8_t);
extern void bw_spi_lcd_text_line_2(device_info_t *, const char *, uint8_t);
extern void bw_spi_lcd_text_line_3(device_info_t *, const char *, uint8_t);
extern void bw_spi_lcd_text_line_4(device_info_t *, const char *, uint8_t);

extern void bw_spi_lcd_cls(device_info_t *);
extern void bw_spi_lcd_set_contrast(device_info_t *, uint8_t);
extern void bw_spi_lcd_set_backlight(device_info_t *, uint8_t);

extern void bw_spi_lcd_read_id(device_info_t *);

#endif /* BW_SPI_LCD_H_ */
