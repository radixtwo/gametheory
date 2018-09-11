
#include "sim.h"
#include "nkrand.h"
#include "ansicolor.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int main() {
    //srand(time(NULL));
    srand((unsigned)getRandomUInt64());
    int type;
    printf("%s1) Human vs. Human\n2) Human vs. AI\n3) AI vs. Human\n4) AI vs. AI\n\n1, 2, 3, or 4?: ", ANSI.erase);
    scanf(" %d", &type);
    sim_t *game = sim_init(false, false);
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
    game_play(game);
    game_free(game);
    return 0;
}


