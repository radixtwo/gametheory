
#include "connectn.h"
#include "negamax.h"
#include "rainbow.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define RCIDX(r,c,nc) ((r)*(nc)+(c))

#define DEFAULT_NROWS 6
#define DEFAULT_NCOLUMNS 7
#define DEFAULT_NCONNECT 4
#define DEFAULT_PLAYER1_TILE 'x'
#define DEFAULT_PLAYER2_TILE 'o'
#define DEFAULT_EMPTY_TILE '-'
#define DEFAULT_PLAYER1_NEGAMAX true
#define DEFAULT_PLAYER2_NEGAMAX true
#define DEFAULT_DEPTH 12
#define DEFAULT_DEPTH2 13

/*--------------------*/
/*  type definitions  */
/*--------------------*/

typedef char *grid_t;

typedef struct _config_t {
    uint8_t nrows;
    uint8_t ncolumns;
    uint8_t nconnect;
    char tile_p1;
    char tile_p2;
    char tile_na;
} config_t;

/*--------------------*/
/*  global variables  */
/*--------------------*/

static bool srand_done;
static connectn_t *game;

/*--------------------*/
/*  static functions  */
/*--------------------*/

static player_t tile2player(char tile) {
    if (tile == game->player1_tile)
        return OAKLEY;
    if (tile == game->player2_tile)
        return TAYLOR;
    return PLAYER_NAP;
}

static char player2tile(player_t player) {
    if (player == OAKLEY)
        return game->player1_tile;
    if (player == TAYLOR)
        return game->player2_tile;
    return game->empty_tile;
}

static size_t grid_size() {
    return game->nrows * game->ncolumns;
}

static grid_t grid_root() {
    size_t root_size = grid_size();
    grid_t root = malloc(root_size * sizeof(char));
    for (size_t t = 0; t < root_size; ++t)
        root[t] = game->empty_tile;
    return root;
}

static bool grid_sterile(grid_t grid) {
    bool infertile = true;
    for (uint8_t col = 0; col < game->ncolumns; ++col) {
        if (grid[RCIDX(0, col, game->ncolumns)] == game->empty_tile) {
            infertile = false;
            break;
        }
    }
    return infertile;
}

static uint8_t grid_nempty(grid_t grid) {
    uint8_t nempty = 0;
    for (size_t t = 0; t < game->grid_size; ++t)
        if (grid[t] == game->empty_tile)
            ++nempty;
    return nempty;
}

static player_t grid_player(grid_t grid) {
    uint8_t plies = game->nrows * game->ncolumns - grid_nempty(grid);
    return !(plies % 2) ? OAKLEY : TAYLOR;
}

static uint8_t grid_potency(grid_t grid, player_t player) {
    uint8_t potency = 0, row, col, n;
    char impotent = (player == OAKLEY) ? game->player2_tile : game->player1_tile;
    bool potent;
    for (row = 0; row < game->nrows; ++row) {
        for (col = 0; col < game->ncolumns - game->nconnect + 1; ++col) {
            potent = true;
            for (n = 0; n < game->nconnect; ++n) {
                if (grid[RCIDX(row, col + n, game->ncolumns)] == impotent) {
                    potent = false;
                    break;
                }
            }
            if (potent)
                ++potency;
        }
    }
    for (row = 0; row < game->nrows - game->nconnect + 1; ++row) {
        for (col = 0; col < game->ncolumns; ++col) {
            potent = true;
            for (n = 0; n < game->nconnect; ++n) {
                if (grid[RCIDX(row + n, col, game->ncolumns)] == impotent) {
                    potent = false;
                    break;
                }
            }
            if (potent)
                ++potency;
        }
    }
    for (row = 0; row < game->nrows - game->nconnect + 1; ++row) {
        for (col = 0; col < game->ncolumns - game->nconnect + 1; ++col) {
            potent = true;
            for (n = 0; n < game->nconnect; ++n) {
                if (grid[RCIDX(row + n, col + n, game->ncolumns)] == impotent) {
                    potent = false;
                    break;
                }
            }
            if (potent)
                ++potency;
        }
    }
    for (row = game->nrows - 1; row >= game->nconnect - 1; --row) {
        for (col = 0; col < game->ncolumns - game->nconnect + 1; ++col) {
            potent = true;
            for (n = 0; n < game->nconnect; ++n) {
                if (grid[RCIDX(row - n, col + n, game->ncolumns)] == impotent) {
                    potent = false;
                    break;
                }
            }
            if (potent)
                ++potency;
        }
    }
    return potency;
}

// returns OAKLEY if p1 wins, TAYLOR if p2 wins, PLAYER_NAP if not winning
static player_t grid_winning(grid_t grid) {
    player_t winner = PLAYER_NAP;
    uint8_t row, col, n;
    char origin;
    bool winning;
    for (row = 0; row < game->nrows; ++row) {
        for (col = 0; col < game->ncolumns - game->nconnect + 1; ++col) {
            origin = grid[RCIDX(row, col, game->ncolumns)];
            if ((winner = tile2player(origin)) == PLAYER_NAP)
                continue;
            winning = true;
            for (n = 1; n < game->nconnect; ++n) {
                if (grid[RCIDX(row, col + n, game->ncolumns)] != origin) {
                    winning = false;
                    break;
                }
            }
            if (winning)
                return winner;
        }
    }
    for (row = 0; row < game->nrows - game->nconnect + 1; ++row) {
        for (col = 0; col < game->ncolumns; ++col) {
            origin = grid[RCIDX(row, col, game->ncolumns)];
            if ((winner = tile2player(origin)) == PLAYER_NAP)
                continue;
            winning = true;
            for (n = 1; n < game->nconnect; ++n) {
                if (grid[RCIDX(row + n, col, game->ncolumns)] != origin) {
                    winning = false;
                    break;
                }
            }
            if (winning)
                return winner;
        }
    }
    for (row = 0; row < game->nrows - game->nconnect + 1; ++row) {
        for (col = 0; col < game->ncolumns - game->nconnect + 1; ++col) {
            origin = grid[RCIDX(row, col, game->ncolumns)];
            if ((winner = tile2player(origin)) == PLAYER_NAP)
                continue;
            winning = true;
            for (n = 1; n < game->nconnect; ++n) {
                if (grid[RCIDX(row + n, col + n, game->ncolumns)] != origin) {
                    winning = false;
                    break;
                }
            }
            if (winning)
                return winner;
        }
    }
    for (row = game->nrows - 1; row >= game->nconnect - 1; --row) {
        for (col = 0; col < game->ncolumns - game->nconnect + 1; ++col) {
            origin = grid[RCIDX(row, col, game->ncolumns)];
            if ((winner = tile2player(origin)) == PLAYER_NAP)
                continue;
            winning = true;
            for (n = 1; n < game->nconnect; ++n) {
                if (grid[RCIDX(row - n, col + n, game->ncolumns)] != origin) {
                    winning = false;
                    break;
                }
            }
            if (winning)
                return winner;
        }
    }
    return 0;
}

static void grid_print(grid_t grid) {
    ANSI_CLEAR();
    ANSI_CURSOR(1, 1);
    bool previous_color = false;
    for (uint8_t row = 0; row < game->nrows; ++row) {
        for (uint8_t col = 0; col < game->ncolumns; ++col) {
            char tile = grid[RCIDX(row, col, game->ncolumns)];
            if (!previous_color && col == game->previous && tile != game->empty_tile) {
                printf(ANSI_COLOR_BOLD ANSI_COLOR_GREEN " %c " ANSI_COLOR_RESET, tile);
                previous_color = true;
            } else if (tile == game->player1_tile)
                printf(ANSI_COLOR_BOLD ANSI_COLOR_RED " %c " ANSI_COLOR_RESET, tile);
            else if (tile == game->player2_tile)
                printf(ANSI_COLOR_BOLD ANSI_COLOR_BLUE " %c " ANSI_COLOR_RESET, tile);
            else
                printf(" %c ", tile);
        }
        printf("\n");
    }
}

static uint8_t potency_max() {
    grid_t root = grid_root();
    int16_t potency = (int16_t)grid_potency(root, OAKLEY);
    free(root);
    return potency;
}

static int16_t heuristic_max() {
    return potency_max() + (int16_t)game->nrows * (int16_t)game->ncolumns;
}

static void connectn_srand() {
    if (srand_done)
        return;
    srand(time(NULL));
    srand_done = true;
}

static bool connectn_terminal(node_t node) {
    return grid_sterile(node) || grid_winning(node);
}

static int16_t connectn_heuristic(node_t node) {
    int16_t winner = (int16_t)grid_winning(node);
    if (winner)
        return winner * (potency_max() + grid_nempty(node));
    if (grid_sterile(node))
        return 0;
    return (int16_t)grid_potency(node, OAKLEY) - (int16_t)grid_potency(node, TAYLOR);
}

static node_t *connectn_congruent(node_t node, size_t *ntwins) {
    grid_t grid = (grid_t)node;
    grid_t *twins = NULL;
    *ntwins = 0;
    char twin_buf[game->grid_size];
    for (uint8_t row = 0; row < game->nrows; ++row)
        for (uint8_t col = 0; col < game->ncolumns; ++col)
            twin_buf[RCIDX(row, game->ncolumns - col - 1, game->ncolumns)] = grid[RCIDX(row, col, game->ncolumns)];
    if (memcmp(twin_buf, grid, game->grid_size)) {
        grid_t twin = malloc(game->grid_size);
        memcpy(twin, twin_buf, game->grid_size);
        (*ntwins)++;
        twins = malloc(*ntwins * sizeof(node_t));
        twins[0] = twin;
    }
    return (node_t *)twins;
}

static node_t *connectn_offspring(node_t node, size_t *noffspring) {
    grid_t grid = (grid_t)node;
    grid_t offspring_buf[game->ncolumns];
    *noffspring = 0;
    player_t player = grid_player(grid);
    for (uint8_t col = 0; col < game->ncolumns; ++col) {
        if (grid[RCIDX(0, col, game->ncolumns)] == game->empty_tile) {
            grid_t child = malloc(game->grid_size);
            memcpy(child, grid, game->grid_size);
            uint8_t row = game->nrows - 1;
            while (true) {
                if (grid[RCIDX(row, col, game->ncolumns)] == game->empty_tile) {
                    child[RCIDX(row, col, game->ncolumns)] = player2tile(player);
                    break;
                }
                if (!row)
                    break;
                --row;
            }
            offspring_buf[(*noffspring)++] = child;
        }
    }
    node_t *offspring = malloc(*noffspring * sizeof(node_t));
    memcpy(offspring, offspring_buf, *noffspring * sizeof(node_t));
    return offspring;
}

static int connectn_compare(const void *addr1, const void *addr2) {
    return (int)((const grid_value_t *)addr2)->value - (int)((const grid_value_t *)addr1)->value;
}

static void connectn_eugenics(node_t *offspring, size_t noffspring) {
    grid_value_t rank[noffspring];
    for (size_t opt = 0; opt < noffspring; ++opt) {
        rank[opt].grid = offspring[opt];
        int16_t value = connectn_heuristic(offspring[opt]);
        rank[opt].value = (value < 0) ? -1 * value : value;
    }
    qsort(rank, noffspring, sizeof(grid_value_t), &connectn_compare);
    for (size_t opt = 0; opt < noffspring; ++opt)
        offspring[opt] = rank[opt].grid;
}

static grid_t human_move() {
    size_t noffspring;
    grid_t *offspring = (grid_t *)connectn_offspring(game->grid, &noffspring);
    uint8_t options[noffspring];
    size_t noptions = 0;
    for (size_t opt = 0; opt < noffspring; ++opt) {
        grid_t child = offspring[opt];
        uint8_t column = 0;
        for (; column < game->grid_size; ++column)
            if (game->grid[column] != child[column])
                break;
        column %= game->ncolumns;
        options[noptions++] = column;
    }
    for (size_t col = 0; col < game->ncolumns; ++col) {
        bool should_print = false;
        for (size_t opt = 0; opt < noffspring; ++opt) {
            if (options[opt] == col) {
                should_print = true;
                break;
            }
        }
        if (should_print)
            printf(ANSI_COLOR_BOLD ANSI_COLOR_YELLOW " %u " ANSI_COLOR_RESET, 1u + (unsigned int)col);
        else
            printf("   ");
    }
    printf("\n");
    char choice;
    grid_t move = NULL;
    printf(
        " Enter a column number:\n"
        ANSI_COLOR_BOLD ANSI_COLOR_YELLOW
        "╔══════════════════════╗\n"
        "║                      ║\n"
        "╚══════════════════════╝\n"
        ANSI_COLOR_RESET
    );
    ANSI_CURSOR(4 + game->nrows, 2);
    scanf(" %c", &choice);
    if (choice == 'b') {
        for (size_t child = 0; child < noffspring; ++child)
            if (offspring[child] != move)
                free(offspring[child]);
        free(offspring);
        game->player *= -1;
        free(game->moves[--game->nmoves]);
        free(game->moves[--game->nmoves]);
        game->grid = game->moves[game->nmoves - 2];
        return game->moves[--game->nmoves];
    }
    while (!move) {
        for (uint8_t opt = 0; opt < noptions; ++opt) {
            if (options[opt] == choice - '1') {
                move = offspring[opt];
                break;
            }
        }
        if (!move) {
            ANSI_CURSOR(2 + game->nrows, 1);
            printf(
                " Enter a column number: "
                ANSI_COLOR_BOLD ANSI_COLOR_RED
                "Invalid choice!"
                ANSI_COLOR_RESET
                "\n"
                ANSI_COLOR_BOLD ANSI_COLOR_YELLOW
                "╔══════════════════════╗\n"
                "║                      ║\n"
                "╚══════════════════════╝\n"
                ANSI_COLOR_RESET
            );
            ANSI_CURSOR(4 + game->nrows, 2);
            scanf(" %c", &choice);
        }
    }
    for (size_t child = 0; child < noffspring; ++child)
        if (offspring[child] != move)
            free(offspring[child]);
    free(offspring);
    return move;
}

/*---------------------*/
/*  library functions  */
/*---------------------*/

connectn_t *init_connectn() {
    return init_connectn_w(DEFAULT_NROWS, DEFAULT_NCOLUMNS, DEFAULT_NCONNECT,
                           DEFAULT_PLAYER1_TILE, DEFAULT_PLAYER2_TILE, DEFAULT_EMPTY_TILE,
                           DEFAULT_PLAYER1_NEGAMAX, DEFAULT_PLAYER2_NEGAMAX, DEFAULT_DEPTH,
                           NULL);
}

connectn_t *load_connectn(const char *filename) {
    return init_connectn_w(DEFAULT_NROWS, DEFAULT_NCOLUMNS, DEFAULT_NCONNECT,
                           DEFAULT_PLAYER1_TILE, DEFAULT_PLAYER2_TILE, DEFAULT_EMPTY_TILE,
                           DEFAULT_PLAYER1_NEGAMAX, DEFAULT_PLAYER2_NEGAMAX, DEFAULT_DEPTH,
                           filename);
}

connectn_t *init_connectn_w(uint8_t nrows, uint8_t ncolumns, uint8_t nconnect,
                            char player1_tile, char player2_tile, char empty_tile,
                            bool player1_negamax, bool player2_negamax, uint8_t depth,
                            const char *filename) {
    connectn_srand();
    connectn_t *connectn = malloc(sizeof(connectn_t));
    game = connectn;
    game->nrows = nrows;
    game->ncolumns = ncolumns;
    game->nconnect = nconnect;
    game->player1_tile = player1_tile;
    game->player2_tile = player2_tile;
    game->empty_tile = empty_tile;
    game->player1_negamax = player1_negamax;
    game->player2_negamax = player2_negamax;
    game->grid_size = grid_size();
    game->grid = grid_root();
    game->moves = malloc((1 + nrows * ncolumns) * sizeof(grid_t));
    game->moves[0] = game->grid;
    game->nmoves = 1;
    game->player = OAKLEY;
    game->previous = ncolumns;
    if (filename)
        game->negamax = load_negamax(game->grid_size, heuristic_max(), ncolumns,
                                     &connectn_terminal, &connectn_heuristic, &connectn_congruent,
                                     &connectn_offspring, &connectn_eugenics,
                                     filename);
    else
        game->negamax = init_negamax(game->grid_size, heuristic_max(), ncolumns,
                                     &connectn_terminal, &connectn_heuristic, &connectn_congruent,
                                     &connectn_offspring, &connectn_eugenics);
    game->negamax2 = init_negamax(game->grid_size, heuristic_max(), ncolumns,
                                 &connectn_terminal, &connectn_heuristic, &connectn_congruent,
                                 &connectn_offspring, &connectn_eugenics);
    //negamax_eval(game->negamax, game->grid, OAKLEY, 20);
    game->depth = depth;
    game->depth2 = DEFAULT_DEPTH2;
    game->eval = 0;
    return connectn;
}

void free_connectn(connectn_t *connectn) {
    game = connectn;
    free_negamax(game->negamax);
    free_negamax(game->negamax2);
    for (size_t m = 0; m < game->nmoves; ++m)
        free(game->moves[m]);
    free(game->moves);
    free(game);
    game = NULL;
}

void save_connectn(const connectn_t *connectn, const char *filename) {
    save_negamax(connectn->negamax, filename);
}

void connectn_toggle_negamax(connectn_t *connectn, bool player1_toggle, bool player2_toggle) {
    game = connectn;
    game->player1_negamax = player1_toggle ? !game->player1_negamax : game->player1_negamax;
    game->player2_negamax = player2_toggle ? !game->player2_negamax : game->player2_negamax;
}

void connectn_reset(connectn_t *connectn) {
    game = connectn;
    for (size_t m = 1; m < game->nmoves; ++m)
        free(game->moves[m]);
    game->nmoves = 1;
    game->grid = game->moves[0];
    game->player = OAKLEY;
    game->previous = game->ncolumns;
    game->eval = 0;
}

void connectn_print(connectn_t *connectn) {
    game = connectn;
    grid_print(game->grid);
}

void connectn_advance(connectn_t *connectn) {
    game = connectn;
    connectn_print(game);
    grid_t move = NULL;
    if ((game->player == OAKLEY && game->player1_negamax) ||
        (game->player == TAYLOR && game->player2_negamax)) {
/*
        if (game->player == TAYLOR)
            move = negamax_move(game->negamax2, game->grid, game->player, game->depth2, &game->eval);
        else
            move = negamax_move(game->negamax, game->grid, game->player, game->depth, &game->eval);
*/
        move = negamax_move(game->negamax, game->grid, game->player, game->depth, &game->eval);
    } else {
        move = human_move();
    }
    uint8_t previous = 0;
    for (; previous < game->grid_size; ++previous)
        if (game->grid[previous] != move[previous])
            break;
    previous %= game->ncolumns;
    game->previous = previous;
    game->grid = game->moves[game->nmoves++] = move;
    game->player *= -1;
}

void connectn_play(connectn_t *connectn) {
    game = connectn;
    while (!connectn_terminal(game->grid))
        connectn_advance(game);
    connectn_print(game);
    player_t winner = grid_winning(game->grid);
    printf("%s!\n", (winner != PLAYER_NAP) ? ((winner == OAKLEY) ? "player 1 wins" : "player 2 wins") : "it's a draw" );
}

