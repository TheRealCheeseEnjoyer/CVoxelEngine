#include "UIInventorySlot.h"

#include <stdio.h>

void UIInventorySlot_init(UIInventorySlot *slot, const char *itemTexture, int amount, vec2 position, vec2 size) {
    UISprite_init(&slot->itemSprite, itemTexture, position, size, true);
    char amountStr[10];
    sprintf(amountStr, "%d", amount);
    UIText_init(&slot->amountText, amountStr, (vec2) {position[0], position[1] + size[1] / 2}, true);
    slot->amount = amount;
}

void UIInventorySlot_set_amount(UIInventorySlot *slot, int amount) {
    char amountStr[10];
    sprintf(amountStr, "%d", amount);
    UIText_set_text(&slot->amountText, amountStr);
}

void UIInventorySlot_set_texture(UIInventorySlot *slot, const char *itemTexture) {
    UISprite_set_texture(&slot->itemSprite, itemTexture);
}

void UIInventorySlot_get_position(UIInventorySlot *slot, vec2 position) {
    UISprite_get_position(&slot->itemSprite, position);
}

void UIInventorySlot_get_size(UIInventorySlot *slot, vec2 size) {
    UISprite_get_size(&slot->itemSprite, size);
}

void UIInventorySlot_draw(UIInventorySlot *slot) {
    UISprite_draw(&slot->itemSprite);
    UIText_draw(&slot->amountText);
}
