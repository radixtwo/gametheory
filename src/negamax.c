
#include "negamax.h"
#include "game.h"
#include "hashmap.h"
#include "nkrand.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define DEBUG_MODE          0
#define BRAIN_NBUCKETS      49
#define BRAIN_MULTIPLIER    28
#define BRAIN_INCREMENT     5
#define BRAIN_LOADFACTOR    2

/*--------------------*/
/*  type definitions  */
/*--------------------*/

typedef enum {
    LOWER,
    EXACT,
    UPPER
} bound_t;

typedef struct {
    bound_t bound;
    unsigned depth;
    int value;
} memo_t;

struct _negamax_t {
    game_t const *game;
    hashmap_t *ttable;
};

// static functions

// attempts to find any node whose value is equivalent to node in transposition table
static memo_t *recall(negamax_t const *negamax, node_t node) {
    game_t const *game = negamax->game;
    memo_t *memo = hashmap_get(negamax->ttable, node);
    if (memo || !game->clone)
        return memo;
    size_t nclones;
    node_t *clones = game->clone(game, node, &nclones);
    for (size_t c = 0; c < nclones; ++c) {
        if (!memo)
            memo = hashmap_get(negamax->ttable, clones[c]);
        free(clones[c]);
    }
    free(clones);
    return memo;
}

static void memoize(negamax_t *negamax, node_t node, bound_t bound, unsigned depth, int value) {
    memo_t memo = {bound, depth, value};
    hashmap_set(negamax->ttable, node, &memo);
}

static void node_free_except(node_t except, node_t *options, size_t noptions) {
    for (size_t n = 0; n < noptions; ++n)
        if (options[n] && options[n] != except) {
            free(options[n]);
        }
    free(options);
}

// evaluates a node
static int negamax_search(negamax_t *negamax, node_t node, player_t player, unsigned depth, int alpha, int beta) {
    int alpha0 = alpha;
    game_t const *game = negamax->game;
    memo_t *memo = recall(negamax, node);
    if (memo && memo->depth >= depth) {
        switch (memo->bound) {
            case EXACT:
                return memo->value;
            case LOWER:
                alpha = (memo->value > alpha) ? memo->value : alpha;
                break;
            case UPPER:
                beta = (memo->value < beta) ? memo->value : beta;
                break;
            default:
                break;
        }
        if (alpha >= beta)
            return memo->value;
    }
    if (depth == 0 || game->leaf(game, node))
        return player * game->heuristic(game, node);
    size_t noptions;
    node_t *options = game->spawn(game, node, &noptions);
    fyshuffle(options, noptions, sizeof(node_t));
    if (game->stratify)
        game->stratify(game, options, noptions);
    int value_best = -1 * game_heuristic_max(game);
    for (size_t i = 0; i < noptions; ++i) {
        int value = -1 * negamax_search(negamax, options[i], -1 * player, depth - 1, -1 * beta, -1 * alpha);
        value_best = value > value_best ? value : value_best;
        alpha = value > alpha ? value : alpha;
        if (alpha >= beta)
            break;
    }
    node_free_except(NULL, options, noptions);
    bound_t bound = EXACT;
    if (value_best <= alpha0)
        bound = UPPER;
    else if (value_best >= beta)
        bound = LOWER;
    memoize(negamax, node, bound, depth, value_best);
    return value_best;
}

/*---------------------*/
/*  library functions  */
/*---------------------*/

negamax_t *negamax_init(game_t const *game) {
    negamax_t *negamax = malloc(sizeof(negamax_t));
    negamax->game = game;
    //negamax->ttable = hashmap_init_w(game_width(game), sizeof(memo_t), 16, 8, 2, 2, ((uint64_t)rand() << 32) | rand(), NULL);
    negamax->ttable = hashmap_init_w(game_width(game), sizeof(memo_t), 16, 8, 2, 2, getRandomUInt64(), NULL);
    return negamax;
}

void negamax_free(negamax_t *negamax) {
    if (!negamax)
        return;
    hashmap_free(negamax->ttable);
    free(negamax);
}

negamax_t *negamax_load(game_t const *game, char const *filename) {
    negamax_t *negamax = malloc(sizeof(negamax_t));
    negamax->game = game;
    negamax->ttable = hashmap_load(filename);
    return negamax;
}

bool negamax_save(negamax_t const *negamax, char const *filename) {
    return hashmap_save(negamax->ttable, filename);
}

size_t negamax_ttable_size(negamax_t const *negamax) {
    return hashmap_size(negamax->ttable);
}

size_t negamax_nbytes(negamax_t const *negamax) {
    return hashmap_nbytes(negamax->ttable);
}

int negamax_eval(negamax_t *negamax, node_t node, player_t player, unsigned depth) {
    return negamax_search(negamax, node, player, depth, -1 * game_heuristic_max(negamax->game), game_heuristic_max(negamax->game));
}

node_t negamax_move(negamax_t *negamax, node_t node, player_t player, unsigned depth, int * const eval) {
    //printf("negamax_move\n");
    game_t const *game = negamax->game;
    int alpha = -1 * game_heuristic_max(game);
    int beta = game_heuristic_max(game);
    int value_best = alpha;
    size_t noptions, nbest = 1;
    node_t *options = game->spawn(game, node, &noptions);
    if (!eval && noptions == 1) {
        node_t move = options[0];
        free(options);
        return move;
    }
    node_t best[noptions];
    for (size_t i = 0; i < noptions; ++i) {
        int value = -1 * negamax_search(negamax, options[i], -1 * player, depth - 1, -1 * beta, -1 * alpha);
        if (value > value_best) {
            value_best = value;
            best[0] = options[i];
            nbest = 1;
        } else if (value == value_best)
            best[nbest++] = options[i];
    }
    if (eval)
        *eval = value_best;
    fyshuffle(best, nbest, sizeof(node_t));
    if (game->stratify)
        game->stratify(game, best, nbest);
    node_t move = best[0];
    node_free_except(move, options, noptions);
    //printf("negamax_move returning\n\n");
    return move;
}

