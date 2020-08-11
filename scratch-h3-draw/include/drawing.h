#ifndef DRAWING_H_
#define DRAWING_H_

#if !defined ORANGE_PI_ONE
 #error Support for Orange Pi One only
#endif


#include "h3.h"
#include "h3/console_fb.h"
#include "device/fb.h"


class Drawing {
public:
	Drawing(void) {	s_pThis = this; }
	~Drawing(void) { }

    void pixel(int32_t x, int32_t y, uint32_t c); 
    void line(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t p, uint32_t pattern);
    void rect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t p);
    void fillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t p);
    void circle(int32_t x, int32_t y, int32_t radius, uint32_t p, uint8_t mask);
    void fillCircle(int32_t x, int32_t y, int32_t radius, uint32_t p);
    void triangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint32_t p);
    void fillTriangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint32_t p);

	static Drawing *Get(void) { return s_pThis;	}

private:
	static Drawing *s_pThis;

    static int32_t fbW(void) { return static_cast<int32_t>(fb_width); }
    static int32_t fbH(void) { return static_cast<int32_t>(fb_height); }
    
};

#endif