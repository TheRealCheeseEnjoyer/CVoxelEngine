#ifndef TIME_H
#define TIME_H

struct time_t {
    float elapsedTime;
    float deltaTime;
};

extern struct time_t Time;

#endif