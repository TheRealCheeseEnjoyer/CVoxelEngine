#include "Rigidbody.h"

#include <string.h>

#include "AABB.h"
#include "Collisions.h"
#include "Constants.h"
#include "../libs/Vector.h"

constexpr vec3 Gravity = {0, -15.f, 0};

struct rigidbody_t {
    vec3* position;
    vec3 velocity;
    vec3 size;
    bool enabled;
};

struct rigidbody_t* rigidbodies;

Rigidbody rigidbody_register(vec3* position, vec3 size) {
    if (!rigidbodies)
        rigidbodies = vec_init(sizeof(struct rigidbody_t));
    struct rigidbody_t rigidbody;
    rigidbody.position = position;
    memcpy(rigidbody.size, size, sizeof(vec3));
    memset(rigidbody.velocity, 0, sizeof(vec3));
    rigidbody.enabled = true;
    vec_append(&rigidbodies, &rigidbody);
    return vec_size(rigidbodies) - 1;
}

void rigidbody_update(float deltaTime) {
    for (int i = 0; i < vec_size(rigidbodies); i++) {
        if (!rigidbodies[i].enabled) continue;
        vec3 oldVelocity = {rigidbodies[i].velocity[X], rigidbodies[i].velocity[Y], rigidbodies[i].velocity[Z]};
        vec3 oldPosition = {(*rigidbodies[i].position)[X], (*rigidbodies[i].position)[Y], (*rigidbodies[i].position)[Z]};

        glm_vec3_muladds(Gravity, deltaTime, oldVelocity);
        glm_vec3_muladds(oldVelocity, deltaTime, oldPosition);
        AABB aabb;
        vec3 halfSize;
        glm_vec3_divs(rigidbodies[i].size, 2, halfSize);
        glm_vec3_add(oldPosition, halfSize, aabb.max);
        glm_vec3_sub(oldPosition, halfSize, aabb.min);
        if (collisions_aabb_to_near_blocks(aabb)) {
            memset(rigidbodies[i].velocity, 0, sizeof(vec3));
            continue;
        }

        memcpy(rigidbodies[i].velocity, oldVelocity, sizeof(vec3));
        memcpy(*rigidbodies[i].position, oldPosition, sizeof(vec3));
    }
}

void rigidbody_add_velocity(Rigidbody rigidbody, vec3 velocity) {
    glm_vec3_add(rigidbodies[rigidbody].velocity, velocity, rigidbodies[rigidbody].velocity);
}

void rigidbody_set_enabled(Rigidbody rigidbody, bool enabled) {
    rigidbodies[rigidbody].enabled = enabled;
}

void rigidbody_get_velocity(Rigidbody rigidbody, vec3 velocity) {
    memcpy(velocity, rigidbodies[rigidbody].velocity, sizeof(vec3));
}

