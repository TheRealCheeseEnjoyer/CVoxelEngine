#include "../include/InputManager.h"

#include <GLFW/glfw3.h>

#include "../include/Vector.h"

#define KEY_NOT_REGISTERED (-1)

typedef struct {
    KeyCode key;
    bool isPressed;
    bool wasPressed;
} KeyState;

Vector KeyStates;

int is_key_registered(KeyCode key) {
    for (int i = 0; i < vec_size(KeyStates); i++) {
        if (vec_get_as(KeyState, KeyStates, i).key == key) return i;
    }

    return KEY_NOT_REGISTERED;
}

void im_init() {
    KeyStates = vec_init(sizeof(KeyState));
}

void im_destroy() {
    vec_free(KeyStates);
}

void im_register_key(KeyCode key) {
    if (is_key_registered(key) != KEY_NOT_REGISTERED) return;

    KeyState state = { key, false, false };
    vec_append(KeyStates, &state);
}

bool im_get_key_down(KeyCode key) {
    int index = is_key_registered(key);
    if (index == KEY_NOT_REGISTERED) return false;

    KeyState state = vec_get_as(KeyState, KeyStates, index);

    return state.isPressed && !state.wasPressed;
}

bool im_get_key(KeyCode key) {
    int index = is_key_registered(key);
    if (index == KEY_NOT_REGISTERED) return false;

    KeyState state = vec_get_as(KeyState, KeyStates, index);
    return state.isPressed;
}

bool im_get_key_up(KeyCode key) {
    int index = is_key_registered(key);
    if (index == KEY_NOT_REGISTERED) return false;

    KeyState state = vec_get_as(KeyState, KeyStates, index);
    return !state.isPressed && state.wasPressed;
}

void im_update_input(GLFWwindow* window) {
    for (int i = 0; i < vec_size(KeyStates); i++) {
        KeyState* state = vec_get(KeyStates, i);
        state->isPressed = glfwGetKey(window, state->key) == GLFW_PRESS;
    }
}

void im_reset_input() {
    for (int i = 0; i < vec_size(KeyStates); i++) {
        KeyState* state = vec_get(KeyStates, i);
        state->wasPressed = state->isPressed;
        state->isPressed = false;
    }
}