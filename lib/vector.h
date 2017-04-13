
/*
    UNTESTED
        < setters >
        < getters >
        < modifiers >
        < search >
        < iterators >
 */

#ifndef VECTOR_H
#define VECTOR_H

#include "nkdefc.h"
#include <stddef.h>
#include <stdbool.h>

typedef void (*printv_t)(void const *);

//struct _vector_t;
typedef struct _vector_t vector_t;

// constructor & destructor
vector_t *vector_init(size_t const width, trash_t const trash);
vector_t *vector_init_w(size_t const width, size_t const capacity, size_t const multiplier, size_t const increment, trash_t const trash);
void vector_free(vector_t *vector);

// setters
void vector_set_multiplier(vector_t *vector, size_t const multiplier);
void vector_set_increment(vector_t *vector, size_t const increment);
void vector_set_trash(vector_t *vector, trash_t const trash);

// getters
size_t vector_size(vector_t const *vector);
void *vector_index(vector_t const *vector, size_t const index);

// modifiers
void vector_insert(vector_t *vector, void const *source, size_t const index);
void vector_append(vector_t *vector, void const *source);
void vector_replace(vector_t *vector, void const *source, size_t const index);
void vector_remove(vector_t *vector, size_t const index);
void vector_clear(vector_t *vector);

// search
int vector_search(vector_t const *vector, void const *key, compare_t const compare, size_t const width, bool const sorted);
void vector_sort(vector_t *vector, compare_t const compare);

// iterators
void *vector_first(vector_t const *vector);
void *vector_next(vector_t const *vector, void const *previous);

// output
void vector_print(vector_t const *vector, printv_t const printv);

#endif // VECTOR_H

