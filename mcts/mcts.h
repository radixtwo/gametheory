
#ifndef NK_MCTS_H
#define NK_MCTS_H

#include "game.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

struct _mcts_t;
typedef struct _mcts_t mcts_t;

mcts_t *mcts_init(game_t const *game);
void mcts_free(mcts_t *mcts);

size_t mcts_nbytes();

int mcts_eval(mcts_t *mcts, node_t node, player_t player, uint8_t nsims);
node_t mcts_move(mcts_t *mcts, node_t node, player_t player, uint8_t nsims);

int mcts_eval_time(mcts_t *mcts, node_t node, player_t player, uint8_t time);
node_t mcts_move_time(mcts_t *mcts, node_t node, player_t player, uint8_t time);

#endif // NK_MCTS_H

