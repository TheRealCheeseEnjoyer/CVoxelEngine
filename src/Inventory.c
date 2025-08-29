#include "Inventory.h"
#include <string.h>
#include "BlockType.h"
#include "ui/UIInventory.h"

static BlockType inventoryBlocks[NUM_SLOTS];

void inventory_init() {
    memset(inventoryBlocks, 0, sizeof(BlockType) * NUM_SLOTS);
}

BlockType inventory_get_block_from_slot(int x, int y) {
    return inventoryBlocks[y * NUM_SLOTS_X + x];
}

void inventory_set_block_in_slot(int x, int y, BlockType type) {
    inventoryBlocks[y * NUM_SLOTS_X + x] = type;
    UIInventory_reload_slot(x, y, type);
}

void inventory_add_block(BlockType type) {
    if (type == 0) return;

    for (int i = 0; i < NUM_SLOTS_X; i++) {
        if (inventoryBlocks[i] == 0) {
            inventoryBlocks[i] = type;
            UIInventory_reload_slot(i, 0, type);
            return;
        }
    }

    for (int y = NUM_SLOTS_Y - 1; y >= 0; y--) {
        for (int x = 0; x < NUM_SLOTS_X; x++) {
            if (inventoryBlocks[y * NUM_SLOTS_X + x] == 0) {
                inventoryBlocks[y * NUM_SLOTS_X + x] = type;
                UIInventory_reload_slot(x, y, type);
                return;
            }
        }
    }
}
