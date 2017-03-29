
/*  file name: hashmap.c
 *  author: neil khemani
 */

// libraries //

#include "hashmap.h"
#include "xxhash.h"

#include "ansicolor.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

// macros //

#define DEFAULT_NBUCKETS    DEFAULT_CAPACITY    // nkdef.c
#define DEFAULT_LOADFACTOR  1.5
//#define HASH_INITIALIZE     5381    // k&r uses 0
//#define HASH_MULTIPLIER     33      // k&r uses 31

#define IDX2PTR(b,i,w) ((void *)((uintptr_t)(b)+(i)*(size_t)(w)))
#define PTR2IDX(a,b,w) (((uintptr_t)(b)-(uintptr_t)(a))/(size_t)(w))

// type definitions, etc. //

typedef void *entry_t;

struct _hashmap_t {
    size_t size;
    size_t width_k;
    size_t width_v;
    size_t nbuckets;
    size_t multiplier;
    size_t increment;
    double loadfactor;
    uint64_t seed;
    entry_t *buckets;
    trash_t trash;
};

// static functions //

// entry_t utilities //

// returns 
static entry_t entry_next(entry_t const entry) {
    return *(entry_t *)entry;
}

static void *entry_key(entry_t const entry) {
    return IDX2PTR(entry, 1, sizeof(entry_t *));
}

static void *entry_value(hashmap_t const *hashmap, entry_t const entry) {
    return IDX2PTR(entry, 1, sizeof(entry_t *) + hashmap->width_k);
}

static void entry_set_next(entry_t entry, entry_t const next) {
    *(entry_t *)entry = next;
}

static void entry_set_key(hashmap_t const *hashmap, entry_t entry, void const *key) {
    memcpy(entry_key(entry), key, hashmap->width_k);
}

static void entry_set_value(hashmap_t const *hashmap, entry_t entry, void const *value) {
    memcpy(entry_value(hashmap, entry), value, hashmap->width_v);
}

static entry_t entry_init(hashmap_t const *hashmap, entry_t const next, void const *key, void const *value) {
    entry_t entry = malloc(sizeof(entry_t *) + hashmap->width_k + hashmap->width_v);
    assert(entry);
    entry_set_next(entry, next);
    entry_set_key(hashmap, entry, key);
    entry_set_value(hashmap, entry, value);
    return entry;
}

static entry_t value2entry(hashmap_t const *hashmap, void *value) {
    return IDX2PTR(value, -1, hashmap->width_k + sizeof(entry_t *));
}

/*
static size_t hash(hashmap_t const *hashmap, void const *key) {
   size_t hash = HASH_INITIALIZE;
   for (size_t i = 0; i < hashmap->width_k; ++i)
      hash = HASH_MULTIPLIER * hash + *(unsigned char *)IDX2PTR(key, i, sizeof(char));
   return hash % hashmap->nbuckets;
}
*/

// returns index of bucket associated with hash of `key`
static size_t key2idx(hashmap_t const *hashmap, void const *key) {
    //return hash(hashmap, key);
    return XXH64(key, hashmap->width_k, hashmap->seed) % hashmap->nbuckets;
}

// returns true if hash map's load factor exceeds threshold factor
static bool rehash_alarm(hashmap_t const *hashmap) {
    return hashmap->size >= hashmap->loadfactor * hashmap->nbuckets;
}

// rehashes map
static void rehash(hashmap_t *hashmap, size_t multiplier, size_t increment) {
    if (hashmap && hashmap->size) {
        size_t nbuckets = hashmap->nbuckets;
        entry_t *buckets = hashmap->buckets;
        hashmap->nbuckets = multiplier * nbuckets + increment;
        hashmap->buckets = calloc(hashmap->nbuckets, sizeof(entry_t));
        for (entry_t *bucket = buckets; bucket < buckets + nbuckets; ++bucket) {
            entry_t root = *bucket;
            while (root) {
                entry_t leaf = root;
                root = entry_next(root);
                entry_set_next(leaf, NULL);
                size_t index = key2idx(hashmap, entry_key(leaf));
                if (!hashmap->buckets[index])
                    hashmap->buckets[index] = leaf;
                else {
                    entry_t clash = hashmap->buckets[index];
                    while (entry_next(clash))
                        clash = entry_next(clash);
                    entry_set_next(clash, leaf);
                }
            }
        }
        free(buckets);
    }
}

// recursively frees any subsequent collided entries in bucket,
// calls client-specific garbage function on current entry's
// value, and deallocates entry's memory in heap
static void bucket_free(hashmap_t *hashmap, entry_t entry) {
    if (hashmap && entry) {
        bucket_free(hashmap, entry_next(entry));
        if (hashmap->trash)
            hashmap->trash(entry_value(hashmap, entry));
        free(entry);
    }
}

// library functions //

hashmap_t *hashmap_init(size_t const width_k, size_t const width_v, uint64_t const seed, trash_t const trash) {
    return hashmap_init_w(width_k, width_v, DEFAULT_NBUCKETS, DEFAULT_MULTIPLIER, DEFAULT_INCREMENT, DEFAULT_LOADFACTOR, seed, trash);
}

// initializes hash map
hashmap_t *hashmap_init_w(size_t const width_k, size_t const width_v, size_t const nbuckets, size_t const multiplier,
                          size_t const increment, double loadfactor, uint64_t const seed, trash_t const trash) {
    assert(width_k > 0 && width_v > 0);
    hashmap_t *hashmap = malloc(sizeof(hashmap_t));
    hashmap->size = 0;
    hashmap->width_k = width_k;
    hashmap->width_v = width_v;
    hashmap->nbuckets = nbuckets ? nbuckets : DEFAULT_NBUCKETS;
    hashmap->multiplier = multiplier ? multiplier : DEFAULT_MULTIPLIER;
    hashmap->increment = increment ? increment : DEFAULT_INCREMENT;
    hashmap->loadfactor = loadfactor >= 1 ? loadfactor : DEFAULT_LOADFACTOR;
    hashmap->seed = seed;
    hashmap->buckets = calloc(hashmap->nbuckets, sizeof(entry_t));
    hashmap->trash = trash;
    return hashmap;
}

// destroys hash map
void hashmap_free(hashmap_t *hashmap) {
    if (hashmap) {
        if (hashmap->size)
            for (entry_t *bucket = hashmap->buckets; bucket < hashmap->buckets + hashmap->nbuckets; ++bucket)
                bucket_free(hashmap, *bucket);
        free(hashmap->buckets);
        free(hashmap);
    }
}

hashmap_t *hashmap_load(char const *filename) {
    assert(filename);
    FILE *fh = fopen(filename, "rb");
    if (!fh)
        return NULL;
    hashmap_t *hashmap = malloc(sizeof(hashmap_t));
    assert(hashmap);
    hashmap->size = 0;
    fread(&hashmap->width_k, sizeof(size_t), 1, fh);
    fread(&hashmap->width_v, sizeof(size_t), 1, fh);
    fread(&hashmap->nbuckets, sizeof(size_t), 1, fh);
    fread(&hashmap->multiplier, sizeof(size_t), 1, fh);
    fread(&hashmap->increment, sizeof(size_t), 1, fh);
    fread(&hashmap->loadfactor, sizeof(double), 1, fh);
    fread(&hashmap->seed, sizeof(uint64_t), 1, fh);
    hashmap->buckets = calloc(hashmap->nbuckets, sizeof(entry_t));
    assert(hashmap->buckets);
    hashmap->trash = NULL;
    char buffer[hashmap->width_k + hashmap->width_v];
    while (fread(buffer, hashmap->width_k + hashmap->width_v, 1, fh))
        hashmap_set(hashmap, buffer, &buffer[hashmap->width_k]);
    fclose(fh);
    return hashmap;
}

bool hashmap_save(hashmap_t const *hashmap, char const *filename) {
    assert(hashmap);
    if (hashmap->trash)
        return false;
    FILE *fh = fopen(filename, "wb");
    if (!fh)
        return false;
    fwrite((void const *)&hashmap->width_k, sizeof(size_t), 1, fh);
    fwrite((void const *)&hashmap->width_v, sizeof(size_t), 1, fh);
    fwrite((void const *)&hashmap->nbuckets, sizeof(size_t), 1, fh);
    fwrite((void const *)&hashmap->multiplier, sizeof(size_t), 1, fh);
    fwrite((void const *)&hashmap->increment, sizeof(size_t), 1, fh);
    fwrite((void const *)&hashmap->loadfactor, sizeof(double), 1, fh);
    fwrite((void const *)&hashmap->seed, sizeof(uint64_t), 1, fh);
    for (entry_t *bucket = hashmap->buckets; bucket < hashmap->buckets + hashmap->nbuckets; ++bucket) {
        entry_t entry = *bucket;
        while (entry) {
            fwrite((void const *)entry_key(entry), hashmap->width_k + hashmap->width_v, 1, fh);
            entry = entry_next(entry);
        }
    }
    fclose(fh);
    return true;
}

void hashmap_set_multiplier(hashmap_t *hashmap, size_t const multiplier) {
    if (multiplier > 1)
        hashmap->multiplier = multiplier;
}

void hashmap_set_increment(hashmap_t *hashmap, size_t const increment) {
    hashmap->increment = increment;
}

void hashmap_set_loadfactor(hashmap_t *hashmap, double const loadfactor) {
    if (loadfactor >= 1.0) {
        hashmap->loadfactor = loadfactor;
        if (rehash_alarm(hashmap))
            rehash(hashmap, hashmap->multiplier, hashmap->increment);
    }
}

void hashmap_set_seed(hashmap_t *hashmap, uint64_t const seed) {
    if (seed != hashmap->seed) {
        hashmap->seed = seed;
        rehash(hashmap, 1, 0);
    }
}

void hashmap_set_trash(hashmap_t *hashmap, trash_t const trash) {
    hashmap->trash = trash;
}

// getters

size_t hashmap_size(hashmap_t const *hashmap) {
    return hashmap->size;
}

void *hashmap_get(hashmap_t const *hashmap, void const *key) {
    for (entry_t entry = hashmap->buckets[key2idx(hashmap, key)]; entry; entry = entry_next(entry))
        if (!memcmp(key, entry_key(entry), hashmap->width_k))
            return entry_value(hashmap, entry);
    return NULL;
}

// modifiers

void hashmap_set(hashmap_t *hashmap, void const *key, void const *value) {
    size_t index = key2idx(hashmap, key);
    entry_t entry = hashmap->buckets[index];
    for (; entry; entry = entry_next(entry)) {
        if (!memcmp(key, entry_key(entry), hashmap->width_k)) {
            if (hashmap->trash)
                hashmap->trash(entry_value(hashmap, entry));
            entry_set_value(hashmap, entry, value);
            return;
        }
    }
    ++hashmap->size;
    entry = entry_init(hashmap, NULL, key, value);
    if (!hashmap->buckets[index])
        hashmap->buckets[index] = entry;
    else {
        entry_t clash = hashmap->buckets[index];
        while (entry_next(clash))
            clash = entry_next(clash);
        entry_set_next(clash, entry);
    }
    if (rehash_alarm(hashmap))
        rehash(hashmap, hashmap->multiplier, hashmap->increment);
}

void hashmap_remove(hashmap_t *hashmap, void const *key) {
    size_t index = key2idx(hashmap, key);
    entry_t entry = hashmap->buckets[index], previous = NULL;
    while (entry) {
        if (!memcmp(key, entry_key(entry), hashmap->width_k))
            break;
        previous = entry;
        entry = entry_next(entry);
    }
    if (entry) {
        if (hashmap->trash)
            hashmap->trash(entry_value(hashmap, entry));
        if (!previous)
            hashmap->buckets[index] = entry_next(entry);
        else
            entry_set_next(previous, entry_next(entry));
        free(entry);
        --hashmap->size;
    }
}

void *hashmap_first(hashmap_t const *hashmap) {
    if (!hashmap->size)
        return NULL;
    for (entry_t *bucket = hashmap->buckets; bucket < hashmap->buckets + hashmap->nbuckets; ++bucket)
        if (*bucket)
            return entry_key(*bucket);
    return NULL;
}

void *hashmap_next(hashmap_t const *hashmap, void const *key) {
    void *value = hashmap_get(hashmap, key);
    if (!value)
        return NULL;
    entry_t entry = value2entry(hashmap, value);
    entry_t next = entry_next(entry);
    if (next)
        return entry_key(next);
    for (entry_t *bucket = &hashmap->buckets[1 + key2idx(hashmap, key)]; bucket < hashmap->buckets + hashmap->nbuckets; ++bucket)
        if (*bucket)
            return entry_key(*bucket);
    return NULL;
}

/*
void hashmap_print(hashmap_t const *hashmap, char const *meta, printkv_t const printkv) {
    printf("%s%s(hashmap_t *)%s%s\n", ANSI.bold, ANSI.green, meta, ANSI.reset);
    printf("%s(%ssz%s,%swk%s,%swv%s,%snb%s)=(%s%llu,%llu,%llu,%llu)\n",
           ANSI.green, ANSI.reset, ANSI.green, ANSI.reset, ANSI.green, ANSI.reset, ANSI.green, ANSI.reset, ANSI.green, ANSI.reset,
           (uint64_t)hashmap->size, (uint64_t)hashmap->width_k, (uint64_t)hashmap->width_v, (uint64_t)hashmap->nbuckets);
    printf("%s(%smr%s,%sit%s,%slf%s,%ssd%s)=(%s%llu,%llu,%f,%llu)\n",
           ANSI.green, ANSI.reset, ANSI.green, ANSI.reset, ANSI.green, ANSI.reset, ANSI.green, ANSI.reset, ANSI.green, ANSI.reset,
           (uint64_t)hashmap->multiplier, (uint64_t)hashmap->increment, hashmap->loadfactor, hashmap->seed);
    printf("%s%8s .. %-8s\n", ANSI.blue, "bucket", "entries");
    for (entry_t *bucket = hashmap->buckets; bucket < hashmap->buckets + hashmap->nbuckets; ++bucket) {
        if (*bucket) {
            size_t index = PTR2IDX(hashmap->buckets, bucket, sizeof(entry_t));
            printf("%8llu ..", (uint64_t)index);
            entry_t node = *bucket;
            unsigned count = 1;
            while ((node = entry_next(node)))
                ++count;
            printf(" %-8u\n", count);
        }
    }
    printf("%s", ANSI.reset);
}
*/

void hashmap_print(hashmap_t const *hashmap, printkv_t const printkv) {
    //printf("starting hashmap_print...\n\n");
    printf("%s(%s", ANSI.green, ANSI.reset);
    for (void *key = hashmap_first(hashmap); key; key = hashmap_next(hashmap, key)) {
        //printf("\n\t%s(%s", ANSI.green, ANSI.reset);
        printf("%s(%s", ANSI.green, ANSI.reset);
        printkv(key, hashmap_get(hashmap, key));
        printf("%s)%s", ANSI.green, ANSI.reset);
        if (hashmap_next(hashmap, key))
            printf("%s,%s", ANSI.green, ANSI.reset);
    }
    //printf("\n%s)%s\n", ANSI.green, ANSI.reset);
    printf("%s)%s", ANSI.green, ANSI.reset);
}

/*
void hashmap_print_sort(hashmap_t const *hashmap, printkv_t const printkv, compare_t const compare) {
    printf("reached h1\n");
    vector_t *keys = vector_init(hashmap->width_k, NULL);
    printf("reached h2\n");
    for (void *key = hashmap_first(hashmap); key; key = hashmap_next(hashmap, key)) {
        vector_append(keys, key);
        printf("%llu\n", *(unsigned long long *)key);
    }
    printf("reached h3\n");
    vector_sort(keys, compare);
    printf("%s(%s", ANSI.green, ANSI.reset);
    for (void *key = vector_first(keys); key; key = vector_next(keys, key)) {
        printf("%s(%s", ANSI.green, ANSI.reset);
        printkv(key, hashmap_get(hashmap, key));
        printf("%s)%s", ANSI.green, ANSI.reset);
        if (vector_next(keys, key))
            printf("%s,%s", ANSI.green, ANSI.reset);
    }
    printf("%s)%s", ANSI.green, ANSI.reset);
    vector_free(keys);
}
*/

