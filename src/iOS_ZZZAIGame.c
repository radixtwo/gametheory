
#include "iOS_ZZZAIGame.h"
#include "zigzagzoe.h"

static z3_t *aiGame;
static char const *dataDirPath;

void SetupAIGame(int m, int n, int k, int initBoard, int staleMode, int humanPlayerNum, int difficulty, char const *dataDirectoryPath) {
    dataDirPath = dataDirectoryPath;
    aiGame = z3_iOS_SetupGame_AI(m, n, k, initBoard, staleMode, humanPlayerNum, difficulty);
}

void EndAIGame() {
    // save game data
    z3_free(aiGame);
}

int *AIHumanMove(int tileNumber, int playerNumber) {
    size_t nResults;
    return z3_iOS_Move_Human(aiGame, tileNumber, playerNumber, &nResults);
}

int *AIMove(int playerNumber) {
    size_t nResults;
    return z3_iOS_Move_AI(aiGame, playerNumber, &nResults);
}



