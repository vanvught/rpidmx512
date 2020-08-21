
#include <stdint.h>
//#include <stdio.h>
#include <string.h>

#include "device/fb.h"
#include "font-7seg-24.h"
#include "fontem.h"


inline const struct glyph *font_get_glyph(const struct font *font, glyph_t glyph) {
	if (glyph > font->max) return NULL;

	size_t first = 0, last = font->count;
	const struct glyph **glyphs = font->glyphs;

	while (first < last) {
		size_t mid = first + (last - first) / 2;
		if (glyph <= glyphs[mid]->glyph)
			last = mid;
		else
			first = mid + 1;
	}
	return (last < font->count && glyphs[last]->glyph == glyph) ? *(glyphs + last) : NULL;
}


inline int16_t font_get_kerning(const struct font *font, glyph_t left, glyph_t right) {
	
	if (font == NULL || left == 0 || right == 0) return 0;
	const struct glyph *g = font_get_glyph(font, right);
	if (g == NULL || g->kerning == NULL) return 0;

	const struct kerning *k;
	for (k = g->kerning; k->left != 0; k++)
		if (k->left == left) return k->offset;
	return 0;
}


inline int font_calculate_box(const struct font *font, int *maxx, int *maxy, const char *str) {
	if ((font == NULL) || (str == NULL)) return 0;

	int x = 0;
	int y = font->height;
	int count = 0;
	glyph_t prev = 0;
	for (const char *p = str; *p; p++, count++) {
		const struct glyph *g = font_get_glyph(font, *p);
		if (g == NULL) continue;
		x += g->advance + font_get_kerning(font, prev, *p);
		prev = *p;
	}

	*maxx = x;
	*maxy = y;
	return count;
}


inline int font_draw_glyph_RGBA32(const struct font *font,
							int x, int y, int width, int height,
							uint8_t *buf, const struct glyph *glyph,
							uint32_t rgb) {

	uint8_t r = rgba32_get_r(rgb);
	uint8_t g = rgba32_get_g(rgb);
	uint8_t b = rgba32_get_b(rgb);

	unsigned rows = glyph->rows, cols = glyph->cols;
	const unsigned char *data = glyph->bitmap;
	unsigned char count = 0, cclass = 0;

	for (unsigned row = 0; row < rows; row++) {
		int yofs = row + y + (font->ascender - glyph->top);

		for (unsigned col = 0; col < cols; col++) {
			int xofs = col + x + glyph->left;
			uint8_t val;

			if (font->compressed) {
				if (count == 0) {
					count = (*data & 0x3f) + 1;
					cclass = *(data++) >> 6;
				}				
				if (cclass == 0)
					val = *(data++);
				else if (cclass == 3)
					val = 0xff;
				else
					val = 0;
				count--;
			} else {
				val = data[(row * cols) + col];
			}

			if ((yofs >= 0) && (yofs < height) && (xofs >= 0) && (xofs < width)) {
				uint8_t *pixel = buf + (yofs * width * 4) + (xofs * 4);				
				*pixel = blend(0, r, val);
				pixel++;
				*pixel = blend(0, g, val);
				pixel++;
				*pixel = blend(0, b, val);
				pixel++; // skip 4th byte
			}
		}
	}
	return glyph->advance;
}


inline int font_draw_char_RGBA32(const struct font *font,
			  int x, int y, int width, int height,
			  uint8_t *buf, glyph_t glyph, glyph_t prev,
			  uint32_t rgb) {

	if (font == NULL) return -1;
	const struct glyph *g = font_get_glyph(font, glyph);
	if (g == NULL) return -2;
	int kerning_offset = font_get_kerning(font, prev, glyph);

	return font_draw_glyph_RGBA32(font, x + kerning_offset, y, width, height, buf, g, rgb) + kerning_offset;
}


void font_7seg_puts(int x, int y, char * strng, uint32_t clr) {
	char *p = strng;
	char prev = 0;
	while (*p) {
		x += font_draw_char_RGBA32(&font__seg_24, x, y, fb_width, fb_height, (unsigned char *)fb_addr, *p, prev, clr);
		prev = *p;
		p++;
	}
}