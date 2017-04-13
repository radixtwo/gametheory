
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
        size_t nResults;
        int *results = NULL;
        while (!(results = HumanMove(tileNumber, playerNumber, &nResults))) {
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

