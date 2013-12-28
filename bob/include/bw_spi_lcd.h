#ifndef BW_SPI_LCD_H_
#define BW_SPI_LCD_H_

#include <stdint.h>

extern int bw_spi_lcd_start (char);
extern void bw_spi_lcd_end (void);

extern void bw_spi_lcd_reinit(void);

extern void bw_spi_lcd_text_line_1(char *, uint8_t);
extern void bw_spi_lcd_text_line_2(char *, uint8_t);

extern void bw_spi_lcd_cls(void);
extern void bw_spi_lcd_set_contrast(uint8_t);
extern void bw_spi_lcd_set_backlight(uint8_t);

extern void bw_spi_lcd_read_id(void);

#endif /* BW_SPI_LCD_H_ */
