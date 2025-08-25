#pragma once

#include <stdint.h>

/* Initial size of hashmap array. Must be a power of 2. */
#define HASHMAP_BASE_SIZE 8

/* From CPython: calculate usable fraction of hash table size (2/3) */
#define USABLE_FRACTION(n) (((n) << 1)/3)
/* Estimate size for resize based on current length and table capacity */
#define ESTIMATE_SIZE(hm) (hm->length*2 + (hm->capacity >> 1))

/* Error codes for hashmap operations */
#define E_HASHMAP_KEY_NOT_FOUND 1
#define E_HASHMAP_FULL 2
#define E_HASHMAP_CANNOT_SET_VALUE 3
#define E_HASHMAP_CANNOT_RESIZE 4
#define E_ALLOC 5

typedef struct {
    char *key;      /* dynamically allocated string, owned by hashmap */
    int value;      /* integer associated with the key                */
    int is_deleted; /* deletion flag                                  */
    uint32_t hash;  /* hash value of the key                          */
} hashmap_item;

typedef struct {
    uint32_t length;      /* number of active (non-deleted) items */
    uint32_t capacity;    /* number of slots in the items array   */
    hashmap_item **items; /* array of hashmap_item pointers       */
} hashmap;

hashmap *hashmap_init();
void hashmap_free(hashmap *hm);
int hashmap_set(hashmap*, char *key, int value);
hashmap_item *hashmap_get(hashmap *hm, char *key);
int hashmap_delete(hashmap *hm, char *key);

unsigned long hash(char *s);
