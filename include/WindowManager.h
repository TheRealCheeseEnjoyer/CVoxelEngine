#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "Settings.h"

GLFWwindow* window_create(const WindowSettings* settings);
GLFWwindow* window_get_handler();

#endif
