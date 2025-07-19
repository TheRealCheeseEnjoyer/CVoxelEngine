#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H
#include <GLFW/glfw3.h>

typedef int KeyCode;

void im_init(KeyCode* keys, int n);
void im_init_empty();
void im_destroy();

void im_register_key(KeyCode key);
bool im_get_key_down(KeyCode key);
bool im_get_key(KeyCode key);
bool im_get_key_up(KeyCode key);

void im_update_input(GLFWwindow* window);
void im_reset_input();
#endif
