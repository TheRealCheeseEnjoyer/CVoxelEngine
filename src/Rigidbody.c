#include "Rigidbody.h"

#include <string.h>

#include "AABB.h"
#include "Collisions.h"
#include "Engine/Time.h"
#include "libs/Vector.h"

static vec3 Gravity = {0, -30.f, 0};

struct rigidbody_t {
    vec3* position;
    vec3 velocity;
    vec3 size;
    bool enabled;
};

struct rigidbody_t* rigidbodies = nullptr;

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

void rigidbody_update() {
    for (int i = 0; i < vec_size(rigidbodies); i++) {
        if (!rigidbodies[i].enabled) continue;

        vec3 oldVelocity = {rigidbodies[i].velocity[0], rigidbodies[i].velocity[1], rigidbodies[i].velocity[2]};
        vec3 oldPosition = {
            (*rigidbodies[i].position)[0], (*rigidbodies[i].position)[1], (*rigidbodies[i].position)[2]
        };

        glm_vec3_muladds(Gravity, Time.deltaTime, oldVelocity);
        glm_vec3_muladds(oldVelocity, Time.deltaTime, oldPosition);

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

void rigidbody_add_velocity(const Rigidbody rigidbody, vec3 velocity) {
    glm_vec3_add(rigidbodies[rigidbody].velocity, velocity, rigidbodies[rigidbody].velocity);
}

void rigidbody_set_enabled(const Rigidbody rigidbody, const bool enabled) {
    rigidbodies[rigidbody].enabled = enabled;
}

void rigidbody_get_velocity(const Rigidbody rigidbody, vec3 velocity) {
    memcpy(velocity, rigidbodies[rigidbody].velocity, sizeof(vec3));
}

void rigidbody_set_velocity(Rigidbody rigidbody, vec3 velocity) {
    rigidbodies[rigidbody].velocity[0] = velocity[0];
    rigidbodies[rigidbody].velocity[1] = velocity[1];
    rigidbodies[rigidbody].velocity[2] = velocity[2];
}
