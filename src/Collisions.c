#include "../include/Collisions.h"

#define X 0
#define Y 1
#define Z 2

bool collisions_ray_to_aabb(vec3 origin, vec3 direction, AABB aabb, float *distance, FaceOrientation *faceHit) {
    float tMin = -INFINITY, tMax = INFINITY;

    FaceOrientation yFace = (FaceOrientation)-1, xFace = (FaceOrientation)-1, zFace = (FaceOrientation)-1;
    if (origin[Y] > aabb.max[Y])
        yFace = FACE_TOP;
    else if (origin[Y] < aabb.min[Y])
        yFace = FACE_BOTTOM;

    if (origin[X] > aabb.max[X])
        xFace = FACE_LEFT;
    else if (origin[X] < aabb.min[X])
        xFace = FACE_RIGHT;

    if (origin[Z] > aabb.max[Z])
        zFace = FACE_FRONT;
    else if (origin[Z] < aabb.min[Z])
        zFace = FACE_BACK;

    float tx1 = (aabb.min[X] - origin[X]) / direction[X];
    float tx2 = (aabb.max[X] - origin[X]) / direction[X];

    tMin = fmax(tMin, fmin(tx1, tx2));
    tMax = fmin(tMax, fmax(tx1, tx2));

    float ty1 = (aabb.min[Y] - origin[Y]) / direction[Y];
    float ty2 = (aabb.max[Y] - origin[Y]) / direction[Y];

    tMin = fmax(tMin, fmin(ty1, ty2));
    tMax = fmin(tMax, fmax(ty1, ty2));

    float tz1 = (aabb.min[Z] - origin[Z]) / direction[Z];
    float tz2 = (aabb.max[Z] - origin[Z]) / direction[Z];

    tMin = fmax(tMin, fmin(tz1, tz2));
    tMax = fmin(tMax, fmax(tz1, tz2)),

    *distance = tMin;
    vec3 ray;// origin + direction * distance;
    glm_vec3_scale(direction, *distance, ray);
    glm_vec3_add(ray, origin, ray);

    if (ray[Y] > aabb.min[Y] && ray[Y] < aabb.max[Y] && ray[Z] > aabb.min[Z] && ray[Z] < aabb.max[Z]) {
        *faceHit = xFace;
    } else if (ray[Y] > aabb.min[Y] && ray[Y] < aabb.max[Y] && ray[X] > aabb.min[X] && ray[X] < aabb.max[X]) {
        *faceHit = zFace;
    } else if (ray[X] > aabb.min[X] && ray[X] < aabb.max[X] && ray[Z] > aabb.min[Z] && ray[Z] < aabb.max[Z]) {
        *faceHit = yFace;
    }

    return tMin <= tMax && tMax > 0;
}

bool collisions_aabb_to_aabb(AABB first, AABB second) {
    return  first.min[X] <= second.max[X] &&
            first.max[X] >= second.min[X] &&
            first.min[Y] <= second.max[Y] &&
            first.max[Y] >= second.min[Y] &&
            first.min[Z] <= second.max[Z] &&
            first.max[Z] >= second.min[Z];
}
