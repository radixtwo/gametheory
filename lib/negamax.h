
#ifndef NEGAMAX_H
#define NEGAMAX_H

#include "game.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// private data struct typedef
struct _negamax_t;
typedef struct _negamax_t negamax_t;

negamax_t *negamax_init(game_t const *game);
void negamax_free(negamax_t *negamax);

int negamax_eval(negamax_t *negamax, node_t node, player_t player, uint8_t depth);
node_t negamax_move(negamax_t *negamax, node_t node, player_t player, uint8_t depth, int * const eval);
//negamax_rank?

#endif // NEGAMAX_H

