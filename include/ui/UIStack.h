#ifndef UISTACK_H
#define UISTACK_H
#include <cglm/vec2.h>
#include "BlockStack.h"

typedef unsigned int UIStack;

UIStack UIStack_init(BlockId type, int amount, vec2 position, vec2 size, bool enabled);
void UIStack_set_amount(UIStack uistack, int amount);
void UIStack_set_stack(UIStack uistack, BlockStack stack);
//void UIStack_set_texture(UIStack slot, const char* itemTexture);
void UIStack_set_position(UIStack uistack, vec2 position);
void UIStack_set_enabled(UIStack uistack, bool enabled);
void UIStack_get_position(UIStack uistack, vec2 position);
void UIStack_get_size(UIStack uistack, vec2 size);
void UIStack_draw(UIStack uistack);

#endif
