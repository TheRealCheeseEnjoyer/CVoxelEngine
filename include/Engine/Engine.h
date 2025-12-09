#ifndef ENGINE_H
#define ENGINE_H

void engine_init();

bool engine_isRunning();

/// Call right before of main loop
void engine_pre_main_loop();

/// Call at the start of main loop
void engine_main_loop_start();

/// Call at the end of main loop
void engine_main_loop_end();

void engine_terminate();

#endif