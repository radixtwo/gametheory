
#ifndef IOS_ZZZ_HUMAN_GAME_H
#define IOS_ZZZ_HUMAN_GAME_H

#include <stddef.h>

void SetupHumanGame(int m, int n, int k, int initBoard, int staleMode);
void EndHumanGame();
int *HumanMove(int tileNumber, int playerNumber, size_t *nResults);
void ReverseHumanMove();
void PrintHumanGame();

#endif // IOS_ZZZ_HUMAN_GAME_H



