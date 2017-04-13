
#include "iOS_ZZZHumanGame.h"
#include <stdio.h>
#include <stdbool.h>

int main() {
    SetupHumanGame(4, 3, 3, 1, 1);
    int playerNumber = 1;
    while (true) {
        PrintHumanGame();
        printf("\n");
        int tileNumber;
        printf("enter tile number:\n");
        scanf(" %d", &tileNumber);
        int *results = NULL;
        while (!(results = HumanMove(tileNumber, playerNumber))) {
            printf("enter tile number:\n");
            scanf(" %d", &tileNumber);
        }
        if (results[0])
            break;
        playerNumber = playerNumber == 1 ? 2 : 1;
    }
    PrintHumanGame();
    EndHumanGame();
    return 0;
}

