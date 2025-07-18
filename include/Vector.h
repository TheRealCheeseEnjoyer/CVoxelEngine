#ifndef VECTOR_H
#define VECTOR_H
#include <stddef.h>

#define VEC_DOES_NOT_CONTAIN -1

typedef struct vector_t* Vector;

Vector vec_init(size_t typeSize);
Vector vec_init_c(size_t typeSize, size_t capacity);

void vec_resize(Vector v);
void* vec_append(Vector v, void* item);
void* vec_get(Vector v, size_t index);
void vec_remove(Vector v, size_t index);
void vec_clear(Vector v);

size_t vec_size(Vector v);
size_t vec_capacity(Vector v);

void vec_free(Vector v);

#define vec_get_as(type, vector, index) (*(type*)vec_get(vector, index))

#endif
