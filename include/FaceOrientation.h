#ifndef FACEORIENTATION_H
#define FACEORIENTATION_H

typedef enum {
    FACE_TOP,
    FACE_BOTTOM,
    FACE_LEFT,
    FACE_RIGHT,
    FACE_FRONT,
    FACE_BACK,
    FACE_NUM,
    FACE_INVALID,
    FACE_TOP_MASK = 0b1,
    FACE_BOTTOM_MASK = 0b10,
    FACE_LEFT_MASK = 0b100,
    FACE_RIGHT_MASK = 0b1000,
    FACE_FRONT_MASK = 0b10000,
    FACE_BACK_MASK = 0b100000,
} FaceOrientation;

static FaceOrientation FaceOrientation_to_mask(FaceOrientation orientation) {
    switch (orientation) {
        case FACE_TOP:
            return FACE_TOP_MASK;
        case FACE_BOTTOM:
            return FACE_BOTTOM_MASK;
        case FACE_LEFT:
            return FACE_LEFT_MASK;
        case FACE_RIGHT:
            return FACE_RIGHT_MASK;
        case FACE_FRONT:
            return FACE_FRONT_MASK;
        case FACE_BACK:
            return FACE_BACK_MASK;
    }

    return FACE_INVALID;
}

#endif
