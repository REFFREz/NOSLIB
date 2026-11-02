// creating a random 2D chunk and printing it.

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "noslib_impl.h"

static char hValues[5] = {' ', '.', '+', '#', '@'};

int main(int argc, char** argv) {
    size_t chunkSize = 30;

    if (argc > 1) {
        chunkSize = atoi(argv[1]);
        if (chunkSize == 0) chunkSize = 30;
    }

    NoiseContext ctx = {
        .chunkSize = chunkSize,
        .freq = 2.7f,
        .octaves = 1,
        .interVersion = SMOOTH_STEP,
        .rangeMax = 5.0f,
        .rangeMin = 0.0f,
        .seed = time(NULL)
    };

    float* chunkData = nl_getChunkAt(ctx, 0, 0);

    for (size_t y = 0; y < chunkSize; y++) {
        for (size_t x = 0; x < chunkSize; x++) {
            size_t offset = y * chunkSize + x;
            int value = chunkData[offset];

            // print 2 since in most terminals ratio of chars width and height is 2:1
            printf("%c%c", hValues[value], hValues[value]);
        }
        printf("\n");
    }

    nl_freeChunk(chunkData);
    return 0;
}