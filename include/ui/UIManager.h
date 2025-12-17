#ifndef UIMANAGER_H
#define UIMANAGER_H
#include "UISprite.h"

void UIManager_init();
void UIManager_get_ortho_matrix(mat4 orthoMatrix);
int UIManager_get_active_texture_id();

void UIManager_register_interactable(UISprite* sprite);
int UIManager_check_hovered(UISprite* sprite, int size);


void UIManager_draw();

#endif
