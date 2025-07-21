#ifndef SHADER_H
#define SHADER_H

#include <cglm/mat4.h>

typedef unsigned int Shader;

Shader shader_create(const char* vertexFilePath, const char* fragmentFilePath);
void shader_use(Shader shader);
void shader_set_mat4(Shader shader, const char* name, const mat4* matrix);
void shader_set_int(Shader shader, const char* name, int value);
void shader_delete(Shader shader);
#endif