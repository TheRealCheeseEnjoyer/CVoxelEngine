#ifndef VECTOR_H
#define VECTOR_H
#include <stddef.h>

/// @param itemSize size in bytes of the items
/// @return pointer to vector
void* vec_init(size_t itemSize);

/// @param itemSize size in bytes of the items
/// @param capacity initial capacity
/// @return pointer to vector
void* vec_init_cap(size_t itemSize, size_t capacity);

/// @param vec vector
/// @param item item to copy into the vector
/// @return 0 on failure (e.g. out of memory for reallocations), 1 on success
int vec_append(void* vec, void* item);

size_t vec_capacity(void* vec);
size_t vec_size(void* vec);

void vec_clear(void* vec);

void vec_free(void* mem);

#endif