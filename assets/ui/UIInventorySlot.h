#ifndef UIINVENTORYSLOT_H
#define UIINVENTORYSLOT_H
#include "ui/UISprite.h"
#include "ui/UIText.h"

typedef struct {
    UISprite itemSprite;
    UIText amountText;
    int amount;
} UIInventorySlot;

void UIInventorySlot_init(UIInventorySlot* slot, const char* itemTexture, int amount, vec2 position, vec2 size);
void UIInventorySlot_set_amount(UIInventorySlot* slot, int amount);
void UIInventorySlot_set_texture(UIInventorySlot* slot, const char* itemTexture);
void UIInventorySlot_get_position(UIInventorySlot* slot, vec2 position);
void UIInventorySlot_get_size(UIInventorySlot* slot, vec2 size);
void UIInventorySlot_draw(UIInventorySlot* slot);

#endif
