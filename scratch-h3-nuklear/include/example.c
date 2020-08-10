/*
 * MIT License
 *
 * Copyright (c) 2016-2017 Patrick Rudolph <siro@das-labor.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Based on x11/main.c.
 *
*/

#define NK_INCLUDE_FIXED_TYPES
//#define NK_INCLUDE_STANDARD_IO
//#define NK_INCLUDE_STANDARD_VARARGS
//#define NK_INCLUDE_DEFAULT_ALLOCATOR

#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_SOFTWARE_FONT

#define NK_RAWFB_IMPLEMENTATION

#define NK_IMPLEMENTATION
#include "nuklear.h"
#include "nuklear_rawfb.h"


#define DTIME           20
#define WINDOW_WIDTH    800
#define WINDOW_HEIGHT   480

#define UNUSED(a) (void)a
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) < (b) ? (b) : (a))
#define LEN(a) (sizeof(a)/sizeof(a)[0])

typedef struct XWindow XWindow;
struct XWindow {
    Display *dpy;
    Window root;
    Visual *vis;
    Colormap cmap;
    XWindowAttributes attr;
    XSetWindowAttributes swa;
    Window win;
    int screen;
    unsigned int width;
    unsigned int height;
};

static void
die(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputs("\n", stderr);
    exit(EXIT_FAILURE);
}

static long
timestamp(void)
{
    struct timeval tv;
    if (gettimeofday(&tv, NULL) < 0) return 0;
    return (long)((long)tv.tv_sec * 1000 + (long)tv.tv_usec/1000);
}

static void
sleep_for(long t)
{
    struct timespec req;
    const time_t sec = (int)(t/1000);
    const long ms = t - (sec * 1000);
    req.tv_sec = sec;
    req.tv_nsec = ms * 1000000L;
    while(-1 == nanosleep(&req, &req));
}

/* ===============================================================
 *
 *                          EXAMPLE
 *
 * ===============================================================*/
/* This are some code examples to provide a small overview of what can be
 * done with this library. To try out an example uncomment the defines */
/*#define INCLUDE_ALL */
/*#define INCLUDE_STYLE */
/*#define INCLUDE_CALCULATOR */
/*#define INCLUDE_OVERVIEW */
/*#define INCLUDE_NODE_EDITOR */

#ifdef INCLUDE_ALL
  #define INCLUDE_STYLE
  #define INCLUDE_CALCULATOR
  #define INCLUDE_OVERVIEW
  #define INCLUDE_NODE_EDITOR
#endif

#ifdef INCLUDE_STYLE
  #include "../style.c"
#endif
#ifdef INCLUDE_CALCULATOR
  #include "../calculator.c"
#endif
#ifdef INCLUDE_OVERVIEW
  #include "../overview.c"
#endif
#ifdef INCLUDE_NODE_EDITOR
  #include "../node_editor.c"
#endif

/* ===============================================================
 *
 *                          DEMO
 *
 * ===============================================================*/
int
main(void)
{
    long dt;
    long started;
    int running = 1;
    int status;
    XWindow xw;
    struct rawfb_context *rawfb;
    void *fb = NULL;
    rawfb_pl pl;
    unsigned char tex_scratch[512 * 512];

    /* GUI */
    rawfb = nk_rawfb_init(fb, tex_scratch, xw.width, xw.height, xw.width * 4, pl);
    if (!rawfb) running = 0;

    #ifdef INCLUDE_STYLE
    /*set_style(&rawfb->ctx, THEME_WHITE);*/
    /*set_style(&rawfb->ctx, THEME_RED);*/
    /*set_style(&rawfb->ctx, THEME_BLUE);*/
    /*set_style(&rawfb->ctx, THEME_DARK);*/
    #endif

    while (running) {
        /* Input */
        XEvent evt;
        started = timestamp();
        nk_input_begin(&rawfb->ctx);
        while (XCheckWindowEvent(xw.dpy, xw.win, xw.swa.event_mask, &evt)) {
            if (XFilterEvent(&evt, xw.win)) continue;
            nk_xlib_handle_event(xw.dpy, xw.screen, xw.win, &evt, rawfb);
        }
        nk_input_end(&rawfb->ctx);

        /* GUI */
        if (nk_begin(&rawfb->ctx, "Demo", nk_rect(50, 50, 200, 200),
            NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|
            NK_WINDOW_CLOSABLE|NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE)) {
            enum {EASY, HARD};
            static int op = EASY;
            static int property = 20;

            nk_layout_row_static(&rawfb->ctx, 30, 80, 1);
            if (nk_button_label(&rawfb->ctx, "button"))
                fprintf(stdout, "button pressed\n");
            nk_layout_row_dynamic(&rawfb->ctx, 30, 2);
            if (nk_option_label(&rawfb->ctx, "easy", op == EASY)) op = EASY;
            if (nk_option_label(&rawfb->ctx, "hard", op == HARD)) op = HARD;
            nk_layout_row_dynamic(&rawfb->ctx, 25, 1);
            nk_property_int(&rawfb->ctx, "Compression:", 0, &property, 100, 10, 1);
        }
        nk_end(&rawfb->ctx);
        if (nk_window_is_closed(&rawfb->ctx, "Demo")) break;

        /* -------------- EXAMPLES ---------------- */
        #ifdef INCLUDE_CALCULATOR
          calculator(&rawfb->ctx);
        #endif
        #ifdef INCLUDE_OVERVIEW
          overview(&rawfb->ctx);
        #endif
        #ifdef INCLUDE_NODE_EDITOR
          node_editor(&rawfb->ctx);
        #endif
        /* ----------------------------------------- */

        /* Draw framebuffer */
        nk_rawfb_render(rawfb, nk_rgb(30,30,30), 1);

         /* Timing */
        dt = timestamp() - started;
        if (dt < DTIME)
            sleep_for(DTIME - dt);
    }

    nk_rawfb_shutdown(rawfb);

    return 0;
}

