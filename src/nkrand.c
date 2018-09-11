
#include "nkrand.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
static const size_t cacheSize = 1024 * 1024;
static char cache[cacheSize];
static size_t cacheCursor = 0;

static char readCache() {
    if (cacheCursor == cacheSize)
        //reload cache
    FILE *fp - fopen("/dev/urandom", "r");
    if (!fp) {
        perror("<nkrand.h>: readCache()");
        exit(-1);
    }

    int value = 0;
    return 1;
}
*/




// performs 'rand() % mod' w/ better uniformity for small 'mod'
int nkrand(size_t mod) {
    return rand() / (1 + RAND_MAX / mod);
    //return getRandomInt() % mod;
}

// fisher-yates shuffle array
void fyshuffle(void *arr, size_t n, size_t width) {
    if (n <= 1)
        return;
    char tmp[width];
    for (size_t i = 0; i < n - 1; ++i) {
        size_t j = i + nkrand(n - i);
        memcpy(tmp, (char *)arr + j * width, width);
        memcpy((char *)arr + j * width, (char *)arr + i * width, width);
        memcpy((char *)arr + i * width, tmp, width);
    }
}

int getRandomInt() {
    //FILE *fp = fopen("/dev/random", "r");
    FILE *fp = fopen("/dev/urandom", "r");
    if (!fp) {
        perror("<nkrand.h>: getRandomInt()");
        exit(-1);
    }

    int value = 0;
    for (size_t byte = 0; byte < sizeof(value); ++byte) {
        value <<= 8;
        value |= fgetc(fp);
    }

    fclose(fp);
    return value;
}

uint64_t getRandomUInt64() {
    //FILE *fp = fopen("/dev/random", "r");
    FILE *fp = fopen("/dev/urandom", "r");
    if (!fp) {
        perror("<nkrand.h>: getRandomUInt64()");
        exit(-1);
    }

    uint64_t value = 0;
    for (size_t byte = 0; byte < sizeof(value); ++byte) {
        value <<= 8;
        value |= fgetc(fp);
    }

    fclose(fp);
    return value;
}



