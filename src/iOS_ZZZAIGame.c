
#include "iOS_ZZZAIGame.h"
#include "zigzagzoe.h"

static z3_t *aiGame;

void SetupAIGame(int m, int n, int k, int initBoard, int staleMode, int playerNumber, int depth, const char *dataPath) {
    //aiGame = z3_iOS_SetupGame_AI(m, n, k, initBoard, staleMode);
}

void EndAIGame() {
    z3_free(aiGame);
}

int *AIMove(int tileNumber, int playerNumber) {
    //size_t nResults;
    //return z3_iOS_Move_AI(aiGame, tileNumber, playerNumber, &nResults);
}

void ReverseAIMove(int nRewind) {
    //game_rewind(aiGame, nRewind);
}


