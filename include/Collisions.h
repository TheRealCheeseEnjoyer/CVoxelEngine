#ifndef COLLISIONS_H
#define COLLISIONS_H

#include "AABB.h"
#include "FaceOrientation.h"

bool collisions_ray_to_aabb(vec3 origin, vec3 direction, AABB aabb, float* distance, FaceOrientation* faceHit);
bool collisions_aabb_to_aabb(AABB first, AABB second);
bool collisions_aabb_to_near_blocks(AABB aabb);

#endif
