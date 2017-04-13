

#ifndef GAME_H
#define GAME_H


//-------------------//
//  HEADER INCLUDES  //
//-------------------//


//#include "negamax.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>


//--------------------//
//  TYPE DEFINITIONS  //
//--------------------//

// 'negamax_t' typedef
typedef struct _negamax_t negamax_t;

// client-specified node reference typedef
typedef void *node_t;

// player enum typedef
typedef enum _player_t {
    P_ONE = 1,
    P_OAKLEY = 1,
    P_TWO = -1,
    P_TAYLOR = -1,
    P_DRAW = 0,
    P_DAKOTA = 0
} player_t;

// (private) data struct typedef definition
//struct _data_t;
typedef struct _data_t data_t;

// generic two-player zero-sum game struct typedef (defined below)
//struct _game_t;
typedef struct _game_t game_t;

// client-specified game state function reference typedefs
typedef bool (*leaf_t)(game_t const *, node_t const);
typedef node_t *(*spawn_t)(game_t const *, node_t const, size_t * const);
typedef player_t (*winner_t)(game_t const *, node_t const);
typedef int (*heuristic_t)(game_t const *, node_t const);
typedef void (*publish_t)(game_t const *, node_t const);
typedef node_t *(*clone_t)(game_t const *, node_t const, size_t * const);
typedef void (*stratify_t)(game_t const *, node_t * const, size_t const);

// game struct definition
struct _game_t {
    // mandatory client-supplied public functions
    leaf_t leaf;
    spawn_t spawn;
    winner_t winner;
    heuristic_t heuristic;
    publish_t publish;
    // optional client-supplied public functions
    clone_t clone;
    stratify_t stratify;
    // private game data reference
    data_t *data;
    // reference to custom client game data
    void *config;
};


//---------------------//
//  LIBRARY FUNCTIONS  //
//---------------------//


// constructor
game_t *game_init(
    node_t const root,
    size_t const width,
    int const heuristic_max,
    uint8_t const depth,
    bool player1_ai,
    bool player2_ai,
    leaf_t const leaf,
    spawn_t const spawn,
    winner_t const winner,
    heuristic_t const heuristic,
    publish_t const publish,
    clone_t const clone,
    stratify_t const stratify
);

// destructor
void game_free(game_t *game);


//-----------//
//  getters  //
//-----------//


node_t game_root(game_t const *game);
size_t game_width(game_t const *game);
int game_heuristic_max(game_t const *game);
negamax_t *game_negamax(game_t const *game);
uint8_t game_depth(game_t const *game);
bool game_player1_ai(game_t const *game);
bool game_player2_ai(game_t const *game);
player_t game_player(game_t const *game);
node_t game_state(game_t const *game);
int game_eval(game_t const *game);
node_t game_move_index(game_t const *game, size_t index);
unsigned game_score(game_t const *game, player_t player);
//void game_moves_print(game_t const *game);
size_t game_moves_size(game_t const *game);

//-----------//
//  setters  //
//-----------//


// reset game to initial state (except score)
void game_reset(game_t *game);

// reset game variables
void game_reset_score(game_t *game);

// reset game variables with new root node
void game_reset_root(game_t *game, node_t const root);

// reset all game variables with new root node
void game_reset_all(game_t *game, node_t const root);

void game_toggle_ai(game_t const *game, bool toggle_p1, bool toggle_p2);
void game_score_add(game_t *game, player_t player);
void game_publish_state(game_t *game);


//-------------//
//  game play  //
//-------------//

// designate & verify played move
bool game_move(game_t *game, node_t move);

// advance game by one move (ai or human)
bool game_advance(game_t *game);

// advance game by one move (ai vs. ai)
void game_advance_ai2(game_t *game1, game_t *game2);

// rewind game by 'nrewind' moves
void game_rewind(game_t *game, size_t nrewind);

// prompts user for new match
bool game_prompt_rematch();

// overall game play loop
void game_play(game_t *game);

// overall game play loop (ai vs. ai)
void game_play_ai2(game_t *game1, game_t *game2);

#endif // GAME_H

