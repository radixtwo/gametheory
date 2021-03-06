
#include "iOS_ZZZAIGame.h"
#include "zigzagzoe.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define FILENAME_LENGTH_MAX 63

static z3_t *aiGame;
static char *dataFilePath;

static char *genFilename(int m, int n, int k, int staleMode, int difficulty) {
    char nameBuf[1 + FILENAME_LENGTH_MAX];
    sprintf(nameBuf, "%d-%d-%d-%d-%d.dat", m, n, k, staleMode, difficulty);
    char *filename = strdup(nameBuf);
    return filename;
}

static void initFilePath(int m, int n, int k, int staleMode, int difficulty, char const *dataDirPath) {
    if (dataDirPath) {
        char *filename = genFilename(m, n, k, staleMode, difficulty);
        char *path = malloc(1 + strlen(dataDirPath) + strlen(filename));
        strcpy(path, dataDirPath);
        strcpy(path + strlen(dataDirPath), filename);
        free(filename);
        dataFilePath = path;
    } else
        dataFilePath = NULL;
}

void SetupAIGame(int m, int n, int k, int initBoard, int staleMode, int humanPlayerNum, int difficulty, char const *dataDirPath) {
    srand((unsigned)time(NULL));
    initFilePath(m, n, k, staleMode, difficulty, dataDirPath);
    aiGame = z3_iOS_SetupGame_AI(m, n, k, initBoard, staleMode, humanPlayerNum, difficulty, dataFilePath);
}

void EndAIGame() {
    z3_iOS_EndGame_AI(aiGame, dataFilePath);
    if (dataFilePath)
        free(dataFilePath);
}

int *AIHumanMove(int tileNumber, int playerNumber) {
    size_t nResults;
    return z3_iOS_Move_Human(aiGame, tileNumber, playerNumber, &nResults);
}

int *AIMove(int playerNumber) {
    size_t nResults;
    return z3_iOS_Move_AI(aiGame, playerNumber, &nResults);
}



