#ifndef HOTBAR_H
#define HOTBAR_H
#include "BlockStack.h"

void UIHotbar_init();

int UIHotbar_get_current_index();
void UIHotbar_reload_slot(int slotIndex, BlockStack stack);

BlockId UIHotbar_move_selector_to_slot(int slotSelected);

void UIHotbar_draw();

#endif
