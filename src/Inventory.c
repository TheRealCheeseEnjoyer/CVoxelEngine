#include "Inventory.h"
#include <string.h>
#include "BlockType.h"
#include "ui/UIHotbar.h"
#include "ui/UIInventory.h"
#include "ui/BlockStack.h"

static BlockStack inventorySlots[NUM_SLOTS];

void inventory_init() {
    memset(inventorySlots, 0, sizeof(BlockStack) * NUM_SLOTS);
}

BlockStack inventory_get_stack_from_slot(int x, int y) {
    return inventorySlots[y * NUM_SLOTS_X + x];
}

void inventory_set_stack_in_slot(int x, int y, BlockStack stack) {
    inventorySlots[y * NUM_SLOTS_X + x] = stack;
    UIInventory_reload_slot(x, y, stack);
}

void inventory_add_block(BlockStack stack) {
    if (stack.type == 0) return;
    for (int i = 0; i < NUM_SLOTS; i++) {
        if (inventorySlots[i].type == stack.type && inventorySlots[i].size < inventorySlots[i].maxSize) {
            inventorySlots[i].size++;
            UIInventory_reload_slot(i % NUM_SLOTS_X, i / NUM_SLOTS_X, inventorySlots[i]);
            return;
        }
    }

    for (int i = 0; i < NUM_SLOTS_X; i++) {
        if (inventorySlots[i].type == 0) {
            inventorySlots[i] = stack;
            UIInventory_reload_slot(i, 0, stack);
            return;
        }
    }

    for (int y = NUM_SLOTS_Y - 1; y >= 0; y--) {
        for (int x = 0; x < NUM_SLOTS_X; x++) {
            if (inventorySlots[y * NUM_SLOTS_X + x].type == 0) {
                inventorySlots[y * NUM_SLOTS_X + x] = stack;
                UIInventory_reload_slot(x, y, stack);
                return;
            }
        }
    }
}

void inventory_use_block_from_hotbar() {
    int selectedIndex = UIHotbar_get_current_index();
    if (--inventorySlots[selectedIndex].size == 0) {
        inventorySlots[selectedIndex].type = 0;
        UIInventory_reload_slot(selectedIndex, 0, inventorySlots[selectedIndex]);
    } else {
        UIInventory_reload_slot(selectedIndex, 0, inventorySlots[selectedIndex]);
    }
}
