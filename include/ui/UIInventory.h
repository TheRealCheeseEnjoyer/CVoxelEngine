#ifndef UIINVENTORY_H
#define UIINVENTORY_H

#include "BlockType.h"

void UIInventory_init();
void UIInventory_draw();
void UIInventory_update();
bool UIInventory_is_enabled();
void UIInventory_toggle();
void UIInventory_reload_slot(int x, int y, BlockType type);

#endif
