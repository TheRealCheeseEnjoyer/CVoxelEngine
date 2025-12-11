#ifndef PLAYER_H
#define PLAYER_H

#include "Settings.h"
#include "Shader.h"

typedef struct player_t* Player;

void player_init();
void player_draw(void);
void player_update(void);
void player_get_position(vec3 position);
void player_get_view(mat4 outView);
void player_get_projection(mat4 outProjection);
void player_get_eye_position(vec3 position);

#endif
