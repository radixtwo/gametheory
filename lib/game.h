
#ifndef GAME_H
#define GAME_H

#include <stddef.h>
#include <stdbool.h>

// client-specified node reference typedef
typedef void *node_t;

// player enum typedef
typedef enum _player_t {
    ONE = 1,
    OAKLEY = 1,
    TWO = -1,
    TAYLOR = -1
} player_t;

// private data struct typedef
struct _data_t;
typedef struct _data_t data_t;

// generic two-player zero-sum game struct typedef
struct _game_t;
typedef struct _game_t game_t;

// client-specified game state function reference typedefs
typedef bool (*leaf_t)(game_t const *, node_t const);
typedef node_t *(*spawn_t)(game_t const *, node_t const, size_t * const);
typedef int (*heuristic_t)(game_t const *, node_t const);
typedef void (*publish_t)(game_t const *, node_t const);
typedef node_t *(*clone_t)(game_t const *, node_t const, size_t * const);
typedef void (*stratify_t)(game_t const *, node_t * const, size_t const);

// game struct definition
struct _game_t {
    // mandatory public functions
    leaf_t leaf;
    spawn_t spawn;
    heuristic_t heuristic;
    publish_t publish;
    // optional public functions
    clone_t clone;
    stratify_t stratify;
    // private data reference
    data_t *data;
    // client-specific configuration reference
    void *config;
};

// (2016dec28) TODO:add negamax to game_t struct underneath (data_t *)data
// constructor
game_t *game_init(
    node_t const root,
    size_t const width,
    int const heuristic_win,
    leaf_t const leaf,
    spawn_t const spawn,
    heuristic_t const heuristic,
    publish_t const publish,
    clone_t const clone,
    stratify_t const stratify
);

// destructor
void game_reset(game_t *game);
void game_free(game_t *game);

// getters
node_t game_root(game_t const *game);
size_t game_width(game_t const *game);
int game_heuristic_win(game_t const *game);

player_t game_player(game_t const *game);
node_t game_state(game_t const *game);
unsigned game_score(game_t const *game, player_t player);

// game play
void game_move(game_t *game, node_t move);
//void game_rewind(game_t *game, size_t nrewind);
void game_score_add(game_t *game, player_t player);

// game loop
//void game_play(game_t *game);
//void game_rematch(game_t *game);

#endif // GAME_H

