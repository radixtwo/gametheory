
#ifndef ZIGZAGZOE_H
#define ZIGZAGZOE_H

#include "game.h"
#include <stdint.h>
#include <stdbool.h>

typedef game_t z3_t;

z3_t *z3_init(bool cpu_p1, bool cpu_p2, uint8_t block_init);
z3_t *z3_init_w(uint8_t nrows, uint8_t ncolumns, uint8_t nconnect, uint8_t block_init,
                char tile_p1, char tile_p2, char tile_na,
                bool cpu_p1, bool cpu_p2, uint8_t depth);
void z3_reset(z3_t *game, uint8_t block_init);
void z3_free(z3_t *game);

void z3_toggle_cpu(z3_t *game, bool toggle_p1, bool toggle_p2);
//TODO:setters for `game.[hc]`
void z3_set_block_init(z3_t *game, uint8_t block_init);
void z3_play(z3_t *game);

#endif // ZIGZAGZOE_H

