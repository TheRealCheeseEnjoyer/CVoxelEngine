#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H
#include <cglm/types.h>
#include <GLFW/glfw3.h>

typedef int KeyCode;
typedef int ButtonCode;

void im_init(KeyCode* keys, int n);
void im_init_empty();
void im_destroy();

void im_register_key(KeyCode key);
bool im_get_key_down(KeyCode key);
bool im_get_key(KeyCode key);
bool im_get_key_up(KeyCode key);


void im_get_mouse_delta(vec2 delta);
void im_register_button(ButtonCode button);
bool im_get_mouse_button_down(ButtonCode button);
bool im_get_mouse_button(ButtonCode button);
bool im_get_mouse_button_up(ButtonCode button);

void im_update_input(GLFWwindow* window);
void im_reset_input();
#endif
