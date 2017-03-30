
// heuristic factors: win, prolong loss, mega weight (connectn concept)
// stratify factors: sort by heuristic, then by look at forcing moves first (forceful prioritized over heuristic)
// sterile == stalemate == draw; stratify should deprioritize stale nodes

//definitions
//m,n,k
//potency
//board
//previous
//sub-board
//mega-board
//block

#include "zigzagzoe.h"
#include "negamax.h"
#include "ansicolor.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include <time.h>

#define RCIDX(r,c,nc) ( (r)*(nc) + (c) )
#define RC2IDX(R,C,r,c,nr,nc) ( (R)*(nc)*(nr)*(nc) + (C)*(nr)*(nc) + (r)*(nc) + (c) )

#define DFLT_NROWS 3
#define DFLT_NCOLUMNS 3
//#define DFLT_NCOLUMNS 4
#define DFLT_NCONNECT 3
#define DFLT_TILE_P1 'X'
#define DFLT_TILE_P2 'O'
#define DFLT_EMPTY_TILE '-'
#define DFLT_DEPTH 10

typedef struct _z3_config_t {
    uint8_t nrows;
    uint8_t ncolumns;
    uint8_t nconnect;
    char tile_p1;
    char tile_p2;
    char tile_na;

//    bool player1_ai;
//    bool player2_ai;
//    uint8_t depth;
//    negamax_t *negamax;

//    int eval;
    uint8_t previous;
} z3_config_t;

// stores previous, mega-block, & board
// width varies with nrows, ncolumns, & nconnect
typedef char *z3_node_t;
//typedef node_t z3_node_t;


//********************//
//  STATIC FUNCTIONS  //
//____________________//


//********************//
//  inline functions  //
//____________________//


// returns number of slots on board
static inline uint8_t z3_nslots(z3_config_t const *config) {
    return config->nrows * config->ncolumns * config->nrows * config->ncolumns;
}

// returns 'z3_node_t' length in terms of M and N
static inline size_t z3_node_width_rc(uint8_t r, uint8_t c) {
    return (size_t)(1 + (r * c) + (r * c * r * c));
}

// 'z3_node_width_rc' wrapper
static inline size_t z3_node_width(z3_config_t const *config) {
    return z3_node_width_rc(config->nrows, config->ncolumns);
}

// extracts 'previous' from 'z3_node_t' (index of sub-board for current player)
static inline uint8_t z3_node_previous(z3_node_t const node) {
    return *(uint8_t *)node;
}

// extracts mega-board from 'z3_node_t' (MxN board recording won sub-boards)
static inline char *z3_node_mega(z3_node_t node) {
    return ++node;
}

// extracts main board from 'z3_node_t'
static inline char *z3_node_blocks(z3_config_t const *config, z3_node_t node) {
    return ++node + config->nrows * config->ncolumns;
}

// extracts sub-board for current player from 'z3_node_t'
static inline char *z3_node_block(z3_config_t const *config, z3_node_t const node, uint8_t previous) {
    char *blocks = z3_node_blocks(config, node);
    uint8_t row = previous / config->ncolumns;
    uint8_t col = previous % config->ncolumns;
    return &blocks[RC2IDX(row, col, 0, 0, config->nrows, config->ncolumns)];
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

// returns root 'z3_node_t' given initial sub-board for player 1
static inline z3_node_t z3_root(z3_config_t const *config, uint8_t previous_init) {
    size_t width = z3_node_width(config);
    z3_node_t root = malloc(width * sizeof(char));
    *(uint8_t *)root = previous_init;
    for (size_t slot = 1; slot < width; ++slot)
        root[slot] = config->tile_na;
    return root;
}

// returns 'true' if the sub-board for the current player has no options
static inline bool z3_node_stale(z3_config_t const *config, z3_node_t const node) {
    uint8_t previous = z3_node_previous(node);
    char *block = z3_node_block(config, node, previous);
    for (uint8_t r = 0; r < config->nrows; ++r)
        for (uint8_t c = 0; c < config->ncolumns; ++c)
            if (block[RCIDX(r, c, config->ncolumns)] == config->tile_na)
                return false;
    return true;
}

// returns true if player to move for given tile is player 1
static inline bool z3_node_p1(z3_config_t const *config, z3_node_t const node) {
    return !((z3_nslots(config) - z3_node_nempty(config, node)) % 2);
}

// returns tile associated with player
static inline char z3_tile_player(z3_config_t const *config, player_t player) {
    return player == OAKLEY ? config->tile_p1 : config->tile_p2;
}


//*******************//
//  helper functions //
//___________________//


// returns potency for given player in a block (a sub-board or the mega-board)
static uint8_t z3_block_potency_player(z3_config_t const *config, char const *block, player_t player) {
    char tile_p = z3_tile_player(config, player);
    char tile_na = config->tile_na;
    bool potential = true;
    uint8_t potency = 0;
    int8_t row, col, n;
    for (row = 0; row < config->nrows; ++row) {
        for (col = 0; col < config->ncolumns - config->nconnect + 1; ++col) {
            potential = true;
            for (n = 0; n < config->nconnect; ++n) {
                char slot = block[RCIDX(row, col + n, config->ncolumns)];
                if (slot != tile_p && slot != tile_na) {
                    potential = false;
                    break;
                }
            }
            if (potential)
              ++potency;
        }
    }
    for (row = 0; row < config->nrows - config->nconnect + 1; ++row) {
        for (col = 0; col < config->ncolumns; ++col) {
            potential = true;
            for (n = 0; n < config->nconnect; ++n) {
                char slot = block[RCIDX(row + n, col, config->ncolumns)];
                if (slot != tile_p && slot != tile_na) {
                    potential = false;
                    break;
                }
            }
            if (potential)
              ++potency;
        }
    }
    for (row = 0; row < config->nrows - config->nconnect + 1; ++row) {
        for (col = 0; col < config->ncolumns - config->nconnect + 1; ++col) {
            potential = true;
            for (n = 0; n < config->nconnect; ++n) {
                char slot = block[RCIDX(row + n, col + n, config->ncolumns)];
                if (slot != tile_p && slot != tile_na) {
                    potential = false;
                    break;
                }
            }
            if (potential)
              ++potency;
        }
    }
    for (row = config->nrows - 1; row >= config->nconnect - 1; --row) {
        for (col = 0; col < config->ncolumns - config->nconnect + 1; ++col) {
            potential = true;
            for (n = 0; n < config->nconnect; ++n) {
                char slot = block[RCIDX(row - n, col + n, config->ncolumns)];
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
static uint8_t z3_node_potency_player(z3_config_t const *config, z3_node_t const node, player_t player) {
    size_t nslots_block = config->nrows * config->ncolumns;
    char *edit = malloc(nslots_block);
    memcpy(edit, z3_node_mega(node), nslots_block);
    char *blocks = z3_node_blocks(config, node);
    for (uint8_t r = 0; r < config->nrows; ++r) {
        for (uint8_t c = 0; c < config->ncolumns; ++c) {
            if (edit[RCIDX(r, c, config->ncolumns)] != config->tile_na)
                continue;
            uint8_t block_potency = z3_block_potency_player(config, &blocks[RC2IDX(r, c, 0, 0, config->nrows, config->ncolumns)], player);
            if (!block_potency)
                edit[RCIDX(r, c, config->ncolumns)] = '$'; // add `tile_unable` or something to config and create DEFAULT for init
        }
    }
    uint8_t potency = z3_block_potency_player(config, edit, player);
    free(edit);
    return potency;
}

// returns maximum possible potency for either player in any block
static uint8_t z3_node_potency_max(z3_config_t const *config) {
    z3_node_t root = z3_root(config, 0);
    uint8_t potency = z3_node_potency_player(config, root, OAKLEY);
    free(root);
    return potency;
}

// returns maximum heuristic value of 'z3_heuristic'
static int z3_heuristic_win(z3_config_t const *config) {
    return z3_node_potency_max(config) * config->nrows * config->ncolumns + z3_node_potency_max(config) + (1 + z3_nslots(config));
}

//TODO
static void z3_node_print(z3_config_t const *config, z3_node_t const node) {
    printf(ANSI.erase);

    char *mega = z3_node_mega(node);
    for (uint8_t row = 0; row < config->nrows; ++row) {
        for (uint8_t col = 0; col < config->ncolumns; ++col) {
            char tile = mega[RCIDX(row, col, config->ncolumns)];
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
    for (uint8_t row = 0; row < config->nrows * config->nrows; ++row) {
        uint8_t R = row / config->nrows;
        uint8_t r = row % config->nrows;
        if (R && !r) {
            //printf("%s%s", ANSI.bold, ANSI.green);
            printf("%s", ANSI.green);
            for (uint8_t brk = 0; brk < config->ncolumns * config->ncolumns; ++brk) {
                if (brk / config->ncolumns  && !(brk % config->ncolumns))
                    printf("|");
                printf("---");
            }
            printf(ANSI.reset);
            printf("\n");
        }
        for (uint8_t col = 0; col < config->ncolumns * config->ncolumns; ++col) {
            uint8_t C = col / config->ncolumns;
            uint8_t c = col % config->ncolumns;
            if (C && !c)
                //printf("%s%s|%s", ANSI.bold, ANSI.green, ANSI.reset);
                printf("%s|%s", ANSI.green, ANSI.reset);
            char tile = blocks[RC2IDX(R, C, r, c, config->nrows, config->ncolumns)];
            if (RCIDX(R, C, config->ncolumns) == config->previous && RCIDX(r, c, config->ncolumns) == previous) {
                printf("%s%s %c %s", ANSI.bold, ANSI.green, tile, ANSI.reset);
            } else if (RCIDX(R, C, config->ncolumns) == previous && tile == config->tile_na)
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
    printf("\n");
}

// returns tile of winner of given block or empty tile if no winner
static char z3_block_won(z3_config_t const *config, char const *block) {
    char tile = config->tile_na;
    bool won = true;
    int8_t row, col, n;
    for (row = 0; row < config->nrows; ++row) {
        for (col = 0; col < config->ncolumns - config->nconnect + 1; ++col) {
            tile = block[RCIDX(row, col, config->ncolumns)];
            if (tile == config->tile_na)
              continue;
            won = true;
            for (n = 1; n < config->nconnect; ++n) {
                char slot = block[RCIDX(row, col + n, config->ncolumns)];
                if (slot != tile) {
                    won = false;
                    break;
                }
            }
            if (won)
              return tile;
        }
    }
    for (row = 0; row < config->nrows - config->nconnect + 1; ++row) {
        for (col = 0; col < config->ncolumns; ++col) {
            tile = block[RCIDX(row, col, config->ncolumns)];
            if (tile == config->tile_na)
              continue;
            won = true;
            for (n = 1; n < config->nconnect; ++n) {
                char slot = block[RCIDX(row + n, col, config->ncolumns)];
                if (slot != tile) {
                    won = false;
                    break;
                }
            }
            if (won)
              return tile;
        }
    }
    for (row = 0; row < config->nrows - config->nconnect + 1; ++row) {
        for (col = 0; col < config->ncolumns - config->nconnect + 1; ++col) {
            tile = block[RCIDX(row, col, config->ncolumns)];
            if (tile == config->tile_na)
              continue;
            won = true;
            for (n = 1; n < config->nconnect; ++n) {
                char slot = block[RCIDX(row + n, col + n, config->ncolumns)];
                if (slot != tile) {
                    won = false;
                    break;
                }
            }
            if (won)
              return tile;
        }
    }
    for (row = config->nrows - 1; row >= config->nconnect - 1; --row) {
        for (col = 0; col < config->ncolumns - config->nconnect + 1; ++col) {
            tile = block[RCIDX(row, col, config->ncolumns)];
            if (tile == config->tile_na)
              continue;
            won = true;
            for (n = 1; n < config->nconnect; ++n) {
                char slot = block[RCIDX(row - n, col + n, config->ncolumns)];
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
static int8_t z3_node_zzz(z3_config_t const *config, z3_node_t const node) {
    char *mega = z3_node_mega(node);
    char winner = z3_block_won(config, mega);
    if (winner == config->tile_p1)
      return 1;
    if (winner == config->tile_p2)
      return -1;
    return 0;
}

// returns number of sub-boards won by given player on given node
static uint8_t z3_blocks_owned(z3_config_t const *config, z3_node_t node, player_t player) {
    char *mega = z3_node_mega(node);
    char tile = z3_tile_player(config, player);
    uint8_t count = 0;
    for (uint8_t t = 0; t < config->nrows * config->ncolumns; ++t)
        if (mega[t] == tile)
            ++count;
    return count;
}


//**********************//
//  'game.h' functions  //
//______________________//


// returns 'true' if given node has no children
static bool z3_leaf(game_t const *game, node_t const node) {
    return z3_node_stale((z3_config_t *)game->config, (z3_node_t)node) ||
             z3_node_zzz((z3_config_t *)game->config, (z3_node_t)node);
}

static node_t *z3_spawn(game_t const *gm, node_t const nd, size_t * const noffspring) {
    z3_t const *game = (z3_t const *)gm;
    z3_config_t const *config = game->config;
    z3_node_t const node = (z3_node_t const)nd;
    uint8_t previous = z3_node_previous(node);
    char *block = z3_node_block(config, node, previous);
    char tile = z3_node_p1(config, node) ? config->tile_p1 : config->tile_p2;
    z3_node_t child = NULL;
    char *block_child = NULL;
    z3_node_t offspring_buf[config->nrows * config->ncolumns];
    *noffspring = 0;
    for (uint8_t r = 0; r < config->nrows; ++r) {
        for (uint8_t c = 0; c < config->ncolumns; ++c) {
            if (block[RCIDX(r, c, config->ncolumns)] == config->tile_na) {
                child = malloc(game_width(game));
                memcpy(child, node, game_width(game));
                *(uint8_t *)child = r * config->ncolumns + c;
                block_child = z3_node_block(config, child, previous);
                block_child[RCIDX(r, c, config->ncolumns)] = tile;
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
static player_t z3_winner(game_t const *gm, node_t const nd) {
    z3_t const *game = (z3_t const *)gm;
    z3_node_t const node = (z3_node_t const)nd;
    z3_config_t *config = game->config;
    int8_t winner = z3_node_zzz(config, node);
    if (winner)
        return winner == OAKLEY ? OAKLEY : TAYLOR;
    return DAKOTA;
}

//2016dec28 NOTE: change return type to int16_t? (would require game.[ch] update, as well)
static int z3_heuristic(game_t const *gm, node_t const nd) {
    z3_t const *game = (z3_t const *)gm;
    z3_node_t const node = (z3_node_t const)nd;
    z3_config_t *config = game->config;
    uint8_t potency_max = z3_node_potency_max(config);
    // drawpreferability + territory * maxpotency + maxpotency + nempty
    int decisive = potency_max * config->nrows * config->ncolumns + potency_max + (1 + z3_node_nempty(config, node));
    int8_t winner = z3_node_zzz(config, node);
    if (winner)
        return winner * decisive;
    if (z3_node_stale(config, node))
        return 0;
        //return -1 * game_player(gm) * z3_node_nempty(config, node);
    uint8_t owned_p1_scaled = potency_max * z3_blocks_owned(config, node, OAKLEY);
    uint8_t owned_p2_scaled = potency_max * z3_blocks_owned(config, node, TAYLOR);
    uint8_t potency_p1 = z3_node_potency_player(config, node, OAKLEY);
    uint8_t potency_p2 = z3_node_potency_player(config, node, TAYLOR);
    int value = (int)owned_p1_scaled + (int)potency_p1 - (int)owned_p2_scaled - (int)potency_p2;
    return value;
}

static void z3_publish(game_t const *game, node_t const node) {
    z3_node_print((z3_config_t const *)game->config, (z3_node_t const)node);
}

//fixed 2017-03-03
static node_t *z3_clone(game_t const *game, node_t const nd, size_t * const ntwins) {
    //remember to check if the found twin is, in fact, distinguishable from node
    z3_config_t const *config = (z3_config_t const *)game->config;
    z3_node_t const node = (z3_node_t const)nd;
    uint8_t previous = z3_node_previous(node);
    char *mega = z3_node_mega(node);
    char *blocks = z3_node_blocks(config, node);
    uint8_t pR = previous / config->ncolumns;
    uint8_t pC = previous % config->ncolumns;
    z3_node_t row_flip = malloc(game_width(game));
    z3_node_t col_flip = malloc(game_width(game));
    z3_node_t rcb_flip = malloc(game_width(game));
    *(uint8_t *)row_flip = RCIDX(config->nrows - (1 + pR), pC, config->ncolumns);
    *(uint8_t *)col_flip = RCIDX(pR, config->ncolumns - (1 + pC), config->ncolumns);
    *(uint8_t *)rcb_flip = RCIDX(config->nrows - (1 + pR), config->ncolumns - (1 + pC), config->ncolumns);
    char *row_flip_mega = z3_node_mega(row_flip);
    char *col_flip_mega = z3_node_mega(col_flip);
    char *rcb_flip_mega = z3_node_mega(rcb_flip);
    for (uint8_t row = 0; row < config->nrows; ++row) {
        for (uint8_t col = 0; col < config->ncolumns; ++col) {
            row_flip_mega[RCIDX(config->nrows - (1 + row), col, config->ncolumns)] =
                mega[RCIDX(row, col, config->ncolumns)];
            col_flip_mega[RCIDX(row, config->ncolumns - (1 + col), config->ncolumns)] = 
                mega[RCIDX(row, col, config->ncolumns)];
            rcb_flip_mega[RCIDX(config->nrows - (1 + row), config->ncolumns - (1 + col), config->ncolumns)] = 
                mega[RCIDX(row, col, config->ncolumns)];
        }
    }
    char *row_flip_blocks = z3_node_blocks(config, row_flip);
    char *col_flip_blocks = z3_node_blocks(config, col_flip);
    char *rcb_flip_blocks = z3_node_blocks(config, rcb_flip);
    for (uint8_t row = 0; row < config->nrows * config->nrows; ++row) {
        uint8_t R = row / config->nrows;
        uint8_t r = row % config->nrows;
        for (uint8_t col = 0; col < config->ncolumns * config->ncolumns; ++col) {
            uint8_t C = col / config->ncolumns;
            uint8_t c = col % config->ncolumns;
            row_flip_blocks[RC2IDX(config->nrows - (1 + R), C,
                                   config->nrows - (1 + r), c,
                                   config->nrows, config->ncolumns)] =
                blocks[RC2IDX(R, C, r, c, config->nrows, config->ncolumns)];
            col_flip_blocks[RC2IDX(R, config->ncolumns - (1 + C),
                                   r, config->ncolumns - (1 + c),
                                   config->nrows, config->ncolumns)] =
                blocks[RC2IDX(R, C, r, c, config->nrows, config->ncolumns)];
            rcb_flip_blocks[RC2IDX(config->nrows - (1 + R), config->ncolumns - (1 + C),
                                   config->nrows - (1 + r), config->ncolumns - (1 + c),
                                   config->nrows, config->ncolumns)] =
                blocks[RC2IDX(R, C, r, c, config->nrows, config->ncolumns)];
        }
    }
    *ntwins = 3;
    node_t *twins = malloc(*ntwins * sizeof(node_t));
    twins[0] = row_flip;
    twins[1] = col_flip;
    twins[2] = rcb_flip;
    return twins;
}


//****************************//
//  PUBLIC LIBRARY FUNCTIONS  //
//____________________________//


// 26dec2016 fixed
z3_t *z3_init(bool player1_ai, bool player2_ai, uint8_t block_init) {
    return z3_init_w(DFLT_NROWS, DFLT_NCOLUMNS, DFLT_NCONNECT, block_init,
                     DFLT_TILE_P1, DFLT_TILE_P2, DFLT_EMPTY_TILE,
                     player1_ai, player2_ai, DFLT_DEPTH);
}

// 26dec2016 fixed
z3_t *z3_init_w(uint8_t nrows, uint8_t ncolumns, uint8_t nconnect, uint8_t block_init,
                char tile_p1, char tile_p2, char tile_na,
                bool player1_ai, bool player2_ai, uint8_t depth) {
    z3_config_t config_raw = {.nrows = nrows, .ncolumns = ncolumns, .nconnect = nconnect,
                              .tile_p1 = tile_p1, .tile_p2 = tile_p2, .tile_na = tile_na,
                              .previous = nrows * ncolumns};
    z3_config_t *config = malloc(sizeof(z3_config_t));
    memcpy(config, &config_raw, sizeof(z3_config_t));
    z3_t *game = game_init(
                     z3_root(config, block_init),
                     z3_node_width(config),
                     z3_heuristic_win(config),
                     depth,
                     player1_ai,
                     player2_ai,
                     &z3_leaf,
                     &z3_spawn,
                     &z3_winner,
                     &z3_heuristic,
                     &z3_publish,
                     &z3_clone,
                     NULL //&z3_stratify
                 );
    game->config = config;
    return game;
}

//fixed 2017-03-03
void z3_reset(z3_t *game, uint8_t block_init) {
    z3_config_t *config = (z3_config_t *)game->config;
    game_reset(game);
    config->previous = config->nrows * config->ncolumns;
}

//fixed 2017-03-03
void z3_free(z3_t *game) {
    game_free(game);
}

//TODO:needs game API fix
void z3_set_block_init(z3_t *game, uint8_t block_init) {
  return;
}






