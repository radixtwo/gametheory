
#include "connectn.h"
#include <stdlib.h>
#include <stdio.h>

#define DEFAULT_SAVEFILE "res/connectn.brain"

static bool play_again(connectn_t *game) {
    printf("would you like to play again (y/n)? ");
    char yn;
    scanf(" %c", &yn);
    while (yn != 'y' && yn != 'n') {
        printf("must enter `y' or `n' !!\n");
        printf("would you like to play again (y/n)? ");
        scanf(" %c", &yn);
    }
    if (yn == 'y') {
        printf("toggle computer (y/n)? ");
        scanf(" %c", &yn);
        if (yn == 'y')
            connectn_toggle_negamax(game, true, true);
        return true;
    } else
        return false;
}

int main() {
    connectn_t *game = load_connectn(DEFAULT_SAVEFILE);
    do {
        connectn_reset(game);
        connectn_play(game);
    } while (play_again(game));
    save_connectn(game, DEFAULT_SAVEFILE);
    free_connectn(game);
    return 0;
}


