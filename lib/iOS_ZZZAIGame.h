
#ifndef IOS_ZZZ_AI_GAME_H
#define IOS_ZZZ_AI_GAME_H

#include <stddef.h>

void SetupAIGame(int m, int n, int k, int initBoard, int staleMode, int playerNumber, int depth, const char *dataPath);
void EndAIGame();
int *AIMove(int tileNumber, int playerNumber);
void ReverseAIMove();
//void PrintAIGame();

#endif // IOS_ZZZ_AI_GAME_H



