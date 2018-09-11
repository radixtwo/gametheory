
#ifndef NKRAND_H
#define NKRAND_H

#include <stddef.h>
#include <stdint.h>

int nkrand(size_t mod);
void fyshuffle(void *arr, size_t n, size_t width);

int getRandomInt();
uint64_t getRandomUInt64();

#endif // NKRAND_H

