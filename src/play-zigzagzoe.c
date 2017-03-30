
#include "zigzagzoe.h"
#include "ansicolor.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

static bool play_again(z3_t *game) {
    printf("would you like to play again (y/n)? ");
    char yn;
    scanf(" %c", &yn);
    while (yn != 'y' && yn != 'n') {
        printf("must enter `y' or `n' !!\n");
        printf("would you like to play again (y/n)? ");
        scanf(" %c", &yn);
    }
    if (yn == 'y') {
        printf("toggle player 1 (y/n)? ");
        scanf(" %c", &yn);
        if (yn == 'y')
            game_toggle_ai(game, true, false);
        printf("toggle player 2 (y/n)? ");
        scanf(" %c", &yn);
        if (yn == 'y')
            game_toggle_ai(game, false, true);
        return true;
    } else
        return false;
}

int main() {
    srand(time(NULL));
    int type;
    printf("%sHvH, HvC, CvH, CvC (1, 2, 3, 4)? ", ANSI.erase);
    scanf(" %d", &type);
    z3_t *game = z3_init(false, false, 4);
    switch(type) {
        case 1:
            break;
        case 2:
            game_toggle_ai(game, false, true);
            break;
        case 3:
            game_toggle_ai(game, true, false);
            break;
        default:
            game_toggle_ai(game, true, true);
            break;
    }
    do {
        z3_reset(game, 4); //needs getter for nrows*ncolumns from z3.h
        game_play(game);
    } while (play_again(game));
    game_free(game);
    return 0;
}


