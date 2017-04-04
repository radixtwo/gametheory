
#include "nkrand.h"
#include <stdlib.h>
#include <string.h>

// performs 'rand() % mod' w/ better uniformity for small 'mod'
int nkrand(size_t mod) {
    return rand() / (1 + RAND_MAX / mod);
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


