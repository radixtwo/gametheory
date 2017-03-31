
#ifndef SIM_H
#define SIM_H

#include "game.h"
#include <stdint.h>
#include <stdbool.h>

typedef game_t sim_t;

sim_t *sim_init(bool player1_ai, bool player2_ai);
sim_t *sim_init_w(uint8_t nvertices, bool player1_ai, bool player2_ai, uint8_t depth);
void sim_reset(sim_t *game);
void sim_free(sim_t *game);

#endif // SIM_H

