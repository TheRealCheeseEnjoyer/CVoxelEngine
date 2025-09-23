#include "managers/SettingsManager.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Settings settings;

#define SETTINGS_FILE "settings.ini"
#define HEADER_WINDOW "[window]\n"
#define HEADER_CONTROLS "[controls]\n"

void load_defaults() {
    settings.window.width = 1920;
    settings.window.height = 1080;
    settings.window.title = "Default Name";

    settings.controls.forward = GLFW_KEY_W;
    settings.controls.backward = GLFW_KEY_S;
    settings.controls.left = GLFW_KEY_A;
    settings.controls.right = GLFW_KEY_D;
    settings.controls.exit = GLFW_KEY_ESCAPE;
    settings.controls.interact = GLFW_KEY_E;
    settings.controls.jump = GLFW_KEY_SPACE;
    settings.controls.run = GLFW_KEY_LEFT_SHIFT;
}

int load_window_settings(FILE* file) {
    char property[32], value[32], line[66];
    while (fgets(line, sizeof(line), file) != NULL && line[0] != '\n') {
        sscanf(line, "%[^:]:%[^\n]\n", property, value);
        if (strcmp(property, "width") == 0) {
            settings.window.width = atoi(value);
        } else if (strcmp(property, "height") == 0) {
            settings.window.height = atoi(value);
        } else if (strcmp(property, "title") == 0) {
            settings.window.title = calloc(strlen(value) + 1, sizeof(char));
            strcpy(settings.window.title, value);
        } else if (strcmp(property, "fullscreen") == 0) {
            settings.window.fullscreen = strcmp(value, "true") == 0;
        }
    }
    return 1;
}

int value_to_keycode(char* str) {
    if (strlen(str) == 1) {
        return toupper(str[0]);
    }

    if (strcmp(str, "space") == 0) return GLFW_KEY_SPACE;
    if (strcmp(str, "enter") == 0) return GLFW_KEY_ENTER;
    if (strcmp(str, "esc") == 0) return GLFW_KEY_ESCAPE;
    if (strcmp(str, "lshift") == 0) return GLFW_KEY_LEFT_SHIFT;
    return -1;
}
void keycode_to_value(int code, char* str) {
    if (code == GLFW_KEY_SPACE) { strcpy(str, "space"); return; }
    if (code == GLFW_KEY_ENTER) { strcpy(str, "enter"); return; }
    if (code == GLFW_KEY_ESCAPE) { strcpy(str, "esc"); return; }
    if (code == GLFW_KEY_LEFT_SHIFT) { strcpy(str, "lshift"); return; }

    str[0] = (char)tolower(code);
    str[1] = '\0';
}

int load_controls(FILE* file) {
    char property[32], value[32], line[66];
    while (fgets(line, sizeof(line), file) != NULL && line[0] != '\n') {
        sscanf(line, "%[^:]:%[^\n]\n", property, value);
        int key = value_to_keycode(value);
        if (key == -1) continue;
        if (strcmp(property, "forward") == 0 ) settings.controls.forward = key;
        else if (strcmp(property, "backward") == 0 ) settings.controls.backward = key;
        else if (strcmp(property, "left") == 0 ) settings.controls.left = key;
        else if (strcmp(property, "right") == 0 ) settings.controls.right = key;
        else if (strcmp(property, "jump") == 0 ) settings.controls.jump = key;
        else if (strcmp(property, "run") == 0 ) settings.controls.run = key;
        else if (strcmp(property, "interact") == 0) settings.controls.interact = key;
        else if (strcmp(property, "exit") == 0) settings.controls.exit = key;
        else if (strcmp(property, "freecam") == 0) settings.controls.freecam = key;
        else if (strcmp(property, "hotbar_1") == 0) settings.controls.hotbar_1 = key;
        else if (strcmp(property, "hotbar_2") == 0) settings.controls.hotbar_2 = key;
        else if (strcmp(property, "hotbar_3") == 0) settings.controls.hotbar_3 = key;
        else if (strcmp(property, "hotbar_4") == 0) settings.controls.hotbar_4 = key;
        else if (strcmp(property, "hotbar_5") == 0) settings.controls.hotbar_5 = key;
        else if (strcmp(property, "hotbar_6") == 0) settings.controls.hotbar_6 = key;
        else if (strcmp(property, "hotbar_7") == 0) settings.controls.hotbar_7 = key;
        else if (strcmp(property, "hotbar_8") == 0) settings.controls.hotbar_8 = key;
        else if (strcmp(property, "hotbar_9") == 0) settings.controls.hotbar_9 = key;
    }
    return 1;
}

int settings_save() {
    FILE* file = fopen(SETTINGS_FILE, "w");
    if (file == NULL) {
        // TODO LOG
        return 0;
    }

    fprintf(file, HEADER_WINDOW);
    fprintf(file, "width:%d\n", settings.window.width);
    fprintf(file, "height:%d\n", settings.window.height);
    fprintf(file, "title:%s\n", settings.window.title);
    fprintf(file, "\n");

    char str[16];
    fprintf(file, HEADER_CONTROLS);
    keycode_to_value(settings.controls.forward, str);
    fprintf(file, "forward:%s\n", str);
    keycode_to_value(settings.controls.backward, str);
    fprintf(file, "backward:%s\n", str);
    keycode_to_value(settings.controls.left, str);
    fprintf(file, "left:%s\n", str);
    keycode_to_value(settings.controls.right, str);
    fprintf(file, "right:%s\n", str);

    keycode_to_value(settings.controls.interact, str);
    fprintf(file, "interact:%s\n", str);
    keycode_to_value(settings.controls.jump, str);
    fprintf(file, "jump:%s\n", str);
    keycode_to_value(settings.controls.run, str);
    fprintf(file, "run:%s\n", str);
    keycode_to_value(settings.controls.exit, str);
    fprintf(file, "exit:%s\n", str);

    fclose(file);
    return 1;
}

Settings* settings_manager_load() {
    load_defaults();

    FILE* file = fopen(SETTINGS_FILE, "r");
    if (!file) {
        // TODO LOG
        settings_save();
        return nullptr;
    }

    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        if (strcmp(line, HEADER_WINDOW) == 0) {
            load_window_settings(file);
        } else if (strcmp(line, HEADER_CONTROLS) == 0) {
            load_controls(file);
        }
    }

    fclose(file);
    return &settings;
}

Settings* settings_manager_get_all() {
    return &settings;
}

Controls * settings_manager_get_controls() {
    return  &settings.controls;
}

WindowSettings * settings_manager_get_window_settings() {
    return &settings.window;
}
