#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H
#include <cglm/types.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

typedef int KeyCode;
typedef int ButtonCode;

void im_init(GLFWwindow* window, KeyCode* keys, int n);
void im_init_empty(GLFWwindow* window);
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
int im_get_scroll_direction();

void im_update_input(GLFWwindow* window);
void im_reset_input();
#endif
