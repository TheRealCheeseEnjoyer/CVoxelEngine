#ifndef INVENTORY_H
#define INVENTORY_H

#define NUM_SLOTS_X 9
#define NUM_SLOTS_Y 4
#define NUM_SLOTS NUM_SLOTS_X * NUM_SLOTS_Y

#include "BlockType.h"


void inventory_init();
BlockType inventory_get_block_from_slot(int x, int y);
void inventory_set_block_in_slot(int x, int y, BlockType type);
void inventory_add_block(BlockType type);

BlockType inventory_get_block_from_hotbar_slot(int i);


#endif
