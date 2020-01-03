
#include "sim.h"
#include "nkrand.h"
#include "ansicolor.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

// main loop
int main() {
    //srand(time(NULL));
    srand((unsigned)getRandomUInt64()); // this is not pseudorandom
    int type;
    printf("%s1) Human vs. Human\n2) Human vs. AI\n3) AI vs. Human\n4) AI vs. AI\n\n1, 2, 3, or 4?: ", ANSI.erase);
    scanf(" %d", &type);
    sim_t *game = sim_init(false, false);
    switch(type) {
        case 1: // nothing needs to be changed
            break;
        case 2: // player 2 is AI
            game_toggle_ai(game, false, true);
            break;
        case 3: // player 1 is AI
            game_toggle_ai(game, true, false);
            break;
        default: // both players are AI
            game_toggle_ai(game, true, true);
            break;
    }
    game_play(game); // game loop
    game_free(game); // release resources
    return 0;
}


