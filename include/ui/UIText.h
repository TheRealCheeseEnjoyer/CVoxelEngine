#ifndef UITEXT_H
#define UITEXT_H
#include <cglm/types.h>

typedef unsigned int UIText;

UIText UIText_init(const char *text, vec2 position, bool enabled);

void UIText_set_enabled(UIText text, bool enabled);

void UIText_set_position(UIText ui_text, vec2 position);
void UIText_set_color(UIText ui_text, vec3 color);
void UIText_set_text(UIText ui_text, const char* text);
void UIText_draw(void);

#endif
