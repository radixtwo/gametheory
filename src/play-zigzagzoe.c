
#include "zigzagzoe.h"
#include "ansicolor.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int main() {
    srand(time(NULL));
    int type = 0;
    z3_t *game = z3_init(true, true);
    printf("%sHvH, HvC, CvH, CvC (1, 2, 3, 4)? ", ANSI.erase);
    scanf(" %d", &type);
    while (type < 1 || type > 4) {
        switch(type) {
            case 1:
                break;
            case 2:
                game_toggle_ai(game, false, true);
                break;
            case 3:
                game_toggle_ai(game, true, false);
                break;
            case 4:
                game_toggle_ai(game, true, true);
                break;
            default:
                printf("must enter '1', '2', '3', or '4'! ");
                scanf(" %d", &type);
                break;
        }
    }
    game_play(game);
    game_free(game);
    return 0;
}


