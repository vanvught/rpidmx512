/** Font structure definitions.
 *
 * This file is distributed under the terms of the MIT License.
 * See the LICENSE file at the top of this tree, or if it is missing a copy can
 * be found at http://opensource.org/licenses/MIT
 */

#ifndef _FONTEM_H
#define _FONTEM_H

#include <stdint.h>
//#include <stdio.h>


/** Alpha compositing "A over B" mechanism */
#define alpha_blend(in, in_alpha, out, out_alpha) \
	((0x100 * in * in_alpha * (255 - out_alpha) + out * out_alpha * (255 - in_alpha)) >> 16)
#define blend(a, b, alpha) \
	(((a) * (255 - (alpha)) + (b) * (alpha)) >> 8)

/** Extract the Alpha channel from a 32-bit RGBA value */
#define rgba32_get_a(rgba) ((rgba >> 24) & 0xff)
/** Extract the Red channel from a 32-bit RGBA value */
#define rgba32_get_r(rgba) ((rgba >> 16) & 0xff)
/** Extract the Green channel from a 32-bit RGBA value */
#define rgba32_get_g(rgba) ((rgba >> 8) & 0xff)
/** Extract the Blue channel from a 32-bit RGBA value */
#define rgba32_get_b(rgba) (rgba & 0xff)

/** Extract the Red channel from a 16-bit RGB value */
#define rgb16_get_r(rgb) ((rgb >> 8) & 0xf8)
/** Extract the Green channel from a 16-bit RGB value */
#define rgb16_get_g(rgb) ((rgb >> 3) & 0xfc)
/** Extract the Blue channel from a 16-bit RGB value */
#define rgb16_get_b(rgb) ((rgb << 3) & 0xf8)
/** Combine Red, Green and Blue channels into a 16-bit RGB value */
#define rgb16_combine(r, g, b) \
	((((r) & 0xf8) << 8) | \
	 (((g) & 0xfc) << 3) | \
	 (((b) & 0xf8) >> 3))


/** Glyph character value rype */
typedef uint16_t glyph_t;

/** Description of a glyph; a single character in a font. */
struct glyph {
	glyph_t			glyph;          /** The glyph this entry refers to */

	int16_t			left;           /** Offset of the left edge of the glyph */
	int16_t			top;            /** Offset of the top edge of the glyph */
	int16_t			advance;        /** Horizonal offset when advancing to the next glyph */

	uint16_t		cols;           /** Width of the bitmap */
	uint16_t		rows;           /** Height of the bitmap */
	const uint8_t		*bitmap;        /** Bitmap data */

	const struct kerning	*kerning;       /** Font kerning data */
};

/** Kerning table; for a pair of glyphs, provides the horizontal adjustment. */
struct kerning {
	glyph_t left;   /** The left-glyph */
	int16_t offset; /** The kerning offset for this glyph pair */
};

/** Description of a font. */
struct font {
	char			*name;          /** Name of the font */
	char			*style;         /** Style of the font */

	uint16_t		size;           /** Point size of the font */
	uint16_t		dpi;            /** Resolution of the font */

	int16_t			ascender;       /** Ascender height */
	int16_t			descender;      /** Descender height */
	int16_t			height;         /** Baseline-to-baseline height */

	uint16_t		count;          /** Number of glyphs */
	uint16_t		max;            /** Maximum glyph index */
	const struct glyph	**glyphs;       /** Font glyphs */
	char			compressed;     /** TRUE if glyph bitmaps are RLE compressed */
};


const struct glyph *font_get_glyph(const struct font *font, glyph_t glyph);
int16_t font_get_kerning(const struct font *font, glyph_t left, glyph_t right);
int font_calculate_box(const struct font *font, int *maxx, int *maxy, const char *str);

int font_draw_glyph_RGBA32(const struct font *font, int x, int y, int width, int height, uint8_t *buf, const struct glyph *g, uint32_t rgb);
int font_draw_char_RGBA32(const struct font *font, int x, int y, int width, int height, uint8_t *buf, glyph_t glyph, glyph_t prev, uint32_t rgb);


void font_7seg_puts(int x, int y, char * strng, uint32_t clr);


#endif /* _FONTEM_H */
