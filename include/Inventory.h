#ifndef INVENTORY_H
#define INVENTORY_H

#define NUM_SLOTS_X 9
#define NUM_SLOTS_Y 4
#define NUM_SLOTS NUM_SLOTS_X * NUM_SLOTS_Y

#include "ui/BlockStack.h"


void inventory_init();
BlockStack inventory_get_stack_from_slot(int x, int y);
void inventory_set_stack_in_slot(int x, int y, BlockStack type);
void inventory_add_block(BlockStack type);
void inventory_use_block_from_hotbar();
BlockId inventory_get_block_from_hotbar_slot(int i);


#endif
