#ifndef HOTBAR_H
#define HOTBAR_H
#include "BlockType.h"

void UIHotbar_init();

int UIHotbar_get_current_index();
void UIHotbar_set_slot_item_texture(int slotIndex, const char* texture);

BlockType UIHotbar_move_selector_to_slot(int slotSelected);

void UIHotbar_draw();

#endif
