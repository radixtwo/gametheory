
#ifndef IOS_ZZZ_AI_GAME_H
#define IOS_ZZZ_AI_GAME_H

//#include <stddef.h>

void SetupAIGame(int m, int n, int k, int initBoard, int staleMode, int humanPlayerNum, int difficulty, char const *dataDirPath);
void EndAIGame();
int *AIHumanMove(int tileNumber, int playerNumber);
int *AIMove(int playerNumber);
//void ReverseAIMove();
//void PrintAIGame();

#endif // IOS_ZZZ_AI_GAME_H



