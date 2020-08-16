#ifndef RENDERER_H
#define RENDERER_H

#include <stdint.h>
#include "microui.h"

#ifdef __cplusplus
extern "C" {
#endif

void r_init(uint32_t w, uint32_t h);
void r_draw_rect(mu_Rect rect, mu_Color color);
void r_draw_text(const char *text, mu_Vec2 pos, mu_Color color);
void r_draw_icon(int id, mu_Rect rect, mu_Color color);
 int r_get_text_width(const char *text, int len);
 int r_get_text_height(void);
void r_set_clip_rect(mu_Rect rect);
void r_clear(mu_Color color);
void r_present(void);

#ifdef __cplusplus
}
#endif


#endif

