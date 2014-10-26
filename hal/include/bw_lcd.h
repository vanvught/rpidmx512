/* Copyright (C) 2014 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef BW_LCD_H_
#define BW_LCD_H_

#include <stdint.h>

#define BW_LCD_DEFAULT_SLAVE_ADDRESS	0x82

#define BW_LCD_OK						0
#define BW_LCD_ERROR					1

#define BW_LCD_MAX_CHARACTERS			16
#define BW_LCD_MAX_LINES				2

#ifdef BW_I2C_LCD
#define lcd_start 						bw_i2c_lcd_start
#define lcd_end							bw_i2c_lcd_end
#define lcd_reinit						bw_i2c_lcd_reinit
#define lcd_set_cursor					bw_i2c_lcd_set_cursor
#define lcd_text						bw_i2c_lcd_text
#define lcd_text_line_1					bw_i2c_lcd_text_line_1
#define lcd_text_line_2					bw_i2c_lcd_text_line_2
#define lcd_text_line_3					bw_i2c_lcd_text_line_3
#define lcd_text_line_4					bw_i2c_lcd_text_line_4
#define	 lcd_cls						bw_i2c_lcd_cls
#define lcd_set_contrast				bw_i2c_lcd_set_contrast
#define lcd_set_backlight				bw_i2c_lcd_set_backlight
#define lcd_set_backlight_temp			bw_i2c_lcd_set_backlight_temp
#define lcd_set_startup_message_line_1	bw_i2c_lcd_set_startup_message_line_1
#define lcd_set_startup_message_line_2	bw_i2c_lcd_set_startup_message_line_2
#define lcd_set_startup_message_line_3	bw_i2c_lcd_set_startup_message_line_3
#define lcd_set_startup_message_line_4	bw_i2c_lcd_set_startup_message_line_4
#define lcd_read_id						bw_i2c_lcd_read_id
#endif

#ifdef BW_SPI_LCD
#define lcd_start 						bw_spi_lcd_start
#define lcd_end							bw_spi_lcd_end
#define lcd_reinit						bw_spi_lcd_reinit
#define lcd_set_cursor					bw_spi_lcd_set_cursor
#define lcd_text						bw_spi_lcd_text
#define lcd_text_line_1					bw_spi_lcd_text_line_1
#define lcd_text_line_2					bw_spi_lcd_text_line_2
#define lcd_text_line_3					bw_spi_lcd_text_line_3
#define lcd_text_line_4					bw_spi_lcd_text_line_4
#define	 lcd_cls						bw_spi_lcd_cls
#define lcd_set_contrast				bw_spi_lcd_set_contrast
#define lcd_set_backlight				bw_spi_lcd_set_backlight
#define lcd_set_backlight_temp			bw_spi_lcd_set_backlight_temp
#define lcd_startup_message_line_1		bw_spi_lcd_startup_message_line_1
#define lcd_startup_message_line_2		bw_spi_lcd_startup_message_line_2
#define lcd_startup_message_line_3		bw_spi_lcd_startup_message_line_3
#define lcd_startup_message_line_4		bw_spi_lcd_startup_message_line_4
#define lcd_read_id						bw_spi_lcd_read_id
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
