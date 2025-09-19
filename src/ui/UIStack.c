#include "ui/UIStack.h"
#include "ui/UIText.h"
#include "ui/UISprite.h"

#include <stdio.h>

#include "Vector.h"

typedef struct {
    UISprite itemSprite;
    UIText amountText;
    int amount;
} uiinventoryslot_t;

static uiinventoryslot_t* slots = nullptr;

UIStack UIStack_init(const char *itemTexture, int amount, vec2 position, vec2 size) {
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

void UIStack_set_amount(UIStack slotIndex, int amount) {
    char amountStr[10];
    sprintf(amountStr, "%d", amount);
    UIText_set_text(slots[slotIndex].amountText, amountStr);
}

void UIStack_set_stack(UIStack slotIndex, BlockStack stack) {
    UISprite_set_texture(slots[slotIndex].itemSprite, blocktype_to_texture_path(stack.type));
    char amountStr[10];
    sprintf(amountStr, "%d", stack.size);
    UIText_set_text(slots[slotIndex].amountText, amountStr);
    if (stack.size == stack.maxSize)
        UIText_set_color(slots[slotIndex].amountText, (vec3){255, 0, 0});
    else
        UIText_set_color(slots[slotIndex].amountText, (vec3){0, 0, 0});
}

void UIStack_set_texture(UIStack slotIndex, const char *itemTexture) {
    UISprite_set_texture(slots[slotIndex].itemSprite, itemTexture);
}

void UIStack_set_position(UIStack slotIndex, vec2 position) {
    UISprite_set_position(slots[slotIndex].itemSprite, position);
    vec2 size;
    UISprite_get_size(slots[slotIndex].itemSprite, size);
    UIText_set_position(slots[slotIndex].amountText, (vec2){position[0], position[1] + size[1] / 2});
}

void UIStack_get_position(UIStack slotIndex, vec2 position) {
    UISprite_get_position(slots[slotIndex].itemSprite, position);
}

void UIStack_get_size(UIStack slotIndex, vec2 size) {
    UISprite_get_size(slots[slotIndex].itemSprite, size);
}

void UIStack_draw(UIStack slotIndex) {
    UISprite_draw(slots[slotIndex].itemSprite);
    UIText_draw(slots[slotIndex].amountText);
}
