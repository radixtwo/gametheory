
#ifndef ANSICOLOR_H
#define ANSICOLOR_H

#define ANSI_CLEAR()        printf("\033[H\033[J")
#define ANSI_CURSOR(x,y)    printf("\033[%d;%dH",(x),(y))

extern const struct _ANSI {
    char *reset;
    char *bold;
    char *erase;
    char *black;
    char *red;
    char *green;
    char *yellow;
    char *blue;
    char *magenta;
    char *cyan;
    char *white;
} ANSI;

#endif // ANSICOLOR_H

