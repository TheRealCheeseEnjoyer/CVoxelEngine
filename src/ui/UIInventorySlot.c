#include "../../include/ui/UIInventorySlot.h"

#include <stdio.h>

#include "Vector.h"

typedef struct {
    UISprite itemSprite;
    UIText amountText;
    int amount;
} uiinventoryslot_t;

static uiinventoryslot_t* slots = nullptr;

UIInventorySlot UIInventorySlot_init(const char *itemTexture, int amount, vec2 position, vec2 size) {
    if (slots == nullptr) {
        slots = vec_init(sizeof(uiinventoryslot_t));
    }
    uiinventoryslot_t slot;
    slot.itemSprite = UISprite_init(itemTexture, position, size);
    char amountStr[10];
    sprintf(amountStr, "%d", amount);
    slot.amountText = UIText_init(amountStr, (vec2) {position[0], position[1] + size[1] / 2}, true);
    slot.amount = amount;
    vec_append(&slots, &slot);
    return vec_size(slots) - 1;
}

void UIInventorySlot_set_amount(UIInventorySlot slotIndex, int amount) {
    char amountStr[10];
    sprintf(amountStr, "%d", amount);
    UIText_set_text(slots[slotIndex].amountText, amountStr);
}

void UIInventorySlot_set_texture(UIInventorySlot slotIndex, const char *itemTexture) {
    UISprite_set_texture(slots[slotIndex].itemSprite, itemTexture);
}

void UIInventorySlot_get_position(UIInventorySlot slotIndex, vec2 position) {
    UISprite_get_position(slots[slotIndex].itemSprite, position);
}

void UIInventorySlot_get_size(UIInventorySlot slotIndex, vec2 size) {
    UISprite_get_size(slots[slotIndex].itemSprite, size);
}

void UIInventorySlot_draw(UIInventorySlot slotIndex) {
    UISprite_draw(slots[slotIndex].itemSprite);
    UIText_draw(slots[slotIndex].amountText);
}
