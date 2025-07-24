#ifndef SETTINGS_H
#define SETTINGS_H
#include "InputManager.h"

typedef struct {
    char* title;
    int width;
    int height;
} WindowSettings;

typedef struct {
    KeyCode forward;
    KeyCode backward;
    KeyCode left;
    KeyCode right;

    KeyCode jump;
    KeyCode run;
    KeyCode interact;
    KeyCode exit;
    KeyCode freecam;
} Controls;

typedef struct {
    WindowSettings window;
    Controls controls;
} Settings;

int settings_load(Settings* settings);

#endif
