
#include "iOS_ZZZHumanGame.h"
#include "zigzagzoe.h"

static z3_t *humanGame;

void SetupHumanGame(int m, int n, int k, int initBoard, int staleMode) {
    humanGame = z3_iOS_SetupGame_Human(m, n, k, initBoard, staleMode);
}

void EndHumanGame() {
    z3_free(humanGame);
}

int *HumanMove(int tileNumber, int playerNumber, size_t *nResults) {
    return z3_iOS_Move_Human(humanGame, tileNumber, playerNumber, nResults);
}

void ReverseHumanMove(int nRewind) {
    game_rewind(humanGame, nRewind);
}

void PrintHumanGame() {
    humanGame->publish(humanGame, game_state(humanGame));
}

