
#include "game.h"
#include "vector.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MOVES_CAPACITY      16
#define MOVES_MULTIPLIER    2
#define MOVES_INCREMENT     1

// private data struct definition
struct _data_t {
    node_t const root;
    size_t const width;
    int const heuristic_win;
    player_t player;
    node_t state;
    vector_t *moves;
    unsigned score[2];
};

// static functions

static node_t node_dup(node_t const node, size_t width) {
    node_t copy = malloc(width);
    memcpy(copy, node, width);
    return copy;
}

/*
static void moves_trash(void *addr) {
    free(*(node_t *)addr);
}
*/

static void player_toggle(game_t *game) {
    game->data->player *= -1;
}

// public functions

game_t *game_init(node_t const root, size_t const width, int const heuristic_win,
                  leaf_t const leaf, spawn_t const spawn, heuristic_t const heuristic, publish_t const publish,
                  clone_t const clone, stratify_t const stratify) {
    data_t data_raw = {.root = root, .width = width, .heuristic_win = heuristic_win};
    data_t *data = malloc(sizeof(data_t));
    memcpy(data, &data_raw, sizeof(data_t));
    data->player = OAKLEY;
    data->state = node_dup(root, width);
    //data->moves = vector_init_w(sizeof(node_t), MOVES_CAPACITY, MOVES_MULTIPLIER, MOVES_INCREMENT, &moves_trash);
    //vector_append(data->moves, &data->state);
    data->score[0] = 0;
    data->score[1] = 0;
    game_t *game = malloc(sizeof(game_t));
    game->leaf = leaf;
    game->spawn = spawn;
    game->heuristic = heuristic;
    game->publish = publish;
    game->clone = clone;
    game->stratify = stratify;
    game->data = data;
    game->config = NULL;
    return game;
}

void game_reset(game_t *game) {
    data_t *data = game->data;
    //vector_free(data->moves);
    free(data->state);
    data->player = OAKLEY;
    data->state = node_dup(data->root, data->width);
    //data->moves = vector_init_w(sizeof(node_t), MOVES_CAPACITY, MOVES_MULTIPLIER, MOVES_INCREMENT, &moves_trash);
    //vector_append(data->moves, &data->state);
    data->score[0] = 0;
    data->score[1] = 0;
}

void game_free(game_t *game) {
    //vector_free(game->data->moves);
    free(game);
}

node_t game_root(game_t const *game) {
    return game->data->root;
}

size_t game_width(game_t const *game) {
    return game->data->width;
}

int game_heuristic_win(game_t const *game) {
    return game->data->heuristic_win;
}

player_t game_player(game_t const *game) {
    return game->data->player;
}

node_t game_state(game_t const *game) {
    return game->data->state;
}

unsigned game_score(game_t const *game, player_t player) {
    return game->data->score[player == ONE ? 0 : 1];
}

void game_move(game_t *game, node_t node) {
    //printf("game_move: start\n");
    size_t noptions;
    node_t *options = game->spawn(game, game_state(game), &noptions);
    //printf("game_move: after spawn\n");
    bool legal = false;
    for (size_t i = 0; i < noptions; ++i) {
        if (!legal && !memcmp(node, options[i], game->data->width))
            legal = true;
        free(options[i]);
    }
    free(options);
    //printf("game_move: before `if legal`\n");
    if (legal) {
        player_toggle(game);
        node_t copy = node_dup(node, game->data->width);
        game->data->state = copy;
        //vector_append(game->data->moves, &copy);
    }
    //printf("game_move: end\n");
}

/*
void game_rewind(game_t *game, size_t nrewind) {
    size_t nmoves = vector_size(game->data->moves);
    if (nmoves > nrewind) {
        if (nrewind % 2 == 1)
            player_toggle(game);
        while (nrewind--)
            vector_remove(game->data->moves, --nmoves);
        game->data->state = vector_index(game->data->moves, nmoves - 1);
    }
}
*/

void game_score_add(game_t *game, player_t player) {
    game->data->score[player == ONE ? 0 : 1]++;
}

//fixed 2017-03-03) still needs fixing...
/*
void game_play(game_t *game) {
    while (!game->leaf(game_state(game)), / *...* /)
      //advance
    
}

void game_rematch(game_t *game) {
    
}
*/



