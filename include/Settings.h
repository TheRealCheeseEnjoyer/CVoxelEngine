#ifndef SETTINGS_H
#define SETTINGS_H

typedef struct {
    char* title;
    int width;
    int height;
} WindowSettings;

typedef struct {
    WindowSettings window;
} Settings;

int settings_load(Settings* settings);

#endif
