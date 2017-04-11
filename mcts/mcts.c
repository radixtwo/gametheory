
#include "mcts.h"
#include "game.h"
//#include "hashmap.h"
#include "nkrand.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


typedef struct _state_t {
    vector_t *parents;
    unsigned wins;
    unsigned losses;
    unsigned rolls;
} state_t;

struct _mcts_t {
    game_t const *game;
    hashmap_t *states;
};

static void state_trash(void *addr) {
    state_t *node = addr;
    vector_free(node->parents);
}

static node_t node_dup(node_t node, size_t width) {
    node_t copy = malloc(width);
    memcpy(copy, node, width);
    return copy;
}

static void node_free_except(node_t except, node_t *options, size_t noptions) {
    for (size_t n = 0; n < noptions; ++n)
        if (options[n] && options[n] != except)
            free(options[n]);
    free(options);
}

/*
static void mcts_load_offspring(mcts_t *mcts, tree_t *root) {
    game_t const *game = mcts->game;
    node_t node = root->node;
    node_t *offspring = game->spawn(game, node, &root->noffspring);
    tree_t **root_offspring = malloc(root->noffspring * sizeof(tree_t *));
    for (size_t n = 0; n < noffspring; ++n) {
        tree_t *root_child = malloc(sizeof(tree_t));
        *root_child = {offspring[n], root, NULL, 0, 0, 0};
        root_offspring[n] = root_child;
    }
    root->offspring = root_offspring;
}
*/

static state_t *state_create(mcts_t *mcts, node_t node, node_t parent) {
    game_t *game = mcts->game;
    vector_t *parents = vector_init(game_width(game), 2, 2, 1, NULL);
    vector_append(parents, parent);
    state_t state = {parents, 0, 0, 0}
    hashmap_set(mcts->states, node, &state);
    return hashmap_get(mcts->state, node);
}

static node_t node_unrolled(mcts_t *mcts, node_t node) {
    game_t *game = mcts->game;
    size_t noffspring;
    node_t *offspring = game->spawn(game, node);
    node_t unrolled = NULL;
    for (size_t n = 0; n < noffspring; ++n)
        if (!hashmap_get(mcts->states, offspring[n]))
            unrolled = offspring[n];
    node_free_except(unrolled, offspring, noffspring);
    return unrolled;
}















static void mcts_backpropagate(mcts_t *mcts, state_t *leaf, int result) {
    state_t *state = leaf;
    while (state) {
        state->wins += win > 0 ? 1 : 0;
        state->losses += win < 0 ? 1 : 0;
        ++state->rolls;
        state = hashmap_get(mcts->nodes, state->parent);
    }
}



static bool mcts_roll(mcts_t *mcts, tree_t *root, player_t player) {
    game_t const *game = mcts->game;
    node_t node = root->node;
    node_t state = node_dup(node, game_width(game));
    while (!game->leaf(state)) {
        size_t noffspring;
        node_t *offspring = game->spawn(game, state, &noffspring);
        free(state);
        state = offspring[nkrand(noffspring)];
        node_free_except(state, offspring, noffspring);
    }
    int value = player * game->heuristic(game, state);
    free(state);
    return value > 0;
}

static state_t *mcts_select(mcts_t *mcts, node_t node) {
    game_t *game = mcts->game;
    state_t *state = hashmap_get(mcts->nodes, node);
    while (state->rolls) {
        if (!state->noffspring)
            mcts_load_offspring(mcts, state);
        for (size_t n = 0; n < state->noffspring; ++n)
            if (!state->noffspring[n]->rolls)
                state = state->noffspring[n];
    }
    return state;
}

static void mcts_eval(mcts_t *mcts, node_t node, player_t player, uint8_t rolls) {
    
    tree_t selection = mcts_select(mcts, player);
    bool win = mcts_roll(mcts, selection);
    mcts_backpropagate(mcts, selection, win);
}

static int mcts_eval_search(mcts_t *mcts, node_t node, player_t player, uint8_t nrolls) {
    game_t const *game = mcts->game;
    node_t *children = game->spawn(node);
    
    if (
}



mcts_t *mcts_init(game_t const *game) {
    mcts_t *mcts = malloc(sizeof(mcts_t));
    mcts->game = game;
    hashmap_t *states = hashmap_init(game_width(game), sizeof(state_t), 16, 8, 8, 2, ((uint64_t)rand() << 32) | rand(), &state_trash);
    return mcts;
}






































