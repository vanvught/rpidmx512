#ifndef _UI_H__
#define _UI_H__

#include <string.h>
#include "drawing.h"



static constexpr uint32_t UI_BORDER_THICKNESS = 1; 
static constexpr uint32_t UI_TITLEBAR_H = 20; 

static constexpr uint32_t UI_TITLEBAR_TXT_CLR = 0x00FFFFFF; 
static constexpr uint32_t UI_TITLEBAR_BG_CLR = 0x00000088; 

static constexpr uint32_t UI_WINDOW_TITLE_LENGTH_MAX = 64; 

typedef void (*f_draw_content_cb) (void *); // callback to draw window content at x,y inside w,h 



class Window {
 public:

   void setupWindow(const uint32_t x, const uint32_t y, const uint32_t w, const uint32_t h, 
                    const char * title, const uint32_t border_clr, const uint32_t bg_clr, 
                    f_draw_content_cb draw_cb, Window * draw_cb_arg);

   void drawWindow(); 

   
   uint32_t m_nContentX; // window content location and dimension in pixels
   uint32_t m_nContentY;
   uint32_t m_nContentW;
   uint32_t m_nContentH;

   bool m_bForceRepaint; // force to repaint whole window and content when true

   Window *Get(void){
      return this;
   }

   Window() {  
      m_nWindowH = 0; m_nWindowW = 0; m_nWindowX = 0; m_nWindowY = 0; 
      m_nContentH = 0; m_nContentW = 0; m_nContentX = 0; m_nContentY = 0;
      m_nBorderColor = 0xFFFFFFFF; m_nBackgroundColor = 0x00000000;
      m_cb_fDraw_cb = nullptr; m_p_cbDrawArgs = nullptr; 
      memset(&m_sTitle[0], 0, UI_WINDOW_TITLE_LENGTH_MAX); 
      m_bForceRepaint = false;
   }

 private:
  
   uint32_t m_nWindowX; // overall window location and dimension in pixels
   uint32_t m_nWindowY;
   uint32_t m_nWindowW;
   uint32_t m_nWindowH;

   uint32_t m_nBorderColor; 
   uint32_t m_nBackgroundColor;

   char m_sTitle[UI_WINDOW_TITLE_LENGTH_MAX]; // null terminated string

   f_draw_content_cb m_cb_fDraw_cb; // draw callback function pointer
   Window * m_p_cbDrawArgs; // draw callback arguments - is pointer to Window object.


};




void Window::drawWindow() {
   Drawing *draw = Drawing::Get();
   // FIXME: Implement UI_BORDER_THICKNESS
   draw->rect(m_nWindowX, m_nWindowY, m_nWindowW , m_nWindowH, m_nBorderColor);  // window frame
   draw->fillRect(m_nWindowX + UI_BORDER_THICKNESS, 
                  m_nWindowY + UI_BORDER_THICKNESS, 
                  m_nWindowW - (2 * UI_BORDER_THICKNESS),
                  UI_TITLEBAR_H - 1, UI_TITLEBAR_BG_CLR); // clear title bar area to bg_clr
   draw->line( m_nWindowX + UI_BORDER_THICKNESS, 
               m_nWindowY + UI_TITLEBAR_H + UI_BORDER_THICKNESS, 
               m_nWindowX + m_nWindowW - (2 * UI_BORDER_THICKNESS),
               m_nWindowY + UI_TITLEBAR_H + UI_BORDER_THICKNESS, m_nBorderColor);  // title bar frame               
   draw->text(m_nWindowX + UI_BORDER_THICKNESS+1, m_nWindowY + 3, &m_sTitle[0], UI_TITLEBAR_TXT_CLR); // title bar text
   draw->fillRect(m_nContentX, m_nContentY, m_nContentW, m_nContentH, m_nBackgroundColor); // clear content area to bg_clr

   if (m_cb_fDraw_cb && m_p_cbDrawArgs) m_cb_fDraw_cb(m_p_cbDrawArgs); // call the draw content area callback with a pointer to this instance.

   m_bForceRepaint = false; //  drawn
}


void Window::setupWindow(const uint32_t x, const uint32_t y, const uint32_t w, const uint32_t h, 
                         const char * title, const uint32_t border_clr, const uint32_t bg_clr, 
                         f_draw_content_cb draw_cb, Window * draw_cb_arg) {

   // FIXME check window is on screen!
   m_nWindowX = x; 
   m_nWindowY = y; 
   m_nWindowW = w; 
   m_nWindowH = h;

   m_nBackgroundColor = bg_clr;
   m_nBorderColor = border_clr;

   memset(&m_sTitle[0], 0, UI_WINDOW_TITLE_LENGTH_MAX);
   const uint32_t len = strlen(title);
   if (len && (len <= UI_WINDOW_TITLE_LENGTH_MAX)){
      sprintf(&m_sTitle[0], "%s", title);
   } else
   {
      sprintf(&m_sTitle[0], "Error! Title To Long (%d)!\x0", len);
   }
   
   // define content area
   m_nContentX = x + UI_BORDER_THICKNESS;
   m_nContentY = y + UI_BORDER_THICKNESS + UI_TITLEBAR_H + 1;
   m_nContentW = w - (2 * UI_BORDER_THICKNESS); // minus L & R border width
   m_nContentH = h - UI_TITLEBAR_H - (UI_BORDER_THICKNESS) - 1; // minus border and title bar 

   // draw content callback and args
   m_cb_fDraw_cb = draw_cb;
   m_p_cbDrawArgs = draw_cb_arg;  // this

   m_bForceRepaint = true;  // first draw   
   drawWindow();
}





#endif