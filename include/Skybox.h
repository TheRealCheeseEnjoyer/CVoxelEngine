#ifndef SKYBOX_H
#define SKYBOX_H
#include <cglm/mat4.h>

void skybox_init(const char* name);
void skybox_draw(void);
void skybox_destroy();

#endif
