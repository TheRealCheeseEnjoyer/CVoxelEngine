#include "managers/InputManager.h"

#include <GLFW/glfw3.h>

#include "CVector/Vector.h"

#define KEY_NOT_REGISTERED (-1)
#define BUTTON_NOT_REGISTERED (-1)

typedef struct {
    KeyCode key;
    bool isPressed;
    bool wasPressed;
} KeyState;

typedef struct {
    ButtonCode button;
    bool isPressed;
    bool wasPressed;
} ButtonState;

static KeyState* KeyStates;
static ButtonState* ButtonStates;
static vec2 mousePos = {0, 0};
static vec2 mouseDelta = {0, 0};
static int mouseScrollDirection = 0;

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (yoffset > 0) {
        mouseScrollDirection = 1;
    } else if (yoffset < 0) {
        mouseScrollDirection = -1;
    }
}


int is_key_registered(KeyCode key) {
    for (int i = 0; i < vec_size(KeyStates); i++) {
        if (KeyStates[i].key == key) return i;
    }
    return KEY_NOT_REGISTERED;
}
int is_button_registered(ButtonCode button) {
    for (int i = 0; i < vec_size(ButtonStates); i++) {
        if (ButtonStates[i].button == button) return i;
    }
    return BUTTON_NOT_REGISTERED;
}

void im_init(GLFWwindow* window, KeyCode* keys, int n) {
    im_init_empty(window);

    for (int i = 0; i < n; i++) {
        im_register_key(keys[i]);
    }
}

void im_init_empty(GLFWwindow* window) {
    KeyStates = vec_init(sizeof(KeyState));
    ButtonStates = vec_init(sizeof(ButtonState));
    glfwSetScrollCallback(window, scroll_callback);
}

void im_destroy() {
    vec_free(KeyStates);
    vec_free(ButtonStates);
}

void im_register_key(KeyCode key) {
    if (is_key_registered(key) != KEY_NOT_REGISTERED) return;

    KeyState state = { key, false, false };

    vec_append(&KeyStates, &state);
}

bool im_get_key_down(KeyCode key) {
    int index = is_key_registered(key);
    if (index == KEY_NOT_REGISTERED) return false;

    KeyState state = KeyStates[index];

    return state.isPressed && !state.wasPressed;
}

bool im_get_key(KeyCode key) {
    int index = is_key_registered(key);
    if (index == KEY_NOT_REGISTERED) return false;

    KeyState state = KeyStates[index];
    return state.isPressed;
}

bool im_get_key_up(KeyCode key) {
    int index = is_key_registered(key);
    if (index == KEY_NOT_REGISTERED) return false;

    KeyState state = KeyStates[index];
    return !state.isPressed && state.wasPressed;
}

void im_get_mouse_delta(vec2 delta) {
    delta[0] = mouseDelta[0];
    delta[1] = mouseDelta[1];
}

void im_register_button(ButtonCode button) {
    if (is_button_registered(button) != BUTTON_NOT_REGISTERED) return;

    ButtonState state = { button, false, false };
    vec_append(&ButtonStates, &state);
}

bool im_get_mouse_button_down(ButtonCode button) {
    int index = is_button_registered(button);
    if (index == BUTTON_NOT_REGISTERED) return false;
    ButtonState state = ButtonStates[index];
    return state.isPressed && !state.wasPressed;
}

bool im_get_mouse_button(ButtonCode button) {
    int index = is_button_registered(button);
    if (index == BUTTON_NOT_REGISTERED) return false;
    ButtonState state = ButtonStates[index];
    return state.isPressed;
}

bool im_get_mouse_button_up(ButtonCode button) {
    int index = is_button_registered(button);
    if (index == BUTTON_NOT_REGISTERED) return false;
    ButtonState state = ButtonStates[index];
    return !state.isPressed && state.wasPressed;
}

int im_get_scroll_direction() {
    return mouseScrollDirection;
}

void im_get_mouse_position(vec2 position) {
    position[0] = mousePos[0];
    position[1] = mousePos[1];
}

void im_update_input(GLFWwindow* window) {
    for (int i = 0; i < vec_size(KeyStates); i++) {
        KeyStates[i].isPressed = glfwGetKey(window, KeyStates[i].key) == GLFW_PRESS;
    }

    for (int i = 0; i < vec_size(ButtonStates); i++) {
        ButtonStates[i].isPressed = glfwGetMouseButton(window, ButtonStates[i].button) == GLFW_PRESS;
    }

    double xPos, yPos;
    glfwGetCursorPos(window, &xPos, &yPos);
    mouseDelta[0] = xPos - mousePos[0];
    mouseDelta[1] = yPos - mousePos[1];
    mousePos[0] = xPos;
    mousePos[1] = yPos;
}

void im_reset_input() {
    for (int i = 0; i < vec_size(KeyStates); i++) {
        KeyStates[i].wasPressed = KeyStates[i].isPressed;
        KeyStates[i].isPressed = false;
    }

    for (int i = 0; i < vec_size(ButtonStates); i++) {
        ButtonStates[i].wasPressed = ButtonStates[i].isPressed;
        ButtonStates[i].isPressed = false;
    }

    mouseScrollDirection = 0;
}