#include "Collisions.h"
#include "../include/VoxelEngine/Block.h"
#include "ChunkManager.h"

#define X 0
#define Y 1
#define Z 2

bool collisions_ray_to_aabb(vec3 origin, vec3 direction, AABB aabb, float* distance, FaceOrientation* faceHit) {
    float tMin = -INFINITY, tMax = INFINITY;

    FaceOrientation yFace = (FaceOrientation)-1, xFace = (FaceOrientation)-1, zFace = (FaceOrientation)-1;
    if (origin[Y] > aabb.center[Y] + aabb.extent[Y])
        yFace = FACE_TOP;
    else if (origin[Y] < aabb.center[Y] - aabb.extent[Y])
        yFace = FACE_BOTTOM;

    if (origin[X] > aabb.center[X] + aabb.extent[X])
        xFace = FACE_LEFT;
    else if (origin[X] < aabb.center[X] - aabb.extent[X])
        xFace = FACE_RIGHT;

    if (origin[Z] > aabb.center[Z] + aabb.extent[Z])
        zFace = FACE_FRONT;
    else if (origin[Z] < aabb.center[Z] - aabb.extent[Z])
        zFace = FACE_BACK;

    float tx1 = (aabb.center[X] - aabb.extent[X] - origin[X]) / direction[X];
    float tx2 = (aabb.center[X] + aabb.extent[X] - origin[X]) / direction[X];

    tMin = fmax(tMin, fmin(tx1, tx2));
    tMax = fmin(tMax, fmax(tx1, tx2));

    float ty1 = (aabb.center[Y] - aabb.extent[Y] - origin[Y]) / direction[Y];
    float ty2 = (aabb.center[Y] + aabb.extent[Y] - origin[Y]) / direction[Y];

    tMin = fmax(tMin, fmin(ty1, ty2));
    tMax = fmin(tMax, fmax(ty1, ty2));

    float tz1 = (aabb.center[Z] - aabb.extent[Z] - origin[Z]) / direction[Z];
    float tz2 = (aabb.center[Z] + aabb.extent[Z] - origin[Z]) / direction[Z];

    tMin = fmax(tMin, fmin(tz1, tz2));
    tMax = fmin(tMax, fmax(tz1, tz2)),

        *distance = tMin;
    vec3 ray; // origin + direction * distance;
    glm_vec3_scale(direction, *distance, ray);
    glm_vec3_add(ray, origin, ray);

    if (ray[Y] > aabb.center[Y] - aabb.extent[Y] && ray[Y] < aabb.center[Y] + aabb.extent[Y] && ray[Z] > aabb.center[Z] - aabb.extent[Z] && ray[Z] < aabb.center[Z] + aabb.extent[Z]) {
        *faceHit = xFace;
    }
    else if (ray[Y] > aabb.center[Y] - aabb.extent[Y] && ray[Y] < aabb.center[Y] + aabb.extent[Y] && ray[X] > aabb.center[X] - aabb.extent[X] && ray[X] < aabb.center[X] + aabb.extent[X]) {
        *faceHit = zFace;
    }
    else if (ray[X] > aabb.center[X] - aabb.extent[X] && ray[X] < aabb.center[X] + aabb.extent[X] && ray[Z] > aabb.center[Z] - aabb.extent[Z] && ray[Z] < aabb.center[Z] + aabb.extent[Z]) {
        *faceHit = yFace;
    }

    return tMin <= tMax && tMax > 0;
}

bool collisions_aabb_to_aabb(AABB first, AABB second) {
    return first.center[X] - first.extent[X] <= second.center[X] + second.extent[X] &&
            first.center[X] + first.extent[X] >= second.center[X] - second.extent[X] &&
            first.center[Y] - first.extent[Y] <= second.center[Y] + second.extent[Y] &&
            first.center[Y] + first.extent[Y] >= second.center[Y] - second.extent[Y] &&
            first.center[Z] - first.extent[Z] <= second.center[Z] + second.extent[Z]&&
            first.center[Z] + first.extent[Z] >= second.center[Z] - second.extent[Z];
}

bool collisions_aabb_to_near_blocks(AABB aabb) {
    for (int x = -ceil(aabb.extent[X]); x <= ceil(aabb.extent[X]); x++) {
        for (int y = -ceil(aabb.extent[Y]); y <= ceil(aabb.extent[Y]); y++) {
            for (int z = -ceil(aabb.extent[Z]); z <= ceil(aabb.extent[Z]); z++) {
                vec3 blockPos = {round(aabb.center[X] + x), round(aabb.center[Y] + y), round(aabb.center[Z] + z)};
                BlockId block = world_get_block_at(blockPos[X], blockPos[Y], blockPos[Z]);
                if (block == BLOCK_INVALID_ID || block == BLOCK_AIR)
                    continue;

                AABB blockAABB;
                block_get_aabb(blockPos, &blockAABB);
                if (collisions_aabb_to_aabb(aabb, blockAABB)) {
                    return true;
                }
            }
        }
    }

    return false;
}
