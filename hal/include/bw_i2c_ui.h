#ifndef BW_I2C_UI_H_
#define BW_I2C_UI_H_

#include <stdint.h>

extern int bw_i2c_ui_start (char);
extern void bw_i2c_ui_end (void);

extern void bw_i2c_ui_reinit(void);

extern void bw_i2c_ui_text_line_1(const char *, uint8_t);
extern void bw_i2c_ui_text_line_2(const char *, uint8_t);
extern void bw_i2c_ui_text_line_3(const char *, uint8_t);
extern void bw_i2c_ui_text_line_4(const char *, uint8_t);

extern void bw_i2c_ui_cls(void);
extern void bw_i2c_ui_set_contrast(uint8_t);
extern void bw_i2c_ui_set_backlight(uint8_t);

extern void bw_i2c_ui_get_backlight(uint8_t *);
extern void bw_i2c_ui_get_contrast(uint8_t *);

extern void bw_i2c_ui_read_id(void);

extern char bw_i2c_ui_read_button(char);
extern char bw_i2c_ui_read_button_last(void);

#endif /* BW_I2C_UI_H_ */
