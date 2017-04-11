
/*
_____________________
filename: zigzagzoe.c
author: neil datyal
_____________________
description:

"zigzagzoe" is played on an M-by-N board, each slot containing another M-by-N sub-board. players take turns placing tiles
in each sub-board. the sub-board where player 1 may drop a tile on the first move (or ply) is randomly selected. on each
move afterwards, a player may only drop a tile in the sub-board specified by the previous player's move. if player 1 drops
a tile in the slot at index 'i' in a sub-board, then player 2 must drop a tile in the sub-board at index 'i' on the main
board.

if a player has K tiles adjacent to each other in a single row, column, or diagonal in a sub-board, then that player has
created a K-connection in that sub-board. for each sub-board, the player who first creates a K-connection in that sub-board
"wins" the sub-board, and the other player may not win that sub-board, even if the other player subsequently creates a coexisting
K-connection in that sub-board. the first player to win K adjacent sub-boards horizontally, vertically, or diagonally on
the main board has created a "zigzagzoe" and wins the entire game.

it is possible that a sub-board has no empty slots, yet a player must drop a tile in that sub-board, as designated by the
previous player's move (known as "stalemate"). in this case, three outcomes for the game are possible: the current player
without any options wins (default), loses, or draws. which of these three outcomes occurs is decided upon prior to the start
of the game.
_____________________
terminology:

"potency" refers to the total possible number of K-connections that a player can create in a sub-board or on the main board.
"previous" refers to the index of the slot where the last player dropped a tile. it also refers to the index of the sub-board
where the current player may drop a tile.
"mega-board" refers to the overall, main M-by-N board, in contrast to the "sub-boards" in each slot of the mega-board.
"block" refers to either a sub-board or the mega-board. for the purposes of this file, blocks are usually M-by-N arrays that
can represent either a sub-board or the mega-board.
_____________________
*/

#include "zigzagzoe.h"
#include "negamax.h"
#include "nkrand.h"
#include "ansicolor.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BLKIDX(r,c,M)       ((r) * (M) + (c))
#define MEGIDX(R,C,r,c,N,M) ((R) * (M) * (N) * (M) + (C) * (N) * (M) + (r) * (M) + (c))

#define INIT_M  3
#define INIT_N  3
#define INIT_K  3
#define INIT_TILE_P1    'X'
#define INIT_TILE_P2    'O'
#define INIT_TILE_NA    '-'
#define INIT_TILE_CLOG  '$'
#define INIT_DEPTH_AI   6
#define INIT_STALE      Z3_WIN

typedef struct _z3_config_t {
    uint8_t M;
    uint8_t N;
    uint8_t K;
    z3_stale_t mate;
    char tile_p1;
    char tile_p2;
    char tile_na;
    char tile_clog;
} z3_config_t;

// stores previous, mega-block, & board
// width varies with M, N, & K
typedef char *z3_node_t;


//********************//
//  STATIC FUNCTIONS  //
//____________________//


//********************//
//  inline functions  //
//____________________//


// returns heap-allocated duplicate of passed node
static inline z3_node_t z3_node_dup(z3_t const *game, z3_node_t node) {
    z3_node_t dup = malloc(game_width(game));
    memcpy(dup, node, game_width(game));
    return dup;
}

// returns number of slots on board
static inline uint8_t z3_nslots(z3_config_t const *config) {
    return config->M * config->N * config->M * config->N;
}

// returns 'z3_node_t' length in terms of M and N
static inline size_t  z3_node_width_rc(uint8_t r, uint8_t c) {
    return (size_t)(1 + (r * c) + (r * c * r * c));
}

// 'z3_node_width_rc' wrapper
static inline size_t  z3_node_width(z3_config_t const *config) {
    return z3_node_width_rc(config->M, config->N);
}

// extracts 'previous' from 'z3_node_t' (index of sub-board for current player)
static inline uint8_t z3_node_previous(z3_node_t const node) {
    return *(uint8_t *)node;
}

// extracts mega-board from 'z3_node_t' (MxN board recording won sub-boards)
static inline char  *z3_node_mega(z3_node_t node) {
    return ++node;
}

// extracts main board from 'z3_node_t'
static inline char  *z3_node_blocks(z3_config_t const *config, z3_node_t node) {
    return ++node + config->M * config->N;
}

// extracts sub-board for current player from 'z3_node_t'
static inline char  *z3_node_block(z3_config_t const *config, z3_node_t const node, uint8_t previous) {
    char *blocks = z3_node_blocks(config, node);
    uint8_t row = previous / config->N;
    uint8_t col = previous % config->N;
    return &blocks[MEGIDX(row, col, 0, 0, config->M, config->N)];
}

// returns number of tiles on board
static inline uint8_t z3_node_ntiles(z3_config_t const *config, z3_node_t const node) {
    char *blocks = z3_node_blocks(config, node);
    uint8_t ntiles = 0;
    for (size_t slot = 0; slot < z3_nslots(config); ++slot)
        if (blocks[slot] != config->tile_na)
            ++ntiles;
    return ntiles;
}

// returns number of empty slots on board
static inline uint8_t z3_node_nempty(z3_config_t const *config, z3_node_t const node) {
    char *blocks = z3_node_blocks(config, node);
    uint8_t nempty = 0;
    for (size_t slot = 0; slot < z3_nslots(config); ++slot)
        if (blocks[slot] == config->tile_na)
            ++nempty;
    return nempty;
}

// returns true if player to move for given tile is player 1
static inline bool  z3_node_player1(z3_config_t const *config, z3_node_t const node) {
    return !(z3_node_ntiles(config, node) % 2);
}

// returns 'true' if the sub-board for the current player has no options
static inline bool  z3_node_stale(z3_config_t const *config, z3_node_t const node) {
    uint8_t previous = z3_node_previous(node);
    char *block = z3_node_block(config, node, previous);
    for (uint8_t slot = 0; slot < config->M * config->N; ++slot)
        if (block[slot] == config->tile_na)
            return false;
    return true;
}

// returns root 'z3_node_t' given initial sub-board for player 1
static inline z3_node_t z3_node_root(z3_config_t const *config, uint8_t previous_init) {
    size_t width = z3_node_width(config);
    z3_node_t root = malloc(width * sizeof(char));
    *(uint8_t *)root = previous_init;
    for (size_t slot = 1; slot < width; ++slot)
        root[slot] = config->tile_na;
    return root;
}

// returns tile associated with player
static inline char  z3_tile_player(z3_config_t const *config, player_t player) {
    return player == P_OAKLEY ? config->tile_p1 : config->tile_p2;
}

static inline player_t  z3_player_tile(z3_config_t const *config, char tile) {
    if (tile == config->tile_p1)
        return P_OAKLEY;
    if (tile == config->tile_p2)
        return P_TAYLOR;
    return P_DAKOTA;
}

//*******************//
//  helper functions //
//___________________//


/*
///  20170331 deprecated  ///
// returns number of sub-boards won by given player on given node
static uint8_t  z3_blocks_owned(z3_config_t const *config, z3_node_t node, player_t player) {
    char *mega = z3_node_mega(node);
    char tile = z3_tile_player(config, player);
    uint8_t count = 0;
    for (uint8_t t = 0; t < config->M * config->N; ++t)
        if (mega[t] == tile)
            ++count;
    return count;
}
*/

// returns tile of winner of given block or empty tile if no winner
static char z3_block_won(z3_config_t const *config, char const *block) {
    char tile = config->tile_na;
    bool won = true;
    int8_t row, col, n;
    for (row = 0; row < config->M; ++row) {
        for (col = 0; col < config->N - config->K + 1; ++col) {
            tile = block[BLKIDX(row, col, config->N)];
            if (tile == config->tile_na)
              continue;
            won = true;
            for (n = 1; n < config->K; ++n) {
                char slot = block[BLKIDX(row, col + n, config->N)];
                if (slot != tile) {
                    won = false;
                    break;
                }
            }
            if (won)
              return tile;
        }
    }
    for (row = 0; row < config->M - config->K + 1; ++row) {
        for (col = 0; col < config->N; ++col) {
            tile = block[BLKIDX(row, col, config->N)];
            if (tile == config->tile_na)
              continue;
            won = true;
            for (n = 1; n < config->K; ++n) {
                char slot = block[BLKIDX(row + n, col, config->N)];
                if (slot != tile) {
                    won = false;
                    break;
                }
            }
            if (won)
              return tile;
        }
    }
    for (row = 0; row < config->M - config->K + 1; ++row) {
        for (col = 0; col < config->N - config->K + 1; ++col) {
            tile = block[BLKIDX(row, col, config->N)];
            if (tile == config->tile_na)
              continue;
            won = true;
            for (n = 1; n < config->K; ++n) {
                char slot = block[BLKIDX(row + n, col + n, config->N)];
                if (slot != tile) {
                    won = false;
                    break;
                }
            }
            if (won)
              return tile;
        }
    }
    for (row = config->M - 1; row >= config->K - 1; --row) {
        for (col = 0; col < config->N - config->K + 1; ++col) {
            tile = block[BLKIDX(row, col, config->N)];
            if (tile == config->tile_na)
              continue;
            won = true;
            for (n = 1; n < config->K; ++n) {
                char slot = block[BLKIDX(row - n, col + n, config->N)];
                if (slot != tile) {
                    won = false;
                    break;
                }
            }
            if (won)
              return tile;
        }
    }
    return config->tile_na;
}

// returns 1 or -1 if player 1 or 2, respectively, created a zigzagzoe for given node
// returns 0, otherwise
static int8_t z3_node_zzz(z3_config_t const *config, z3_node_t const node) {
    char *mega = z3_node_mega(node);
    char winner = z3_block_won(config, mega);
    if (winner == config->tile_p1)
      return 1;
    if (winner == config->tile_p2)
      return -1;
    return 0;
}

// returns potency for given player in a block (a sub-board or the mega-board)
static uint8_t  z3_block_potency_player(z3_config_t const *config, char const *block, player_t player) {
    char tile_p = z3_tile_player(config, player);
    char tile_na = config->tile_na;
    bool potential = true;
    uint8_t potency = 0;
    int8_t row, col, n;
    for (row = 0; row < config->M; ++row) {
        for (col = 0; col < config->N - config->K + 1; ++col) {
            potential = true;
            for (n = 0; n < config->K; ++n) {
                char slot = block[BLKIDX(row, col + n, config->N)];
                if (slot != tile_p && slot != tile_na) {
                    potential = false;
                    break;
                }
            }
            if (potential)
              ++potency;
        }
    }
    for (row = 0; row < config->M - config->K + 1; ++row) {
        for (col = 0; col < config->N; ++col) {
            potential = true;
            for (n = 0; n < config->K; ++n) {
                char slot = block[BLKIDX(row + n, col, config->N)];
                if (slot != tile_p && slot != tile_na) {
                    potential = false;
                    break;
                }
            }
            if (potential)
              ++potency;
        }
    }
    for (row = 0; row < config->M - config->K + 1; ++row) {
        for (col = 0; col < config->N - config->K + 1; ++col) {
            potential = true;
            for (n = 0; n < config->K; ++n) {
                char slot = block[BLKIDX(row + n, col + n, config->N)];
                if (slot != tile_p && slot != tile_na) {
                    potential = false;
                    break;
                }
            }
            if (potential)
              ++potency;
        }
    }
    for (row = config->M - 1; row >= config->K - 1; --row) {
        for (col = 0; col < config->N - config->K + 1; ++col) {
            potential = true;
            for (n = 0; n < config->K; ++n) {
                char slot = block[BLKIDX(row - n, col + n, config->N)];
                if (slot != tile_p && slot != tile_na) {
                    potential = false;
                    break;
                }
            }
            if (potential)
              ++potency;
        }
    }
    return potency;
}

// returns potency for given player on mega-board for given node
static uint8_t  z3_mega_potency_player(z3_config_t const *config, z3_node_t const node, player_t player) {
    size_t nslots_block = config->M * config->N;
    char *edit = malloc(nslots_block);
    memcpy(edit, z3_node_mega(node), nslots_block);
    char *blocks = z3_node_blocks(config, node);
    for (uint8_t r = 0; r < config->M; ++r) {
        for (uint8_t c = 0; c < config->N; ++c) {
            if (edit[BLKIDX(r, c, config->N)] != config->tile_na)
                continue;
            uint8_t block_potency = z3_block_potency_player(config, &blocks[MEGIDX(r, c, 0, 0, config->M, config->N)], player);
            if (!block_potency)
                edit[BLKIDX(r, c, config->N)] = config->tile_clog;
        }
    }
    uint8_t potency = z3_block_potency_player(config, edit, player);
    free(edit);
    return potency;
}

// returns maximum possible potency for either player in any block
static uint8_t  z3_node_potency_max(z3_config_t const *config) {
    z3_node_t root = z3_node_root(config, 0);
    uint8_t potency = z3_mega_potency_player(config, root, P_OAKLEY);
    free(root);
    return potency;
}

// returns the sum of the potencies of the sub-boards for the given player
static uint8_t  z3_subsum_potency_player(z3_config_t const *config, z3_node_t const node, player_t player) {
    size_t nblocks = config->M * config->N;
    uint8_t potency_max = z3_node_potency_max(config);
    uint8_t potency = 0;
    for (size_t b = 0; b < nblocks; ++b) {
        char *block = z3_node_block(config, node, b);
        char tile_player = z3_block_won(config, block);
        player_t player_block_won = z3_player_tile(config, tile_player);
        if (player_block_won == player)
            potency += potency_max;
        else if (player_block_won == P_DAKOTA)
            potency += z3_block_potency_player(config, block, player);
    }
    return potency;
}

// prints the given node to standard output
static void z3_node_print(z3_config_t const *config, z3_node_t const node) {
    char *mega = z3_node_mega(node);
    for (uint8_t row = 0; row < config->M; ++row) {
        for (uint8_t col = 0; col < config->N; ++col) {
            char tile = mega[BLKIDX(row, col, config->N)];
            if (tile == config->tile_p1)
                printf("%s%s %c %s", ANSI.bold, ANSI.red, tile, ANSI.reset);
            else if (tile == config->tile_p2)
                printf("%s%s %c %s", ANSI.bold, ANSI.blue, tile, ANSI.reset);
            else
                printf(" %c ", tile);
        }
        printf("\n");
    }
    printf("\n");
    uint8_t previous = z3_node_previous(node);
    char *blocks = z3_node_blocks(config, node);
    uint8_t opt_count = 0;
    for (uint8_t row = 0; row < config->M * config->M; ++row) {
        uint8_t R = row / config->M;
        uint8_t r = row % config->M;
        if (R && !r) {
            printf("%s", ANSI.green);
            for (uint8_t brk = 0; brk < config->N * config->N; ++brk) {
                if (brk / config->N  && !(brk % config->N))
                    printf("|");
                printf("---");
            }
            printf(ANSI.reset);
            printf("\n");
        }
        for (uint8_t col = 0; col < config->N * config->N; ++col) {
            uint8_t C = col / config->N;
            uint8_t c = col % config->N;
            if (C && !c)
                printf("%s|%s", ANSI.green, ANSI.reset);
            char tile = blocks[MEGIDX(R, C, r, c, config->M, config->N)];
            if (BLKIDX(R, C, config->N) == previous && tile == config->tile_na)
                printf("%s%s %c %s", ANSI.bold, ANSI.yellow, 'a' + opt_count++, ANSI.reset);
            else if (tile == config->tile_p1)
                printf("%s%s %c %s", ANSI.bold, ANSI.red, tile, ANSI.reset);
            else if (tile == config->tile_p2)
                printf("%s%s %c %s", ANSI.bold, ANSI.blue, tile, ANSI.reset);
            else
                printf(" %c ", tile);
        }
        printf("\n");
    }
    //printf("\n");
}


//**********************//
//  'game.h' functions  //
//______________________//


// returns 'true' if given node has no children (or options)
static bool z3_leaf(game_t const *game, node_t const node) {
    return z3_node_stale(game->config, node) ||
             z3_node_zzz(game->config, node);
}

// returns array of children of passed node
// stores length of array in '*noffspring'
static node_t *z3_spawn(game_t const *game, node_t const node_raw, size_t * const noffspring) {
    z3_config_t const *config = game->config;
    z3_node_t const node = node_raw;
    uint8_t previous = z3_node_previous(node);
    char *block = z3_node_block(config, node, previous);
    char tile = z3_node_player1(config, node) ? config->tile_p1 : config->tile_p2;
    z3_node_t child = NULL;
    char *block_child = NULL;
    z3_node_t offspring_buf[config->M * config->N];
    *noffspring = 0;
    for (uint8_t r = 0; r < config->M; ++r) {
        for (uint8_t c = 0; c < config->N; ++c) {
            if (block[BLKIDX(r, c, config->N)] == config->tile_na) {
                child = malloc(game_width(game));
                memcpy(child, node, game_width(game));
                *(uint8_t *)child = r * config->N + c;
                block_child = z3_node_block(config, child, previous);
                block_child[BLKIDX(r, c, config->N)] = tile;
                offspring_buf[(*noffspring)++] = child;
            }
        }
    }
    node_t *offspring = malloc(*noffspring * sizeof(node_t));
    memcpy(offspring, offspring_buf, *noffspring * sizeof(node_t));
    char *mega = z3_node_mega(node);
    char *mega_child = NULL;
    if (mega[previous] == config->tile_na) {
        for (size_t i = 0; i < *noffspring; ++i) {
            child = offspring[i];
            block_child = z3_node_block(config, child, previous);
            if (z3_block_won(config, block_child) != config->tile_na) {
                mega_child = z3_node_mega(child);
                mega_child[previous] = tile;
            }
        }
    }
    return offspring;
}

// returns player winning given node 
static player_t z3_winner(game_t const *game, node_t const node) {
    z3_config_t *config = game->config;
    int8_t winner = z3_node_zzz(config, node);
    if (winner)
        return winner == P_OAKLEY ? P_OAKLEY : P_TAYLOR;
    if (z3_node_stale(config, node)) {
        bool player1 = z3_node_player1(config, node);
        if (config->mate == Z3_WIN)
            return player1 ? P_OAKLEY : P_TAYLOR;
        if (config->mate == Z3_DRAW)
            return P_DAKOTA;
        return player1 ? P_TAYLOR : P_OAKLEY;
    }
    return P_DAKOTA;
}

// returns maximum heuristic value of 'z3_heuristic'
static int  z3_heuristic_max(z3_config_t const *config) {
    uint8_t potency_max = z3_node_potency_max(config);
    //return (1 + config->M * config->N) * potency_max + (1 + z3_nslots(config));
    return potency_max * config->M * config->N * potency_max + (1 + z3_nslots(config));
}

static int  z3_heuristic_max2(z3_config_t const *config) {
    uint8_t potency_max = z3_node_potency_max(config);
    return potency_max + config->M * config->N * potency_max + (1 + z3_nslots(config));
}

static int  z3_heuristic_max3(z3_config_t const *config) {
    uint8_t potency_max = z3_node_potency_max(config);
    return config->M * config->N * potency_max + (1 + z3_nslots(config));
}

// returns heuristic value of given node
static int  z3_heuristic(game_t const *game, node_t const node_raw) {
    z3_config_t *config = game->config;
    z3_node_t const node = node_raw;
    uint8_t potency_max = z3_node_potency_max(config);
    //int decisive = (1 + config->M * config->N) * potency_max + (1 + z3_node_nempty(config, node));
    int decisive = potency_max * config->M * config->N * potency_max + (1 + z3_node_nempty(config, node));
    player_t winner = z3_winner(game, node_raw);
    if (winner)
        return winner * decisive;
    if (z3_node_stale(config, node))
        return 0;
    uint8_t potency_mega_p1 = z3_mega_potency_player(config, node, P_OAKLEY);
    uint8_t potency_mega_p2 = z3_mega_potency_player(config, node, P_TAYLOR);
    uint8_t potency_subsum_p1 = z3_subsum_potency_player(config, node, P_OAKLEY);
    uint8_t potency_subsum_p2 = z3_subsum_potency_player(config, node, P_TAYLOR);
    //return potency_mega_p1 - potency_mega_p2 + potency_subsum_p1 - potency_subsum_p2;
    return potency_mega_p1 * potency_subsum_p1 - potency_mega_p2 * potency_subsum_p2;
/*
    ///  20170331 - deprecated  ///
    int blocks_p1 = z3_blocks_owned(config, node, P_OAKLEY);
    int blocks_p2 = z3_blocks_owned(config, node, P_TAYLOR);
    return blocks_p1 - blocks_p2;
*/
/*
    ///  20170330 - deprecated  ///
    uint8_t potency_max = z3_node_potency_max(config);
    // territory * maxpotency + maxpotency + nempty
    int decisive = potency_max * config->M * config->N + potency_max + (1 + z3_node_nempty(config, node));
    uint8_t owned_p1_scaled = potency_max * z3_blocks_owned(config, node, P_OAKLEY);
    uint8_t owned_p2_scaled = potency_max * z3_blocks_owned(config, node, P_TAYLOR);
    uint8_t potency_p1 = z3_mega_potency_player(config, node, P_OAKLEY);
    uint8_t potency_p2 = z3_mega_potency_player(config, node, P_TAYLOR);
    int value = (int)owned_p1_scaled + (int)potency_p1 - (int)owned_p2_scaled - (int)potency_p2;
    return value;
*/
}

static int z3_heuristic2(game_t const *game, node_t const node_raw) {
    z3_config_t *config = game->config;
    z3_node_t const node = node_raw;
    uint8_t potency_max = z3_node_potency_max(config);
    //int decisive = (1 + config->M * config->N) * potency_max + (1 + z3_node_nempty(config, node));
    int decisive = potency_max + config->M * config->N * potency_max + (1 + z3_node_nempty(config, node));
    player_t winner = z3_winner(game, node_raw);
    if (winner)
        return winner * decisive;
    if (z3_node_stale(config, node))
        return 0;
    uint8_t potency_mega_p1 = z3_mega_potency_player(config, node, P_OAKLEY);
    uint8_t potency_mega_p2 = z3_mega_potency_player(config, node, P_TAYLOR);
    uint8_t potency_subsum_p1 = z3_subsum_potency_player(config, node, P_OAKLEY);
    uint8_t potency_subsum_p2 = z3_subsum_potency_player(config, node, P_TAYLOR);
    return potency_mega_p1 - potency_mega_p2 + potency_subsum_p1 - potency_subsum_p2;
    //return potency_subsum_p1 - potency_subsum_p2;
}

static int z3_heuristic3(game_t const *game, node_t const node_raw) {
    z3_config_t *config = game->config;
    z3_node_t const node = node_raw;
    uint8_t potency_max = z3_node_potency_max(config);
    //int decisive = (1 + config->M * config->N) * potency_max + (1 + z3_node_nempty(config, node));
    int decisive = config->M * config->N * potency_max + (1 + z3_node_nempty(config, node));
    player_t winner = z3_winner(game, node_raw);
    if (winner)
        return winner * decisive;
    if (z3_node_stale(config, node))
        return 0;
    //uint8_t potency_mega_p1 = z3_mega_potency_player(config, node, P_OAKLEY);
    //uint8_t potency_mega_p2 = z3_mega_potency_player(config, node, P_TAYLOR);
    uint8_t potency_subsum_p1 = z3_subsum_potency_player(config, node, P_OAKLEY);
    uint8_t potency_subsum_p2 = z3_subsum_potency_player(config, node, P_TAYLOR);
    //return potency_mega_p1 - potency_mega_p2 + potency_subsum_p1 - potency_subsum_p2;
    return potency_subsum_p1 - potency_subsum_p2;
}

// prints given node to standard output
static void z3_publish(game_t const *game, node_t const node) {
    z3_node_print(game->config, node);
}

// returns array of nodes in same symmetry group as given node
// stores length of array in '*ntwins'
static node_t *z3_clone(game_t const *game, node_t const node_raw, size_t * const ntwins) {
    z3_config_t const *config = game->config;
    z3_node_t const node = node_raw;
    uint8_t previous = z3_node_previous(node);
    char *mega = z3_node_mega(node);
    char *blocks = z3_node_blocks(config, node);
    uint8_t pR = previous / config->N;
    uint8_t pC = previous % config->N;
    z3_node_t row_flip = malloc(game_width(game));
    z3_node_t col_flip = malloc(game_width(game));
    z3_node_t rcb_flip = malloc(game_width(game));
    *(uint8_t *)row_flip = BLKIDX(config->M - (1 + pR), pC, config->N);
    *(uint8_t *)col_flip = BLKIDX(pR, config->N - (1 + pC), config->N);
    *(uint8_t *)rcb_flip = BLKIDX(config->M - (1 + pR), config->N - (1 + pC), config->N);
    char *row_flip_mega = z3_node_mega(row_flip);
    char *col_flip_mega = z3_node_mega(col_flip);
    char *rcb_flip_mega = z3_node_mega(rcb_flip);
    for (uint8_t row = 0; row < config->M; ++row) {
        for (uint8_t col = 0; col < config->N; ++col) {
            row_flip_mega[BLKIDX(config->M - (1 + row), col, config->N)] =
                mega[BLKIDX(row, col, config->N)];
            col_flip_mega[BLKIDX(row, config->N - (1 + col), config->N)] = 
                mega[BLKIDX(row, col, config->N)];
            rcb_flip_mega[BLKIDX(config->M - (1 + row), config->N - (1 + col), config->N)] = 
                mega[BLKIDX(row, col, config->N)];
        }
    }
    char *row_flip_blocks = z3_node_blocks(config, row_flip);
    char *col_flip_blocks = z3_node_blocks(config, col_flip);
    char *rcb_flip_blocks = z3_node_blocks(config, rcb_flip);
    for (uint8_t row = 0; row < config->M * config->M; ++row) {
        uint8_t R = row / config->M;
        uint8_t r = row % config->M;
        for (uint8_t col = 0; col < config->N * config->N; ++col) {
            uint8_t C = col / config->N;
            uint8_t c = col % config->N;
            row_flip_blocks[MEGIDX(config->M - (1 + R), C,
                                   config->M - (1 + r), c,
                                   config->M, config->N)] =
                blocks[MEGIDX(R, C, r, c, config->M, config->N)];
            col_flip_blocks[MEGIDX(R, config->N - (1 + C),
                                   r, config->N - (1 + c),
                                   config->M, config->N)] =
                blocks[MEGIDX(R, C, r, c, config->M, config->N)];
            rcb_flip_blocks[MEGIDX(config->M - (1 + R), config->N - (1 + C),
                                   config->M - (1 + r), config->N - (1 + c),
                                   config->M, config->N)] =
                blocks[MEGIDX(R, C, r, c, config->M, config->N)];
        }
    }
    *ntwins = 3;
    node_t *twins = malloc(*ntwins * sizeof(node_t));
    twins[0] = row_flip;
    twins[1] = col_flip;
    twins[2] = rcb_flip;
    return twins;
}

/*
///  20170331 created & deprecated  ///

static game_t const *game_stratify;

static int stratify_cmp_p1(void const *addr1, void const *addr2) {
    return game_stratify->heuristic(game_stratify, *(node_t *)addr2) - game_stratify->heuristic(game_stratify, *(node_t *)addr1);
}

static int stratify_cmp_p2(void const *addr1, void const *addr2) {
    return game_stratify->heuristic(game_stratify, *(node_t *)addr1) - game_stratify->heuristic(game_stratify, *(node_t *)addr2);
}

static void z3_stratify(game_t const *game, node_t * const offspring, size_t const noffspring) {
    game_stratify = game;
    bool player1 = !z3_node_player1(game->config, offspring[0]);
    if (player1)
        qsort(offspring, noffspring, sizeof(node_t), &stratify_cmp_p1);
    else
        qsort(offspring, noffspring, sizeof(node_t), &stratify_cmp_p2);
}
*/


//****************************//
//  PUBLIC LIBRARY FUNCTIONS  //
//____________________________//


// returns 'z3_t *' game initialized with default values
z3_t  *z3_init(bool player1_ai, bool player2_ai) {
    uint8_t block_init = nkrand(INIT_M * INIT_N);
    return z3_init_w(INIT_M, INIT_N, INIT_K, block_init, INIT_STALE,
                     INIT_TILE_P1, INIT_TILE_P2, INIT_TILE_NA, INIT_TILE_CLOG,
                     INIT_DEPTH_AI, player1_ai, player2_ai);
}

z3_t  *z3_init_h2(bool player1_ai, bool player2_ai) {
    uint8_t block_init = nkrand(INIT_M * INIT_N);
    return z3_init_h2_w(INIT_M, INIT_N, INIT_K, block_init, INIT_STALE,
                     INIT_TILE_P1, INIT_TILE_P2, INIT_TILE_NA, INIT_TILE_CLOG,
                     INIT_DEPTH_AI, player1_ai, player2_ai);
}

// returns 'z3_t *' game initialized with passed values
z3_t  *z3_init_w(uint8_t M, uint8_t N, uint8_t K, uint8_t block_init, z3_stale_t mate,
                 char tile_p1, char tile_p2, char tile_na, char tile_clog,
                 uint8_t depth, bool player1_ai, bool player2_ai) {
    z3_config_t config_raw = {.M = M, .N = N, .K = K,
                              .mate = mate,
                              .tile_p1 = tile_p1,
                              .tile_p2 = tile_p2,
                              .tile_na = tile_na,
                              .tile_clog = tile_clog
                              };
    z3_config_t *config = malloc(sizeof(z3_config_t));
    memcpy(config, &config_raw, sizeof(z3_config_t));
    z3_node_t root = z3_node_root(config, block_init);
    z3_t *game = game_init(
                     root,
                     z3_node_width(config),
                     z3_heuristic_max2(config),
                     depth,
                     player1_ai,
                     player2_ai,
                     &z3_leaf,
                     &z3_spawn,
                     &z3_winner,
                     &z3_heuristic2,
                     &z3_publish,
                     &z3_clone,
                     NULL // &z3_stratify
                 );
    free(root);
    game->config = config;
    return game;
}

z3_t  *z3_init_h2_w(uint8_t M, uint8_t N, uint8_t K, uint8_t block_init, z3_stale_t mate,
                 char tile_p1, char tile_p2, char tile_na, char tile_clog,
                 uint8_t depth, bool player1_ai, bool player2_ai) {
    z3_config_t config_raw = {.M = M, .N = N, .K = K,
                              .mate = mate,
                              .tile_p1 = tile_p1,
                              .tile_p2 = tile_p2,
                              .tile_na = tile_na,
                              .tile_clog = tile_clog
                              };
    z3_config_t *config = malloc(sizeof(z3_config_t));
    memcpy(config, &config_raw, sizeof(z3_config_t));
    z3_node_t root = z3_node_root(config, block_init);
    z3_t *game = game_init(
                     root,
                     z3_node_width(config),
                     z3_heuristic_max2(config),
                     depth,
                     player1_ai,
                     player2_ai,
                     &z3_leaf,
                     &z3_spawn,
                     &z3_winner,
                     &z3_heuristic2,
                     &z3_publish,
                     &z3_clone,
                     NULL // &z3_stratify
                 );
    free(root);
    game->config = config;
    return game;
}

// resets given game to initial values with passed initial sub-board for player 1
void  z3_reset(z3_t *game) {
    game_reset(game);
}

// frees game from dynamically-allocated memory
void  z3_free(z3_t *game) {
    game_free(game);
}

void z3_advance_ai2(z3_t *game1, z3_t *game2) {
    player_t player = game_player(game1);
    node_t move = NULL;
    if ((player == P_OAKLEY && game_player1_ai(game1)) || (player == P_TAYLOR && game_player2_ai(game1)))
        move = negamax_move(game_negamax(game1), game_state(game1), player, game_depth(game1), NULL);
    else
        move = negamax_move(game_negamax(game2), game_state(game1), player, game_depth(game2), NULL);
    //game1->publish(game1, move);
    game_move(game1, move);
    free(move);
}

static void z3_play_ai2_main(z3_t *game1, z3_t *game2, int zzz_count[2]) {
    z3_config_t *config1 = game1->config;
    int move_count = 0;
    while (!game1->leaf(game1, game_state(game1))) {
        game_publish_state(game1);
        //game1->publish(game1, game_state(game1));
        z3_advance_ai2(game1, game2);
        //printf("eval = %d; heuristic = %d\n", -1 * game_player(game) * game->data->eval, game->heuristic(game, game_state(game)));
        if (move_count % 2) {
        }
        ++move_count;
    }
    //game_moves_print(game1);
    game_publish_state(game1);
    //game1->publish(game1, game_state(game1));
    unsigned game1_nbytes = (unsigned)(negamax_nbytes(game_negamax(game1)));
    unsigned game2_nbytes = (unsigned)(negamax_nbytes(game_negamax(game2)));
    unsigned game1_size = (unsigned)negamax_ttable_size(game_negamax(game1));
    unsigned game2_size = (unsigned)negamax_ttable_size(game_negamax(game2));
    player_t winner = game1->winner(game1, game_state(game1));
    game_score_add(game1, winner);
    int8_t zzz = z3_node_zzz(game1->config, game_state(game1));
    if (zzz)
        ++zzz_count[winner == P_OAKLEY ? 0 : 1];
    printf("\n");
    printf("%s\n", winner == P_OAKLEY ? "player 1 wins!" : winner == P_TAYLOR ? "player 2 wins!" : "it's a draw!");
    printf("memory: (%f + %f) MiB = %f MiB\n", (double)game1_nbytes/1024/1024, (double)game2_nbytes/1024/1024, ((double)game1_nbytes + game2_nbytes)/1024/1024);
    printf("ttable:  %u, %u, 1:2 -> %f, 2:1 -> %f\n", game1_size, game2_size, (double)game1_size / game2_size, (double)game2_size / game1_size);
    printf("nmoves:\t%d\n", move_count);
    printf("score:\t%u - %u\n", game_score(game1, P_OAKLEY), game_score(game1, P_TAYLOR));
    printf("zzz:\t%d - %d\n", zzz_count[0], zzz_count[1]);
    printf("\n");
    //sleep(2);
    uint8_t block_init = nkrand(config1->M * config1->N);
    z3_node_t root = z3_node_root(config1, block_init);
    game_reset_root(game1, root);
    free(root);
}

void z3_play_ai2(z3_t *game1, z3_t *game2) {
    int zzz_count[2] = {0, 0};
    z3_play_ai2_main(game1, game2, zzz_count);
    while (game_prompt_rematch())
        z3_play_ai2_main(game1, game2, zzz_count);
}

z3_t *z3_iOS_SetupGame_Human(int M, int N, int K, int initBlock, int staleMode) {
    z3_stale_t stale = staleMode == 1 ? Z3_WIN : staleMode == 2 ? Z3_LOSS : Z3_DRAW;
    return z3_init_w(M, N, K, initBlock - 1, stale, 
                     INIT_TILE_P1, INIT_TILE_P2, INIT_TILE_NA, INIT_TILE_CLOG,
                     INIT_DEPTH_AI, false, false);
}

//z3_t *z3_iOS_SetupGame_AI(int M, int N, int K, int initBlock, int stateMode, int playerAI, int difficulty);

int *z3_iOS_Move_Human(z3_t *humanGame, int tileNumber, int playerNumber) {
    z3_config_t *config = humanGame->config;
    --tileNumber;
    const char playerTile = playerNumber == 1 ? config->tile_p1 : config->tile_p2;

    int blockIndex = tileNumber / (config->M * config->N) + (tileNumber % (config->N * config->N)) / config->N;
    int subBlockIndex = ((tileNumber / (config->N * config->N)) % config->N) * config->N + tileNumber % config->N;
    int tileIndex = blockIndex * config->N * config->M + subBlockIndex;
    uint8_t checkBlockIndex = z3_node_previous(game_state(humanGame));
    if (blockIndex != checkBlockIndex)
        return NULL;
    if (subBlockIndex >= config->M * config->N)
        return NULL;

    z3_node_t currentState = z3_node_dup(humanGame, game_state(humanGame));
    z3_node_t newState = z3_node_dup(humanGame, game_state(humanGame));

    char *newBlocks = z3_node_blocks(config, newState);
    if (newBlocks[tileIndex] != config->tile_na)
        return NULL;
    newBlocks[tileIndex] = playerTile;
    *(uint8_t *)newState = subBlockIndex;

    size_t noptions;
    node_t *options = humanGame->spawn(humanGame, currentState, &noptions);
    size_t optionIndex = config->M * config->N;
    for (size_t i = 0; i < noptions; ++i) {
        if (!memcmp(newBlocks, z3_node_blocks(config, options[i]), config->M * config->M * config->N * config->N)) {
            optionIndex = i;
            break;
        }
    }

    if (optionIndex == config->M * config->N)
        return NULL;

    bool legal = game_move(humanGame, options[optionIndex]);
    if (!legal)
        return NULL;
    free(newState);
    newState = z3_node_dup(humanGame, options[optionIndex]);

    for (size_t i = 0; i < noptions; ++i)
        free(options[i]);
    free(options);

    int *gameInfo = malloc(4);
    gameInfo[0] = 0;
    if (humanGame->leaf(humanGame, newState)) {
        player_t winner = humanGame->winner(humanGame, newState);
        switch (winner) {
            case P_OAKLEY:
                gameInfo[0] = 1;
                break;
            case P_TAYLOR:
                gameInfo[0] = 2;
                break;
            case P_DAKOTA:
                gameInfo[0] = 3;
        }
    }

    char *currentMega = z3_node_mega(currentState);
    char *newMega = z3_node_mega(newState);
    if (!memcmp(currentMega, newMega, config->M * config->N))
        gameInfo[1] = 0;
    else
        gameInfo[1] = 1 + blockIndex;

    free(currentState);
    free(newState);

    gameInfo[2] = gameInfo[1] ? playerNumber : 0;
    gameInfo[3] = gameInfo[0] ? 0 : 1 + subBlockIndex;
    return gameInfo;
}

//int *z3_iOS_Move_AI(int tileNumber, int playerNumber);

















