
//-------------------//
//  HEADER INCLUDES  //
//-------------------//


#include "game.h"
#include "negamax.h"
#include "vector.h"
#include "ansicolor.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


//----------//
//  MACROS  //
//----------//


#define MOVES_CAPACITY      16
#define MOVES_MULTIPLIER    2
#define MOVES_INCREMENT     1
#define DEFAULT_DEPTH       8


//------------------------//
//  VARIABLE DEFINITIONS  //
//------------------------//


typedef struct _data_t {
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

    vector_t *moves;
    unsigned score[2];
} data_t;


//--------------------//
//  STATIC FUNCTIONS  //
//--------------------//


//--------------------//
//  inline functions  //
//--------------------//


// dynamically allocates duplicate of passed node
static inline node_t node_dup(node_t const node, size_t width) {
    node_t copy = malloc(width);
    memcpy(copy, node, width);
    return copy;
}

// toggles current player
static inline void player_toggle(game_t *game) {
    game->data->player *= -1;
}

static inline bool player_human(game_t *game) {
    player_t player = game_player(game);
    return (player == P_OAKLEY && !game_player1_ai(game)) || (player == P_TAYLOR && !game_player2_ai(game));
}

// clears screen & prints game state to standard output
static inline void publish_state(game_t *game) {
    printf(ANSI.erase);
    game->publish(game, game_state(game));
    printf("\n");
}


//----------------------------//
//  'moves' vector functions  //
//----------------------------//


// frees each element of 'game->data->moves' vector
static void moves_trash(void *node_ref) {
    free(*(node_t *)node_ref);
}

// static variable for 'moves_printv'
//static game_t *game_moves;

// 'moves' vector print function
//static void moves_printv(void const *node_ref) {
    //game_moves->publish(game_moves, *(node_t *)node_ref);
//}


//-----------------------//
//  game play functions  //
//-----------------------//


// prompts human player for choice & returns selected move
static node_t human_move(game_t *game) {
    size_t noffspring;
    node_t *offspring = game->spawn(game, game_state(game), &noffspring);
    printf("enter choice (");
    for (size_t n = 0; n < noffspring; ++n) {
        printf("%s%s%c%s", ANSI.bold, ANSI.yellow, 'a' + (char)n, ANSI.reset);
        if (n < noffspring - 1)
            printf(", ");
    }
    printf("):\n");
    int index;
    char choice;
    scanf(" %c", &choice);
    index = choice - (int)'a';
    while (index < 0 || index >= (int)noffspring) {
        printf("invalid choice!\nenter choice:\n");
        scanf(" %c", &choice);
        index = choice - (int)'a';
    }
    node_t move = offspring[index];
    for (size_t option = 0; option < noffspring; ++option)
        if (option != (size_t)index) {
            free(offspring[option]);
        }
    free(offspring);
    return move;
}


//---------------------//
//  LIBRARY FUNCTIONS  //
//---------------------//


//--------------------------------//
//  initialization & destruction  //
//--------------------------------//


// initializes game
game_t *game_init(node_t const root, size_t const width, int const heuristic_max,
                  uint8_t const depth, bool player1_ai, bool player2_ai,
                  leaf_t const leaf, spawn_t const spawn, winner_t const winner,
                  heuristic_t const heuristic, publish_t const publish,
                  clone_t const clone, stratify_t const stratify) {
    data_t data_raw = {.root = node_dup(root, width), .width = width, .heuristic_max = heuristic_max,
                       .depth = depth, .player1_ai = player1_ai, .player2_ai = player2_ai};
    data_t *data = malloc(sizeof(data_t));
    memcpy(data, &data_raw, sizeof(data_t));

    data->player = P_OAKLEY;
    data->state = node_dup(root, width);
    data->eval = 0;

    data->moves = vector_init_w(sizeof(node_t), MOVES_CAPACITY, MOVES_MULTIPLIER, MOVES_INCREMENT, &moves_trash);
    vector_append(data->moves, &data->state);
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
        data->depth = !depth ? DEFAULT_DEPTH : depth;
        data->negamax = negamax_init(game);
    }

    return game;
}

// frees 'game_t' struct & other necessary data
void game_free(game_t *game) {
    data_t *data = game->data;
    free(data->root);
    //free(data->state);
    vector_free(data->moves);
    negamax_free(data->negamax);
    free(game);
}


//-----------//
//  getters  //
//-----------//


// NOTE: DO NOT MODIFY RETURN VALUE!!!!
node_t game_root(game_t const *game) {
    return game->data->root;
}

size_t game_width(game_t const *game) {
    return game->data->width;
}

int game_heuristic_max(game_t const *game) {
    return game->data->heuristic_max;
}

// NOTE: DO NOT MODIFY RETURN VALUE!!!!
negamax_t *game_negamax(game_t const *game) {
    return game->data->negamax;
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

// NOTE: DO NOT MODIFY RETURN VALUE!!!!
node_t game_state(game_t const *game) {
    return game->data->state;
}

int game_eval(game_t const *game) {
    return game->data->eval;
}

// NOTE: DO NOT MODIFY RETURN VALUE!!!!
node_t game_move_index(game_t const *game, size_t index) {
    return *(node_t *)vector_index(game->data->moves, index);
}

unsigned game_score(game_t const *game, player_t player) {
    return game->data->score[player == P_ONE ? 0 : 1];
}

/*
void game_moves_print(game_t const *game) {
    vector_print(game->data->moves, &moves_printv);
}
*/

size_t game_moves_size(game_t const *game) {
    return vector_size(game->data->moves);
}

//-----------//
//  setters  //
//-----------//


// resets 'game_t' data struct for new match
void game_reset(game_t *game) {
    data_t *data = game->data;
    //free(data->state);
    //printf("clearing moves\n");
    vector_clear(data->moves);
    data->player = P_OAKLEY;
    data->state = node_dup(data->root, data->width);
    data->eval = 0;
    vector_append(data->moves, &data->state);
}

// resets 'game_t' data struct values for rematch
void game_reset_score(game_t *game) {
    data_t *data = game->data;
    //free(data->state);
    vector_clear(data->moves);
    data->player = P_OAKLEY;
    data->state = node_dup(data->root, data->width);
    data->eval = 0;
    vector_append(data->moves, &data->state);
    data->score[0] = 0;
    data->score[1] = 0;
}


// resets 'game_t' data struct values with new root node
void game_reset_root(game_t *game, node_t const root) {
    data_t *data = game->data;
    //free(data->state);
    vector_clear(data->moves);
    data->player = P_OAKLEY;
    free(data->root);
    data->root = node_dup(root, data->width);
    data->state = node_dup(data->root, data->width);
    data->eval = 0;
    vector_append(data->moves, &data->state);
}

// resets 'game_t' data struct values with new root node
void game_reset_all(game_t *game, node_t const root) {
    data_t *data = game->data;
    //free(data->state);
    vector_clear(data->moves);
    data->player = P_OAKLEY;
    free(data->root);
    data->root = node_dup(root, data->width);
    data->state = node_dup(data->root, data->width);
    data->eval = 0;
    vector_append(data->moves, &data->state);
    data->score[0] = 0;
    data->score[1] = 0;
}

void game_toggle_ai(game_t const *game, bool toggle_p1, bool toggle_p2) {
    data_t *data = game->data;
    data->player1_ai = toggle_p1 ? !data->player1_ai : data->player1_ai;
    data->player2_ai = toggle_p2 ? !data->player2_ai : data->player2_ai;
    if (!data->negamax && (data->player1_ai || data->player2_ai))
        data->negamax = negamax_init(game);
}

void game_score_add(game_t *game, player_t player) {
    game->data->score[player == P_ONE ? 0 : 1]++;
}

void game_publish_state(game_t *game) {
    publish_state(game);
}


//-------------//
//  game play  //
//-------------//


void game_move(game_t *game, node_t node) {
    size_t noptions;
    node_t *options = game->spawn(game, game_state(game), &noptions);
    bool legal = false;
    for (size_t i = 0; i < noptions; ++i) {
        if (!legal && !memcmp(node, options[i], game->data->width))
            legal = true;
        free(options[i]);
    }
    free(options);
    if (legal) {
        player_toggle(game);
        node_t copy = node_dup(node, game->data->width);
        game->data->state = copy;
        vector_append(game->data->moves, &copy);
    }
}

void game_advance(game_t *game) {
    data_t *data = game->data;
    player_t player = game_player(game);
    node_t move = NULL;
    if ((player == P_OAKLEY && data->player1_ai) || (player == P_TAYLOR && data->player2_ai))
        move = negamax_move(data->negamax, data->state, player, data->depth, &data->eval);
    else
        move = human_move(game);
    game_move(game, move);
    free(move);
}

void game_advance_ai2(game_t *game1, game_t *game2) {
    data_t *data1 = game1->data;
    data_t *data2 = game2->data;
    player_t player = data1->player;
    node_t move = NULL;
    if ((player == P_OAKLEY && data1->player1_ai) || (player == P_TAYLOR && data1->player2_ai))
        move = negamax_move(data1->negamax, data1->state, player, data1->depth, &data1->eval);
    else
        move = negamax_move(data2->negamax, data1->state, player, data2->depth, &data2->eval);
    game_move(game1, move);
    free(move);
}

void game_rewind(game_t *game, size_t nrewind) {
    size_t nmoves = vector_size(game->data->moves);
    if (nmoves > nrewind) {
        if (nrewind % 2 == 1)
            player_toggle(game);
        while (nrewind--)
            vector_remove(game->data->moves, --nmoves);
        game->data->state = *(node_t *)vector_index(game->data->moves, nmoves - 1);
    }
}

bool game_prompt_rematch() {
    printf("would you like to play again (y/n)?\n");
    char yn;
    scanf(" %c", &yn);
    while (yn != 'y' && yn != 'n') {
        printf("must enter 'y' or 'n'!\n");
        printf("would you like to play again (y/n)?\n");
        scanf(" %c", &yn);
    }
    return yn == 'y';
/*
    if (yn == 'y') {
        printf("toggle player 1 (y/n)? ");
        scanf(" %c", &yn);
        if (yn == 'y')
            game_toggle_ai(game, true, false);
        printf("toggle player 2 (y/n)? ");
        scanf(" %c", &yn);
        if (yn == 'y')
            game_toggle_ai(game, false, true);
        return true;
    } else
        return false;
*/
}

void game_play(game_t *game) {
    data_t *data = game->data;
    do {
        while (!game->leaf(game, game_state(game))) {
            publish_state(game);
            if (player_human(game)) {
                size_t nmoves = vector_size(game->data->moves);
                char reverse;
                if (data->player1_ai != data->player2_ai && nmoves > 2) {
                    printf("reverse last move (y/n)?\n");
                    scanf(" %c", &reverse);
                    if (reverse == 'y') {
                        game_rewind(game, 2);
                        continue;
                    } 
                } else if (nmoves > 1) {
                    printf("reverse last move (y/n)?\n");
                    scanf(" %c", &reverse);
                    if (reverse == 'y') {
                        game_rewind(game, 1);
                        continue;
                    } 
                }
            }
            game_advance(game);
            //printf("eval = %d; heuristic = %d\n", -1 * game_player(game) * game->data->eval, game->heuristic(game, game_state(game)));
        }
        publish_state(game);
        player_t winner = game->winner(game, game_state(game));
        printf("%s\n", winner == P_OAKLEY ? "player 1 wins!" : winner == P_TAYLOR ? "player 2 wins!" : "it's a draw!");
        game_score_add(game, winner);
        printf("score: %u - %u\n", game_score(game, P_OAKLEY), game_score(game, P_TAYLOR));
        game_reset(game);
    } while (game_prompt_rematch());;
}


void game_play_ai2(game_t *game1, game_t *game2) {
    do {
        while (!game1->leaf(game1, game_state(game1))) {
            //game1->publish(game1, game_state(game1));
            game_advance_ai2(game1, game2);
            //printf("eval = %d; heuristic = %d\n", -1 * game_player(game) * game->data->eval, game->heuristic(game, game_state(game)));
        }
        printf("\n");
        //game1->publish(game1, game_state(game1));
        player_t winner = game1->winner(game1, game_state(game1));
        printf("%s\n", winner == P_OAKLEY ? "player 1 wins!" : winner == P_TAYLOR ? "player 2 wins!" : "it's a draw!");
        game_score_add(game1, winner);
        printf("score: %u - %u\n", game_score(game1, P_OAKLEY), game_score(game1, P_TAYLOR));
        game_reset(game1);
    } while (game_prompt_rematch());;
}


