#ifndef COLLISIONS_H
#define COLLISIONS_H

#include "AABB.h"
#include "FaceOrientation.h"

bool collisions_ray_to_aabb(vec3 origin, vec3 direction, AABB aabb, float* distance, FaceOrientation* faceHit);

#endif
