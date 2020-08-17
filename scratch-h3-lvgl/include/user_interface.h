#ifndef USER_INTERFACE_H_
#define USER_INTERFACE_H

#include "microui.h"

#ifdef __cplusplus
extern "C" {
#endif

int ui_text_width(mu_Font font, const char *text, int len);
int ui_text_height(mu_Font font);

void ui_process_frame(mu_Context *ctx);

#ifdef __cplusplus
}
#endif



#endif // USER_INTERFACE_H_