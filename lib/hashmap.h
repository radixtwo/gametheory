
/*
    UNTESTED
        < constructors >
        < setters >
        hashmap_remove
        < iterators >
 */

#ifndef HASHMAP_H
#define HASHMAP_H

#include "nkdefc.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef void (*printkv_t)(void const *, void const *);

struct _hashmap_t;
typedef struct _hashmap_t hashmap_t;

// constructor & destructor
hashmap_t *hashmap_init(size_t const width_k, size_t const width_v, uint64_t const seed, trash_t const trash);
hashmap_t *hashmap_init_w(size_t const width_k, size_t const width_v, size_t const nbuckets, size_t const multiplier,
                          size_t const increment, double loadfactor, uint64_t const seed, trash_t const trash);
void hashmap_free(hashmap_t *hashmap);

// file io
hashmap_t *hashmap_load(char const *filename);
bool hashmap_save(hashmap_t const *hashmap, char const *filename);

// setters
void hashmap_set_multiplier(hashmap_t *hashmap, size_t const multiplier);
void hashmap_set_increment(hashmap_t *hashmap, size_t const increment);
void hashmap_set_loadfactor(hashmap_t *hashmap, double const loadfactor);
void hashmap_set_seed(hashmap_t *hashmap, uint64_t const seed);
void hashmap_set_trash(hashmap_t *hashmap, trash_t const trash);

// getters
size_t hashmap_size(hashmap_t const *hashmap);
void *hashmap_get(hashmap_t const *hashmap, void const *key);
size_t hashmap_nbytes(hashmap_t const *hashmap);

// modifiers
void hashmap_set(hashmap_t *hashmap, void const *key, void const *value);
void hashmap_remove(hashmap_t *hashmap, void const *key);

// iterators
void *hashmap_first(hashmap_t const *hashmap);
void *hashmap_next(hashmap_t const *hashmap, void const *key);

// output
void hashmap_print(hashmap_t const *hashmap, printkv_t const printkv);
//void hashmap_print_sort(hashmap_t const *hashmap, printkv_t const printkv, compare_t const compare);

#endif // HASHMAP_H

