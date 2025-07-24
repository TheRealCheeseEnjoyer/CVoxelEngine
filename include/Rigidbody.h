#ifndef RIGIDBODY_H
#define RIGIDBODY_H
#include <cglm/vec3.h>

typedef unsigned int Rigidbody;

Rigidbody rigidbody_register(vec3* postion, vec3 size);
void rigidbody_update(float deltaTime);
void rigidbody_add_velocity(Rigidbody rigidbody, vec3 velocity);
void rigidbody_set_enabled(Rigidbody rigidbody, bool enabled);

#endif
