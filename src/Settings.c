#include "../include/Settings.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SETTINGS_FILE "settings.ini"
#define HEADER_WINDOW "[window]\n"

void load_defaults(Settings* settings) {
    settings->window.width = 1920;
    settings->window.height = 1080;
    settings->window.title = "Default Name";
}

int load_window_settings(FILE* file, WindowSettings* settings) {
    char property[32], value[32];
    while (fscanf(file, "%[^:]:%[^\n]\n", property, value) == 2) {
        if (strcmp(property, "width") == 0) {
            settings->width = atoi(value);
        } else if (strcmp(property, "height") == 0) {
            settings->height = atoi(value);
        } else if (strcmp(property, "title") == 0) {
            settings->title = calloc(strlen(value) + 1, sizeof(char));
            strcpy(settings->title, value);
        }
    }
}

int settings_save(Settings* settings) {
    FILE* file = fopen(SETTINGS_FILE, "w");
    if (file == NULL) {
        // TODO LOG
        return 0;
    }

    fprintf(file, HEADER_WINDOW);
    fprintf(file, "width:%d\n", settings->window.width);
    fprintf(file, "height:%d\n", settings->window.height);
    fprintf(file, "title:%s\n", settings->window.title);
    fprintf(file, "\n");

    fclose(file);
}

int settings_load(Settings* settings) {
    load_defaults(settings);

    FILE* file = fopen(SETTINGS_FILE, "r");
    if (!file) {
        // TODO LOG
        settings_save(settings);
        return 0;
    }

    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        if (strcmp(line, HEADER_WINDOW) == 0) {
            load_window_settings(file, &settings->window);
        }
    }

    fclose(file);

    return 0;
}
