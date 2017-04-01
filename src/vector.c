
// libraries //

#include "vector.h"

#include "ansicolor.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <search.h>
#include <assert.h>

// macros //

#define IDX2PTR(b,i,w) ((void *)((uintptr_t)(b)+(i)*(size_t)(w)))
#define PTR2IDX(a,b,w) (((uintptr_t)(b)-(uintptr_t)(a))/(size_t)(w))

// type definitions, etc. //

struct _vector_t {
    void *data;
    size_t size;
    size_t width;
    size_t capacity;
    size_t multiplier;
    size_t increment;
    trash_t trash;
};

// static functions //

static void *idx2vec(vector_t const *vector, size_t const index) {
    return IDX2PTR(vector->data, index, vector->width);
}

static size_t vec2idx(vector_t const *vector, void const *element) {
    return PTR2IDX(vector->data, element, vector->width);
}

static bool boost_alarm(vector_t const *vector) {
    return vector->size == vector->capacity;
}

static void boost_capacity(vector_t *vector) {
    vector->capacity = vector->multiplier * vector->capacity + vector->increment;
    //void *data_boost = realloc(vector->data, vector->capacity * vector->width);
    void *data_boost = NULL;
    while (!(data_boost = realloc(vector->data, vector->capacity * vector->width)));
    //assert(data_boost);
    vector->data = data_boost;
}

// library functions

vector_t *vector_init(size_t const width, trash_t const trash) {
    return vector_init_w(width, DEFAULT_CAPACITY, DEFAULT_MULTIPLIER, DEFAULT_INCREMENT, trash);
}

// initializes vector
vector_t *vector_init_w(size_t const width, size_t const capacity, size_t const multiplier, size_t const increment, trash_t const trash) {
    assert(width > 0);
    vector_t *vector = malloc(sizeof(vector_t));
    vector->size = 0;
    vector->width = width;
    vector->capacity = capacity ? capacity : DEFAULT_CAPACITY;
    vector->multiplier = multiplier > 1 ? multiplier : DEFAULT_MULTIPLIER;
    vector->increment = increment;
    vector->trash = trash;
    vector->data = malloc(vector->capacity * vector->width);
    return vector;
}

// destroys vector
void vector_free(vector_t *vector) {
    assert(vector);
    if (vector->trash)
        for (size_t i = 0; i < vector->size; ++i)
            vector->trash(idx2vec(vector, i));
    free(vector->data);
    free(vector);
}

// setters

void vector_set_multiplier(vector_t *vector, size_t const multiplier) {
    if (multiplier)
        vector->multiplier = multiplier;
}

void vector_set_increment(vector_t *vector, size_t const increment) {
    if (increment)
        vector->increment = increment;
}

void vector_set_trash(vector_t *vector, trash_t const trash) {
    vector->trash = trash;
}

// returns number of elements in array
size_t vector_size(vector_t const *vector) {
    return vector->size;
}

// returns element at `index`
void *vector_index(vector_t const *vector, size_t const index) {
    assert(index < vector->size);
    return idx2vec(vector, index);
}

// inserts element from `source` at `index` and shifts subsequent elements
void vector_insert(vector_t *vector, void const *source, size_t const index) {
    assert(index < vector->size);
    if (boost_alarm(vector))
        boost_capacity(vector);
    memmove(idx2vec(vector, index + 1), idx2vec(vector, index), (vector->size - index) * vector->width);
    memcpy(idx2vec(vector, index), source, vector->width);
    ++vector->size;
}

// appends element from `source` to array
void vector_append(vector_t *vector, void const *source) {
    if (boost_alarm(vector))
        boost_capacity(vector);
    memcpy(idx2vec(vector, vector->size), source, vector->width);
    ++vector->size;
}

// replaces element at `index` with that from `source` and calls garbage function if necessary
void vector_replace(vector_t *vector, void const *source, size_t const index) {
    assert(index < vector->size);
    if (vector->trash)
        vector->trash(idx2vec(vector, index));
    memcpy(idx2vec(vector, index), source, vector->width);
}

void vector_remove(vector_t *vector, size_t const index) {
    assert(index < vector->size);
    if (vector->trash)
        vector->trash(idx2vec(vector, index));
    memmove(idx2vec(vector, index), idx2vec(vector, index + 1), (vector->size - index - 1) * vector->width);
    --vector->size;
}

int vector_search(vector_t const *vector, void const *key, compare_t compare, size_t start, bool const sorted) {
    assert(start < vector->size);
    size_t vector_size = vector->size;
    void *match = sorted ? bsearch(key, idx2vec(vector, start), vector_size, vector->width, compare)
                         :   lfind(key, idx2vec(vector, start), &vector_size, vector->width, compare);
    return match ? PTR2IDX(vector->data, match, vector->width) : vector_size;
}

void vector_sort(vector_t *vector, compare_t compare) {
    qsort(vector->data, vector->size, vector->width, compare);
}

void *vector_first(vector_t const *vector) {
    if (!vector->size)
        return NULL;
    return vector->data;
}

void *vector_next(vector_t const *vector, void const *previous) {
    size_t previous_index = vec2idx(vector, previous);
    assert(previous_index < vector->size);
    if (previous_index == vector->size - 1)
        return NULL;
    return idx2vec(vector, previous_index + 1);
}

void vector_print(vector_t const *vector, printv_t const printv) {
    printf("%s(%s", ANSI.red, ANSI.reset);
    for (size_t i = 0; i < vector->size; ++i) {
        void *value = idx2vec(vector, i);
        printv(value);
        if (value < idx2vec(vector, vector->size - 1))
            printf("%s,%s", ANSI.red, ANSI.reset);
    }
    printf("%s)%s", ANSI.red, ANSI.reset);
}

