#ifndef UISPRITE_H
#define UISPRITE_H
#include <cglm/mat4.h>

typedef struct {
    mat4 transform;
    unsigned int texture;
} UISprite;

void UISprite_init(UISprite* sprite, const char* texture, vec2 position, vec2 size);

void UISprite_set_position(UISprite* sprite, vec2 position);

void UISprite_draw(UISprite* sprite);

#endif
