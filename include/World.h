#ifndef WORLD_H
#define WORLD_H

#include <cglm/vec3.h>

#define WORLD_SIZE_X 10
#define WORLD_SIZE_Y 1
#define WORLD_SIZE_Z 10

void world_init(vec3 initialPosition);
void world_draw(mat4 projection, mat4 view);

void world_destroy();

#endif
