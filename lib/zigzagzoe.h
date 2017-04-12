
#ifndef ZIGZAGZOE_H
#define ZIGZAGZOE_H

#include "game.h"
#include <stdint.h>
#include <stdbool.h>

typedef enum _z3_stale_t {
    Z3_WIN = 1,
    Z3_DRAW = 0,
    Z3_LOSS = -1
} z3_stale_t;

typedef game_t z3_t;

z3_t *z3_init(bool player1_ai, bool player2_ai);
z3_t *z3_init_h2(bool player1_ai, bool player2_ai);
z3_t *z3_init_w(uint8_t M, uint8_t N, uint8_t K, uint8_t block_init, z3_stale_t mate,
                char tile_p1, char tile_p2, char tile_na, char tile_clog,
                uint8_t depth, bool player1_ai, bool player2_ai);
z3_t *z3_init_h2_w(uint8_t M, uint8_t N, uint8_t K, uint8_t block_init, z3_stale_t mate,
                char tile_p1, char tile_p2, char tile_na, char tile_clog,
                uint8_t depth, bool player1_ai, bool player2_ai);
void z3_reset(z3_t *game);
void z3_free(z3_t *game);
void z3_advance_ai2(z3_t *game1, z3_t *game2);
void z3_play_ai2(z3_t *game1, z3_t *game2);

// iOS app functionality
z3_t *z3_iOS_SetupGame_Human(int M, int N, int K, int initBlock, int staleMode);
//z3_t *z3_iOS_SetupGame_AI(int M, int N, int K, int initBlock, int stateMode, int playerAI, int difficulty);
int *z3_iOS_Move_Human(z3_t *humanGame, int tileNumber, int playerNumber, size_t *nResults);
//int *z3_iOS_Move_AI(int tileNumber, int playerNumber);


#endif // ZIGZAGZOE_H

