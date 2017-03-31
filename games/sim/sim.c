

#include "sim.h"
#include "game.h"
#include "ansicolor.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define XORSWP(x,y) do { x ^= y; y ^= x; x ^= y; } while (0)

#define DFLT_NVERTICES 6
#define DFLT_DEPTH 7

// references by 'config' in 'game.h'
typedef uint8_t sim_nvertex_t;

// length equals (N choose 2) for graph with N vertices
// represents adjacency matrix of graph
// possible values are 0, 1, or -1
typedef int8_t *sim_graph_t;

static inline uint8_t max(uint8_t x, uint8_t y) {
    return x > y ? x : y;
}

static inline uint8_t min(uint8_t x, uint8_t y) {
    return x < y ? x : y;
}

static inline uint8_t choose2(uint8_t n) {
    return n * (n - 1) / 2;
}

static inline uint8_t edge_index(uint8_t row, uint8_t column) {
    uint8_t m = max(row, column);
    uint8_t n = min(row, column);
    return choose2(m) + n;
}

static inline size_t sim_graph_width(sim_nvertex_t *nvx) {
    return choose2(*nvx);
}

static inline uint8_t sim_graph_nedges_max(sim_nvertex_t *nvx) {
    return choose2(*nvx);
}

static inline int8_t sim_graph_edge_value(sim_nvertex_t *nvx, sim_graph_t node, uint8_t row, uint8_t column) {
    //add verification
    return node[edge_index(row, column)];
}

static inline uint8_t sim_graph_nedges(sim_nvertex_t *nvx, sim_graph_t node) {
    uint8_t nedges = 0;
    for (size_t edge = 0; edge < sim_graph_nedges_max(nvx); ++edge)
        if (node[edge])
            ++nedges;
    return nedges;
}

static inline uint8_t sim_graph_nempty(sim_nvertex_t *nvx, sim_graph_t node) {
    uint8_t nempty = 0;
    for (size_t edge = 0; edge < sim_graph_nedges_max(nvx); ++edge)
        if (!node[edge])
            ++nempty;
    return nempty;
}

static inline bool sim_graph_player1(sim_nvertex_t *nvx, sim_graph_t node) {
    return !(sim_graph_nedges(nvx, node) % 2);
}

static inline sim_graph_t sim_graph_root(sim_nvertex_t *nvx) {
    return calloc(sim_graph_width(nvx), sizeof(int8_t));
}



// helper functions //

static int8_t triangle_verify(sim_nvertex_t *nvx, sim_graph_t node, uint8_t *vx) {
    int8_t e01 = sim_graph_edge_value(nvx, node, vx[0], vx[1]);
    int8_t e02 = sim_graph_edge_value(nvx, node, vx[0], vx[2]);
    int8_t e12 = sim_graph_edge_value(nvx, node, vx[1], vx[2]);
    if (e01 && e02 && e12 && e01 == e02 && e01 == e12 && e02 == e12)
        return e01;
    return 0;
}

static int8_t triangle_search(sim_nvertex_t *nvx, sim_graph_t node, uint8_t *vx, uint8_t const left, uint8_t const right) {
    if (left == right) {
        int8_t loser = triangle_verify(nvx, node, vx);
        if (loser)
            return loser;
    }
    for (uint8_t n = left; n <= right; ++n) {
        if (left != n)
            XORSWP(vx[left], vx[n]);
        int8_t loser = triangle_search(nvx, node, vx, left + 1, right);
        if (loser)
            return loser;
        if (left != n)
            XORSWP(vx[left], vx[n]);
    }
    return 0;
}

static player_t sim_graph_winner(sim_nvertex_t *nvx, sim_graph_t node) {
    uint8_t nedges_max = sim_graph_nedges_max(nvx);
    uint8_t *vx = malloc(nedges_max * sizeof(uint8_t));
    for (size_t edge = 0; edge < nedges_max; ++edge)
        vx[edge] = edge;
    int8_t loser = triangle_search(nvx, node, vx, 0, *nvx - 1);
    free(vx);
    return loser == -1 ? OAKLEY : loser ? TAYLOR : DAKOTA;
}


// game.h functions


static bool sim_leaf(game_t const *game, node_t const node) {
    sim_nvertex_t *nvx = game->config;
    uint8_t nedges_max = sim_graph_nedges_max(nvx);
    if (sim_graph_winner(nvx, node) != DAKOTA)
        return true;
    for (size_t edge = 0; edge < nedges_max; ++edge)
        if (((sim_graph_t)node)[edge] == 0)
            return false;
    return true;
}

static node_t *sim_spawn(game_t const *game, node_t const node, size_t * const noffspring) {
    sim_graph_t graph = node;
    size_t width = sim_graph_width(game->config);
    uint8_t nedges_max = sim_graph_nedges_max(game->config);
    int8_t color = sim_graph_player1(game->config, node) ? 1 : -1;
    node_t offspring_buf[nedges_max];
    *noffspring = 0;
    for (size_t edge = 0; edge < nedges_max; ++edge) {
        if (!graph[edge]) {
            sim_graph_t child = malloc(width);
            memcpy(child, graph, width);
            child[edge] = color;
            offspring_buf[(*noffspring)++] = child;
        }
    }
    node_t *offspring = malloc(*noffspring * sizeof(node_t));
    memcpy(offspring, offspring_buf, *noffspring * sizeof(node_t));
    return offspring;
}

static player_t sim_winner(game_t const *game, node_t const node) {
    return sim_graph_winner(game->config, (sim_graph_t)node);
}

static int sim_heuristic(game_t const *game, node_t const node) {
    player_t winner = sim_graph_winner(game->config, (sim_graph_t)node);
    return winner == OAKLEY ? 1 : winner ? -1 : 0;
}

static void sim_publish(game_t const *game, node_t const node) {
    sim_nvertex_t *nvx = game->config;
    sim_graph_t graph = node;
    uint8_t nedges_max = sim_graph_nedges_max(nvx);
    uint8_t count = 0, goal = 1;
    for (size_t edge = 0; edge < nedges_max; ++edge) {
        ++count;
        if (!graph[edge])
            printf("- ");
        else if (graph[edge] == 1)
            printf("x ");
        else
            printf("o ");
        if (count == goal) {
            printf("\n");
            count = 0;
            ++goal;
        }
    }
}


// public functions



sim_t *sim_init(bool player1_ai, bool player2_ai) {
    return sim_init_w(DFLT_NVERTICES, player1_ai, player2_ai, DFLT_DEPTH);
}

sim_t *sim_init_w(uint8_t nvertices, bool player1_ai, bool player2_ai, uint8_t depth) {
    sim_nvertex_t *nvx = malloc(sizeof(uint8_t));
    *nvx = nvertices;
    sim_t *game = game_init(
                      sim_graph_root(nvx),
                      sim_graph_width(nvx),
                      1,
                      depth,
                      player1_ai,
                      player2_ai,
                      &sim_leaf,
                      &sim_spawn,
                      &sim_winner,
                      &sim_heuristic,
                      &sim_publish,
                      NULL, //clone
                      NULL //stratify
                  );
    game->config = nvx;
    return game;
}

void sim_reset(sim_t *game) {
    game_reset(game);
}

void sim_free(sim_t *game) {
    game_free(game);
}






















































