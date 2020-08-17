#ifndef _UI_H__
#define  _UI_H__

#include "drawing.h"

namespace Ui {
class Ui {
   static Ui *instance;
   int data;
 
   // Private constructor so that no objects can be created.
   Ui() {
      data = 0;
   }

   public:
   static Ui *Get() {
      if (!instance)
      instance = new Ui;
      return instance;
   }

   void drawWindowFrame(uint32_t x, uint32_t y, uint32_t w, uint32_t h, char * title, uint32_t border_clr);


   int getData() {
      return this -> data;
   }

   void setData(int data) {
      this -> data = data;
   }
};


//Initialize pointer to zero so that it can be initialized in first call to getInstance
Ui *Ui::instance = 0;



void Ui::drawWindowFrame(uint32_t x, uint32_t y, uint32_t w, uint32_t h, char * title, uint32_t border_clr){
    Drawing *draw = Drawing::Get();
    draw->rect()
}


} // namespace

#endif