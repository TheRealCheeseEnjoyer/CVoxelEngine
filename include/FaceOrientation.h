#ifndef FACEORIENTATION_H
#define FACEORIENTATION_H

typedef enum {
    FACE_TOP = 0b1,
    FACE_BOTTOM = 0b10,
    FACE_LEFT = 0b100,
    FACE_RIGHT = 0b1000,
    FACE_FRONT = 0b10000,
    FACE_BACK = 0b100000
} FaceOrientation;

#endif
