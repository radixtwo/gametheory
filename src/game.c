
#include "game.h"
#include "negamax.h"
#include "vector.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MOVES_CAPACITY      16
#define MOVES_MULTIPLIER    2
#define MOVES_INCREMENT     1
#define DEFAULT_DEPTH       8


// private data struct definition
struct _data_t {
    node_t root;
    size_t width;
    int heuristic_max;

    negamax_t *negamax;
    uint8_t depth;
    bool player1_ai;
    bool player2_ai;

    player_t player;
    node_t state;
    int eval;

    //vector_t *moves;
    unsigned score[2];
};

//////////////////////
// static functions //
//////////////////////

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

static node_t human_move(game_t *game) {
    size_t noffspring;
    node_t *offspring = game->spawn(game, game_state(game), &noffspring);
    printf("enter choice (");
    for (size_t n = 0; n < noffspring; ++n) {
        printf("%c", 'a' + (char)n);
        if (n < noffspring - 1)
            printf(",");
    }
    printf("): ");
    int index;
    char choice;
    scanf(" %c", &choice);
    index = choice - (int)'a';
    while (index < 0 || index >= noffspring) {
        printf("invalid choice!\nenter choice: ");
        scanf(" %c", &choice);
        index = choice - (int)'a';
    }
    for (size_t option = 0; option < noffspring; ++option)
        if (option != index)
            free(offspring[option]);
    node_t move = offspring[index];
    free(offspring);
    return move;
}

//////////////////////
// public functions //
//////////////////////

game_t *game_init(node_t const root, size_t const width, int const heuristic_max,
                  uint8_t const depth, bool player1_ai, bool player2_ai,
                  leaf_t const leaf, spawn_t const spawn, winner_t const winner,
                  heuristic_t const heuristic, publish_t const publish,
                  clone_t const clone, stratify_t const stratify) {
    data_t data_raw = {.root = root, .width = width, .heuristic_max = heuristic_max,
                       .depth = depth, .player1_ai = player1_ai, .player2_ai = player2_ai};
    data_t *data = malloc(sizeof(data_t));
    memcpy(data, &data_raw, sizeof(data_t));
    data->player = P_OAKLEY;
    data->state = node_dup(root, width);
    data->eval = 0;
    //data->moves = vector_init_w(sizeof(node_t), MOVES_CAPACITY, MOVES_MULTIPLIER, MOVES_INCREMENT, &moves_trash);
    //vector_append(data->moves, &data->state);
    data->score[0] = 0;
    data->score[1] = 0;
    game_t *game = malloc(sizeof(game_t));
    game->leaf = leaf;
    game->spawn = spawn;
    game->winner = winner;
    game->heuristic = heuristic;
    game->publish = publish;
    game->clone = clone;
    game->stratify = stratify;
    game->data = data;
    game->config = NULL;
    if (player1_ai || player2_ai) {
        data->depth = depth ? DEFAULT_DEPTH : depth;
        data->negamax = negamax_init(game);
    }
    return game;
}

void game_reset(game_t *game) {
    data_t *data = game->data;
    //vector_free(data->moves);
    free(data->state);
    data->player = P_OAKLEY;
    data->state = node_dup(data->root, data->width);
    data->eval = 0;
    //data->moves = vector_init_w(sizeof(node_t), MOVES_CAPACITY, MOVES_MULTIPLIER, MOVES_INCREMENT, &moves_trash);
    //vector_append(data->moves, &data->state);
    data->score[0] = 0;
    data->score[1] = 0;
}

void game_free(game_t *game) {
    //vector_free(game->data->moves);
    negamax_free(game->data->negamax);
    free(game);
}

node_t game_root(game_t const *game) {
    return game->data->root;
}

size_t game_width(game_t const *game) {
    return game->data->width;
}

int game_heuristic_max(game_t const *game) {
    return game->data->heuristic_max;
}

uint8_t game_depth(game_t const *game) {
    return game->data->depth;
}

bool game_player1_ai(game_t const *game) {
    return game->data->player1_ai;
}

bool game_player2_ai(game_t const *game) {
    return game->data->player2_ai;
}

player_t game_player(game_t const *game) {
    return game->data->player;
}

node_t game_state(game_t const *game) {
    return game->data->state;
}

unsigned game_score(game_t const *game, player_t player) {
    return game->data->score[player == P_ONE ? 0 : 1];
}

void game_toggle_ai(game_t const *game, bool toggle_p1, bool toggle_p2) {
    data_t *data = game->data;
    data->player1_ai = toggle_p1 ? !data->player1_ai : data->player1_ai;
    data->player2_ai = toggle_p2 ? !data->player2_ai : data->player2_ai;
    if (!data->negamax && (data->player1_ai || data->player2_ai))
        data->negamax = negamax_init(game);
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

void game_advance(game_t *game) {
    data_t *data = game->data;
    player_t player = game_player(game);
    node_t move = NULL;
    if (player == P_OAKLEY && data->player1_ai || player == P_TAYLOR && data->player2_ai)
        move = negamax_move(data->negamax, game_state(game), player, data->depth, &data->eval);
    else
        move = human_move(game);
    game_move(game, move);
    free(move);
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
    game->data->score[player == P_ONE ? 0 : 1]++;
}

void game_play(game_t *game) {
    game->publish(game, game_state(game));
    while (!game->leaf(game, game_state(game))) {
        game_advance(game);
        game->publish(game, game_state(game));
    }
    player_t winner = game->winner(game, game_state(game));
    printf("%s\n", winner == P_OAKLEY ? "player 1 wins!" : winner == P_TAYLOR ? "player 2 wins!" : "it's a draw!");
}

/*
void game_rematch(game_t *game) {
    
}
*/



