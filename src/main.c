
#include <stdio.h>
#include "iOS_ZZZHumanGame.h"

int main() {
    SetupHumanGame(3, 3, 3, 1, 1);
    PrintHumanGame();
    size_t nResults;
    int *result1 = HumanMove(10, 1, &nResults);
    PrintHumanGame();
    //int *result2 = HumanMove(3, 1);
    //int *result2 = HumanMove(27, 1);
    //PrintHumanGame();
    EndHumanGame();
    return 0;
}

