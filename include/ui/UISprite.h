#ifndef UISPRITE_H
#define UISPRITE_H
#include <cglm/mat4.h>

typedef unsigned int UISprite;

UISprite UISprite_init(const char* texture, vec2 position, vec2 size, bool enabled);

void UISprite_set_position(UISprite sprite, vec2 position);
void UISprite_set_texture(UISprite sprite, const char* texture);
void UISprite_set_enabled(UISprite sprite, bool enabled);

void UISprite_get_size(UISprite sprite, vec2 size);
void UISprite_get_position(UISprite sprite, vec2 position);

void UISprite_draw(void);

#endif
