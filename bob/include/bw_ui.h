#ifndef BW_UI_H_
#define BW_UI_H_

#include <stdint.h>

#define BW_UI_DEFAULT_SLAVE_ADDRESS		0x94
#define BW_UI_I2C_BYTE_WAIT_US			16

#define BW_UI_OK 						0
#define BW_UI_ERROR						1

#define BW_UI_MAX_CHARACTERS			16
#define BW_UI_MAX_LINES					2
#define BW_UI_ID_STRING_LENGTH			20

#ifdef BW_I2C_UI
#include <bw_i2c_ui.h>
#define ui_start 				bw_i2c_ui_start
#define ui_end					bw_i2c_ui_end
#define ui_reinit				bw_i2c_ui_reinit
#define ui_set_cursor			bw_i2c_ui_set_cursor
#define ui_text					bw_i2c_ui_text
#define ui_text_line_1			bw_i2c_ui_text_line_1
#define ui_text_line_2			bw_i2c_ui_text_line_2
#define ui_text_line_3			bw_i2c_ui_text_line_3
#define ui_text_line_4			bw_i2c_ui_text_line_4
#define	 ui_cls					bw_i2c_ui_cls
#define ui_set_contrast			bw_i2c_ui_set_contrast
#define ui_set_backlight		bw_i2c_ui_set_backlight
#define ui_set_backlight_temp	bw_i2c_ui_set_backlight_temp
#define ui_get_backlight		bw_i2c_ui_get_backlight
#define ui_get_contrast			bw_i2c_ui_get_contrast
#define ui_read_id				bw_i2c_ui_read_id
#define ui_read_button			bw_i2c_ui_read_button
#define ui_read_button_last		bw_i2c_ui_read_button_last
#endif

#ifdef BW_SPI_UI
#include <bw_spi_ui.h>
#define ui_start 				bw_spi_ui_start
#define ui_end					bw_spi_ui_end
#define ui_reinit				bw_spi_ui_reinit
#define ui_set_cursor			bw_spi_ui_set_cursor
#define ui_text					bw_spi_ui_text
#define ui_text_line_1			bw_spi_ui_text_line_1
#define ui_text_line_2			bw_spi_ui_text_line_2
#define ui_text_line_3			bw_spi_ui_text_line_3
#define ui_text_line_4			bw_spi_ui_text_line_4
#define	 ui_cls					bw_spi_ui_cls
#define ui_set_contrast			bw_spi_ui_set_contrast
#define ui_set_backlight		bw_spi_ui_set_backlight
#define ui_set_backlight_temp	bw_spi_ui_set_backlight_temp
#define ui_read_id				bw_spi_ui_read_id
#define ui_read_button			bw_spi_ui_read_button
#define ui_read_button_last		bw_spi_ui_read_button_last
#endif

extern int ui_start(char);
extern void ui_end(void);
extern void ui_reinit(void);
extern void ui_set_cursor(uint8_t, uint8_t );
extern void ui_text(const char *, uint8_t );
extern void ui_text_line_1(const char *, uint8_t);
extern void ui_text_line_2(const char *, uint8_t);
extern void ui_text_line_3(const char *, uint8_t);
extern void ui_text_line_4(const char *, uint8_t);
extern void ui_cls(void);
extern void ui_set_contrast(uint8_t);
extern void ui_set_backlight(uint8_t);
extern void ui_set_backlight_temp(uint8_t);
extern void ui_get_backlight(uint8_t *);
extern void ui_get_contrast(uint8_t *);
extern void ui_read_id(void);
// UI specific
extern char ui_read_button(char);
extern char ui_read_button_last(void);

#endif /* BW_UI_H_ */
