#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashmap.h"

static int hashmap_ensure_size(hashmap *hm);
static int hashmap_resize(hashmap *hm);
static int32_t _hashmap_find_empty_index(hashmap *hm, uint32_t hash);
static int32_t _hashmap_find_index(hashmap *hm, char *key, uint32_t hash);

/**
 * Allocate and initialize a new hashmap with default capacity
 *
 * @return  Pointer to new hashmap, or NULL if allocation fails
 */
hashmap *hashmap_init()
{
    uint32_t capacity = HASHMAP_BASE_SIZE;
    hashmap *hm = calloc(1, sizeof(hashmap));
    if (hm == NULL) {
        return NULL;
    }

    hm->items = calloc(capacity, sizeof(hashmap_item*));
    if (hm->items == NULL) {
        free(hm);
        return NULL;
    }

    hm->capacity = capacity;
    hm->length = 0;
    return hm;
}

/**
 * Free all memory used by a hashmap
 * @param hm  Hashmap to free
 */
void hashmap_free(hashmap *hm)
{
    if (hm == NULL) {
        return;
    }

    hashmap_item *item;
    for (uint32_t i = 0; i < hm->capacity; i++) {
        item = hm->items[i];
        if (item != NULL && item->key != NULL) {
            free(item->key);
        }
        free(item);
    }
    free(hm->items);
    hm->items = NULL;
    hm->length = 0;
    free(hm);
}

/**
 * Set a key-value pair in the hashmap
 * @param hm  Hashmap to update
 * @param key  String key to set
 * @param value  Integer value to store
 *
 * If the key already exists, its value is updated.
 * If the key doesn't exist, a new entry is created.
 * The key string is copied, so the original can be freely modified
 * or deallocated after this call.
 *
 * @return  0 on success, negative error code on failure
 */
int hashmap_set(hashmap *hm, char *key, int value)
{
    int result;
    int is_new_key = 0;

    if (hm == NULL || key == NULL) {
        return -E_HASHMAP_CANNOT_SET_VALUE;
    }

    /* Ensure we have enough space */
    if ((result = hashmap_ensure_size(hm)) != 0) {
        return result;
    }

    /* Find the key or an empty slot */
    unsigned long h = hash(key);
    int32_t index = _hashmap_find_index(hm, key, h);
    if (index == -E_HASHMAP_KEY_NOT_FOUND) {
        is_new_key = 1;
        index = _hashmap_find_empty_index(hm, h);
    }
    if (index == -E_HASHMAP_KEY_NOT_FOUND) {
        return -E_HASHMAP_FULL;
    }

    if (is_new_key) {
        /* For new keys, allocate memory for the key copy */
        char *key_copy = strdup(key);
        if (key_copy == NULL) {
            return -E_HASHMAP_CANNOT_SET_VALUE;
        }

        hashmap_item *new_item = calloc(1, sizeof(hashmap_item));
        if (new_item == NULL) {
            free(key_copy);
            return -E_HASHMAP_CANNOT_SET_VALUE;
        }

        new_item->key = key_copy;
        new_item->value = value;
        new_item->is_deleted = 0;
        new_item->hash = h;

        /* Free the previous item if it exists */
        hashmap_item *old_item = hm->items[index];
        if (old_item != NULL) {
            /* If item is marked as deleted, free its resources */
            if (old_item->is_deleted && old_item->key != NULL) {
                free(old_item->key);
            }
            free(old_item);
        }

        hm->items[index] = new_item;
    } else {
        /* For existing keys, just update the value */
        hashmap_item *item = hm->items[index];
        item->value = value;
    }

    if (is_new_key) {
        hm->length++;
    }

    return 0;
}

/**
 * Resize hashmap if it's too full, i.e. the load factor is above
 * the threshold.
 * @param hm  Hashmap to check
 *
 * @return  0 on success, error code on failure
 */
static int hashmap_ensure_size(hashmap *hm)
{
    int result;
    if (hm->length >= USABLE_FRACTION(hm->capacity)) {
        result = hashmap_resize(hm);
        if (result != 0) {
            return result;
        }
    }
    return 0;
}

/**
 * Resize hashmap to a larger capacity.
 * @param hm  Hashmap to resize
 *
 * Creates a new, larger array and rehashes all existing items
 * into it. Deleted items are purged during the resize.
 *
 * @return  0 on success, error code on failure
 */
static int hashmap_resize(hashmap *hm)
{
    hashmap_item *item;
    hashmap_item **old_items = hm->items;
    uint32_t old_capacity = hm->capacity;
    uint32_t old_length = hm->length;
    uint32_t est_size = ESTIMATE_SIZE(hm);
    uint32_t new_capacity = HASHMAP_BASE_SIZE;

    /* Find the next power of 2 that will fit our data */
    while (new_capacity <= est_size) {
        new_capacity <<= 1;
    }

    /* Reset length and allocate new array */
    hm->length = 0;
    hm->items = calloc(new_capacity, sizeof(hashmap_item*));
    if (hm->items == NULL) {
        hm->items = old_items;
        return -E_ALLOC;
    }
    hm->capacity = new_capacity;

    /* Move all non-deleted items to the new array */
    for (uint32_t i = 0; i < old_capacity; i++) {
        item = old_items[i];
        if (item == NULL || item->is_deleted == 1) {
            continue;
        }
        int32_t index = _hashmap_find_empty_index(hm, item->hash);
        if (index == -E_HASHMAP_KEY_NOT_FOUND) {
            free(hm->items);
            hm->items = old_items;
            hm->capacity = old_capacity;
            hm->length = old_length;
            return index;
        }
        hm->items[index] = item;
        hm->length++;
    }

    /* Free the deleted items and old array */
    for (uint32_t i = 0; i < old_capacity; i++) {
        item = old_items[i];
        if (item != NULL && item->is_deleted == 1) {
            free(item);
        }
    }
    free(old_items);
    return 0;
}

/**
 * Delete a key-value pair from the hashmap
 * @param hm  Hashmap to modify
 * @param key  Key to delete
 *
 * Mark the item as deleted and free associated key memory.
 * The slot remains occupied but marked as deleted until a resize.
 *
 * @return  0 on success, error code on failure
 */
int hashmap_delete(hashmap *hm, char *key)
{
    if (hm == NULL || key == NULL) {
        return -E_HASHMAP_KEY_NOT_FOUND;
    }

    int32_t index = _hashmap_find_index(hm, key, hash(key));
    if (index < 0) {
        return index;
    }

    hashmap_item *item = hm->items[index];
    free(item->key);
    item->key = NULL;
    item->is_deleted = 1;
    hm->length--;

    return 0;
}

/**
 * Get a value by key from the hashmap
 * @param hm  Hashmap to search
 * @param key  Key to look up
 *
 * @return  Pointer to hashmap_item if found, NULL if not found
 */
hashmap_item *hashmap_get(hashmap *hm, char *key)
{
    if (hm == NULL || key == NULL) {
        return NULL;
    }

    int32_t index = _hashmap_find_index(hm, key, hash(key));
    if (index == -E_HASHMAP_KEY_NOT_FOUND) {
        return NULL;
    }
    return hm->items[index];
}

/**
 * Find the array index of a key.
 * @param hm  Hashmap to search
 * @param key  Key to find
 * @param hash  Pre-computed hash of the key
 *
 * Uses quadratic probing to handle collisions.
 * The probing sequence is derived from the CPython dict implementation.
 *
 * @return  Non-negative index if found, -E_HASHMAP_KEY_NOT_FOUND if not found
 */
static int32_t _hashmap_find_index(hashmap *hm, char *key, uint32_t hash)
{
    uint32_t mask = hm->capacity - 1;
    uint32_t i = hash & mask;
    uint32_t perturb;
    hashmap_item *item;

    /* Maximum number of probes - prevent potential infinite loop */
    uint32_t max_iterations = hm->capacity;
    uint32_t iterations = 0;

    for (perturb = hash; iterations < max_iterations;
         i = mask & (i * 5 + perturb + 1), perturb >>= 5, iterations++) {

        item = hm->items[i];
        if (item == NULL) {
            return -E_HASHMAP_KEY_NOT_FOUND;
        }
        if (item->is_deleted == 0
            && item->hash == hash
            && strcmp(item->key, key) == 0) {
            return (int32_t)i;
        }
    }

    /* If we've probed everywhere and didn't find it, the key doesn't exist */
    return -E_HASHMAP_KEY_NOT_FOUND;
}

/**
 * Find empty or deleted slot for insertion.
 * @param hm  Hashmap to search
 * @param hash  Hash value to use for probing
 *
 * Uses the same probing sequence as _hashmap_find_index to find
 * the first empty or deleted slot.
 *
 * @return  Index of empty/deleted slot, or -E_HASHMAP_KEY_NOT_FOUND if table is full
 */
static int32_t _hashmap_find_empty_index(hashmap *hm, uint32_t hash)
{
    uint32_t mask = hm->capacity - 1;
    uint32_t i = hash & mask;
    uint32_t perturb;
    hashmap_item *item;

    /* Maximum number of probes - prevent potential infinite loop */
    uint32_t max_iterations = hm->capacity;
    uint32_t iterations = 0;

    for (perturb = hash; iterations < max_iterations;
         i = mask & (i * 5 + perturb + 1), perturb >>= 5, iterations++) {

        item = hm->items[i];
        if (item == NULL || item->is_deleted == 1) {
            return (int32_t)i;
        }
    }

    /* If we've probed everywhere and all slots are full */
    return -E_HASHMAP_KEY_NOT_FOUND;
}

/**
 * DJB2 hash function for strings.
 * @param s  String to hash
 *
 * This is the classic DJB2 hash algorithm:
 * hash(i) = hash(i-1) * 33 + str[i]
 *
 * @return  32-bit hash value
 */
unsigned long hash(char *s)
{
    unsigned long hash = 5381, c;

    while ((c = (unsigned long)*s++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash;
}
