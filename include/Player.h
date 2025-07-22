#ifndef PLAYER_H
#define PLAYER_H
#include <cglm/mat4.h>

#include "Settings.h"

typedef struct player_t* Player;

Player player_init(Controls* controls);
void player_update(Player player, float deltaTime);
void player_get_view_matrix(Player player, mat4 outView);
void player_eye_position(Player player, vec3 position);
void player_free(Player player);

#endif
