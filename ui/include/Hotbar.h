#ifndef HOTBAR_H
#define HOTBAR_H
#include "../../include/BlockType.h"

void Hotbar_init();

int Hotbar_get_current_index();
BlockType Hotbar_change_selection(int slotSelected);

void Hotbar_draw();

#endif
