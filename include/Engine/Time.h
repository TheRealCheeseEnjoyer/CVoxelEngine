#ifndef TIME_H
#define TIME_H

struct time_t {
    uint64_t frameNumber;
    float elapsedTime;
    float deltaTime;
};

extern struct time_t Time;

#endif