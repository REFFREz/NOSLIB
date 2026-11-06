// creating a random 1D chunk and printing it.

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "noslib.h"

static char hValues[5] = {' ', '.', '+', '#', '@'};

int main(int argc, char** argv) {
    size_t chunkSize = 50;

    if (argc > 1) {
        chunkSize = atoi(argv[1]);
        if (chunkSize == 0) chunkSize = 30;
    }

    NoiseContext ctx = {
        .chunkSize = chunkSize,
        .freq = 4.0f,
        .octaves = 1,
        .interVersion = SMOOTH_STEP,
        .rangeMax = 5.0f,
        .rangeMin = 0.0f,
        .seed = time(NULL)
    };

    float* chunkData = nl_getChunkAt1D(ctx, 0);

    for (size_t r = 0; r < 5; r++) {
        for (size_t i = 0; i < chunkSize; i++) {
            size_t value = chunkData[i];
            printf("%c%c", hValues[value], hValues[value]);
        }
        printf("\n");
    }

    nl_free(chunkData);
    return 0;
}