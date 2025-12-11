#ifndef UIITEM_H
#define UIITEM_H
#include <cglm/vec2.h>

#include "BlockType.h"

typedef unsigned int UIItem;

UIItem UIItem_init(BlockType type, vec2 position, vec2 size);
void UIItem_set_type(UIItem item, BlockType type);
void UIItem_set_position(UIItem item, vec2 position);

void UIItem_get_size(UIItem item, vec2 size);
void UIItem_get_position(UIItem item, vec2 position);

void UIItem_draw(UIItem item);

#endif