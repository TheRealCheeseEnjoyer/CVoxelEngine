#ifndef SKYBOX_H
#define SKYBOX_H
#include <cglm/mat4.h>

void skybox_init(const char* name);
void skybox_draw(vec3 eyePos, mat4 projection, mat4 view);
void skybox_destroy();

#endif
