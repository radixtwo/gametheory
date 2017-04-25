
#include "testWrite.h"
#include <stdio.h>

void TestWrite(char const *directoryPath) {
    FILE *fp = fopen(directoryPath, "w");
    fprintf(fp, "Hello Dusan! This is Neil.\n");
    fclose(fp);
}


