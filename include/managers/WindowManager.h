#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "../Settings.h"

GLFWwindow* window_create();
GLFWwindow* window_get_handler();
void window_get_size(vec2 size);

#endif
