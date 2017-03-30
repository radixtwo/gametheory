
#ifndef CONNECTN_H
#define CONNECTN_H

#include "game.h"
#include <stdint.h>
#include <stdbool.h>

typedef game_t connectn_t;

connectn_t *connectn_init(uint8_t nrows, uint8_t ncolumns, uint8_t nconnect, char tile_p1, char tile_p2, char tile_na);

connectn_t *init_connectn();
connectn_t *load_connectn(const char *filename);
connectn_t *init_connectn_w(uint8_t nrows, uint8_t ncolumns, uint8_t nconnect,
                            char player1_tile, char player2_tile, char empty_tile,
                            bool player1_negamax, bool player2_negamax, uint8_t depth,
                            const char *filename);
void free_connectn(connectn_t *connectn);
void save_connectn(const connectn_t *connectn, const char *filename);
void connectn_toggle_negamax(connectn_t *connectn, bool player1_toggle, bool player2_toggle);
void connectn_reset(connectn_t *connectn);
void connectn_print(connectn_t *connectn);
void connectn_advance(connectn_t *connectn);
void connectn_play(connectn_t *connectn);

#endif // CONNECTN_H

