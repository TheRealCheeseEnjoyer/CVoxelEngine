#include "../include/Rigidbody.h"

#include <stdio.h>
#include <string.h>

#include "../include/AABB.h"
#include "../include/Collisions.h"
#include "../include/Constants.h"
#include "../include/Vector.h"

constexpr vec3 Gravity = {0, -15.f, 0};

struct rigidbody_t {
    vec3* position;
    vec3 velocity;
    vec3 size;
    bool enabled;
};

Vector rigidbodies;

Rigidbody rigidbody_register(vec3* position, vec3 size) {
    if (!rigidbodies)
        rigidbodies = vec_init(sizeof(struct rigidbody_t));
    struct rigidbody_t rigidbody;
    rigidbody.position = position;
    memcpy(rigidbody.size, size, sizeof(vec3));
    memset(rigidbody.velocity, 0, sizeof(vec3));
    rigidbody.enabled = true;
    vec_append(rigidbodies, &rigidbody);
    return vec_size(rigidbodies) - 1;
}

void rigidbody_update(float deltaTime) {
    for (int i = 0; i < vec_size(rigidbodies); i++) {
        struct rigidbody_t* rigidbody = vec_get(rigidbodies, i);
        if (!rigidbody->enabled) continue;
        vec3 oldVelocity = {rigidbody->velocity[X], rigidbody->velocity[Y], rigidbody->velocity[Z]};
        vec3 oldPosition = {(*rigidbody->position)[X], (*rigidbody->position)[Y], (*rigidbody->position)[Z]};

        glm_vec3_muladds(Gravity, deltaTime, oldVelocity);
        glm_vec3_muladds(oldVelocity, deltaTime, oldPosition);
        AABB aabb;
        vec3 halfSize;
        glm_vec3_divs(rigidbody->size, 2, halfSize);
        glm_vec3_add(oldPosition, halfSize, aabb.max);
        glm_vec3_sub(oldPosition, halfSize, aabb.min);
        if (collisions_aabb_to_near_blocks(aabb)) {
            memset(rigidbody->velocity, 0, sizeof(vec3));
            continue;
        }

        memcpy(rigidbody->velocity, oldVelocity, sizeof(vec3));
        memcpy(*rigidbody->position, oldPosition, sizeof(vec3));
    }
}

void rigidbody_add_velocity(Rigidbody rigidbody, vec3 velocity) {
    struct rigidbody_t* rb = vec_get(rigidbodies, rigidbody);
    glm_vec3_add(rb->velocity, velocity, rb->velocity);
}

void rigidbody_set_enabled(Rigidbody rigidbody, bool enabled) {
    struct rigidbody_t* rb = vec_get(rigidbodies, rigidbody);
    rb->enabled = enabled;
}

void rigidbody_get_velocity(Rigidbody rigidbody, vec3 velocity) {
    struct rigidbody_t* rb = vec_get(rigidbodies, rigidbody);
    memcpy(velocity, rb->velocity, sizeof(vec3));
}

