#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H
#include "Settings.h"

Settings *settings_manager_load();

Settings* settings_manager_get_all();
Controls* settings_manager_get_controls();
WindowSettings* settings_manager_get_window_settings();

#endif
