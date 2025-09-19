#ifndef HOTBAR_H
#define HOTBAR_H
#include "BlockStack.h"
#include "BlockType.h"

void UIHotbar_init();

int UIHotbar_get_current_index();
void UIHotbar_reload_slot(int slotIndex, BlockStack stack);

BlockType UIHotbar_move_selector_to_slot(int slotSelected);

void UIHotbar_draw();

#endif
