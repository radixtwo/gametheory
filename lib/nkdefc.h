
#ifndef NKDEFC_H
#define NKDEFC_H

#define DEFAULT_CAPACITY    8
#define DEFAULT_MULTIPLIER  3
#define DEFAULT_INCREMENT   1

typedef int (*compare_t)(void const *, void const *);
typedef void (*trash_t)(void *);

#endif // NKDEFC_H

