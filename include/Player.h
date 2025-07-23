#ifndef PLAYER_H
#define PLAYER_H

#include "Settings.h"

typedef struct player_t* Player;

void player_init(Controls* controls);
void player_update(float deltaTime);
void player_get_view_matrix(mat4 outView);
void player_eye_position(vec3 position);

#endif
