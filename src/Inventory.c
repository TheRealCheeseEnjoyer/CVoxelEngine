#include "Inventory.h"
#include <string.h>
#include "BlockType.h"

static BlockType inventoryBlocks[NUM_SLOTS];

void inventory_init() {
    memset(inventoryBlocks, 0, sizeof(BlockType) * NUM_SLOTS);
}

BlockType inventory_get_block_from_slot(int x, int y) {
    return inventoryBlocks[y * NUM_SLOTS_X + x];
}

void inventory_set_block_in_slot(int x, int y, BlockType type) {
    inventoryBlocks[y * NUM_SLOTS_X + x] = type;
}
