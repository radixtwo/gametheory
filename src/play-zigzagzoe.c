
#include "zigzagzoe.h"
#include "ansicolor.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int main() {
    srand(time(NULL));
    int type = 0;
    printf("%sregular match or heuristic test (1, 2)? ", ANSI.erase);
    scanf(" %d", &type);
    if (type == 1) {
        z3_t *game = z3_init(true, true);
        printf("HvH, HvAI, AIvH, or AIvAI (1, 2, 3, 4)? ");
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
        z3_free(game);
    } else {
        z3_t *game1 = z3_init(true, false);
        z3_t *game2 = z3_init_h2(false, true);
        printf("C1vC2 or C2vC1 (1, 2)? ");
        scanf(" %d", &type);
        while (type < 1 || type > 2) {
            switch(type) {
                case 1:
                    break;
                case 2:
                    game_toggle_ai(game1, true, true);
                    game_toggle_ai(game2, true, true);
                    break;
                default:
                    printf("must enter '1' or '2'! ");
                    scanf(" %d", &type);
                    break;
            }
        }
        z3_play_ai2(game1, game2);
        z3_free(game1);
        z3_free(game2);
    }
    return 0;
}


