#ifndef SETTINGS_H
#define SETTINGS_H
#include "managers/InputManager.h"

struct windowSettings_t {
    char title[64];
    int width;
    int height;
    bool fullscreen;
};

struct controls_t {
    KeyCode forward;
    KeyCode backward;
    KeyCode left;
    KeyCode right;

    KeyCode jump;
    KeyCode run;
    KeyCode interact;
    KeyCode exit;
    KeyCode freecam;
    KeyCode hotbar_1;
    KeyCode hotbar_2;
    KeyCode hotbar_3;
    KeyCode hotbar_4;
    KeyCode hotbar_5;
    KeyCode hotbar_6;
    KeyCode hotbar_7;
    KeyCode hotbar_8;
    KeyCode hotbar_9;
};

struct settings_t {
    struct windowSettings_t window;
    struct controls_t controls;
};

extern struct settings_t Settings;

void settings_load();

#endif
