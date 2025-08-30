#ifndef SETTINGS_H
#define SETTINGS_H
#include "managers/InputManager.h"

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
    KeyCode hotbar_1;
    KeyCode hotbar_2;
    KeyCode hotbar_3;
    KeyCode hotbar_4;
    KeyCode hotbar_5;
    KeyCode hotbar_6;
    KeyCode hotbar_7;
    KeyCode hotbar_8;
    KeyCode hotbar_9;
} Controls;

typedef struct {
    WindowSettings window;
    Controls controls;
} Settings;

#endif
