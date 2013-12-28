#ifndef BW_LCD_H_
#define BW_LCD_H_

#include <stdint.h>

#define BW_LCD_DEFAULT_SLAVE_ADDRESS	0x82
#define BW_LCD_I2C_BYTE_WAIT_US			10

#define BW_LCD_OK						0
#define BW_LCD_ERROR					1

#define BW_LCD_MAX_CHARACTERS			16
#define BW_LCD_MAX_LINES				2
#define BW_LCD_ID_STRING_LENGTH			20

#ifdef BW_I2C_LCD
#include <bw_i2c_lcd.h>
#define lcd_start 			bw_i2c_lcd_start
#define lcd_end				bw_i2c_lcd_end
#define lcd_reinit			bw_i2c_lcd_reinit
#define lcd_set_cursor		bw_i2c_lcd_set_cursor
#define lcd_text			bw_i2c_lcd_text
#define lcd_text_line_1		bw_i2c_lcd_text_line_1
#define lcd_text_line_2		bw_i2c_lcd_text_line_2
#define lcd_text_line_3		bw_i2c_lcd_text_line_3
#define lcd_text_line_4		bw_i2c_lcd_text_line_4
#define	 lcd_cls			bw_i2c_lcd_cls
#define lcd_set_contrast	bw_i2c_lcd_set_contrast
#define lcd_set_backlight	bw_i2c_lcd_set_backlight
#define lcd_read_id			bw_i2c_lcd_read_id
#endif

#ifdef BW_SPI_LCD
#include <bw_spi_lcd.h>
#define lcd_start 				bw_spi_lcd_start
#define lcd_end					bw_spi_lcd_end
#define lcd_reinit				bw_spi_lcd_reinit
#define lcd_set_cursor			bw_spi_lcd_set_cursor
#define lcd_text				bw_spi_lcd_text
#define lcd_text_line_1			bw_spi_lcd_text_line_1
#define lcd_text_line_2			bw_spi_lcd_text_line_2
#define lcd_text_line_3			bw_spi_lcd_text_line_3
#define lcd_text_line_4			bw_spi_lcd_text_line_4
#define	 lcd_cls				bw_spi_lcd_cls
#define lcd_set_contrast		bw_spi_lcd_set_contrast
#define lcd_set_backlight		bw_spi_lcd_set_backlight
#define lcd_set_backlight_temp	bw_spi_lcd_set_backlight_temp
#define lcd_read_id				bw_spi_lcd_read_id
#endif

extern int lcd_start (char);
extern void lcd_end (void);
extern void lcd_reinit(void);
extern void lcd_set_cursor(uint8_t, uint8_t );
extern void lcd_text(const char *, uint8_t );
extern void lcd_text_line_1(const char *, uint8_t);
extern void lcd_text_line_2(const char *, uint8_t);
extern void lcd_text_line_3(const char *, uint8_t);
extern void lcd_text_line_4(const char *, uint8_t);
extern void lcd_cls(void);
extern void lcd_set_contrast(uint8_t);
extern void lcd_set_backlight(uint8_t);
extern void lcd_set_backlight_temp(uint8_t);
extern void lcd_read_id(void);

#endif /* BW_LCD_H_ */
