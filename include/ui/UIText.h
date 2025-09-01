#ifndef UITEXT_H
#define UITEXT_H
#include <stddef.h>
#include <cglm/types.h>

typedef struct {
    mat4 transform;
    unsigned int texture;
} UIChar;

typedef struct {
    UIChar* chars;
    size_t len;
    bool enabled;
} UIText;

void UIText_init(UIText* ui_text, const char* text, vec2 position, bool enabled);
void UIText_draw(UIText* ui_text);

#endif
