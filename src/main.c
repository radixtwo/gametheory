
#include <stdio.h>
#include "iOS_ZZZHumanGame.h"

int main() {
    SetupHumanGame(3, 3, 3, 1, 1);
    PrintHumanGame();
    int *result1 = HumanMove(10, 1);
    PrintHumanGame();
    //int *result2 = HumanMove(3, 1);
    //int *result2 = HumanMove(27, 1);
    //PrintHumanGame();
    EndHumanGame();
    return 0;
}

