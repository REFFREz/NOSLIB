#ifndef NOSLIB_H
#define NOSLIB_H

#include <stdint.h>
#include <stddef.h>

typedef uint64_t u64;
typedef int64_t i64;

typedef enum {
    SMOOTH_STEP,
    SMOOTHER_STEP,
    COSINE_INTERPOLATION,
    BILINEAR_INTERPOLATION
} NoiseInterpolationVersions;

typedef struct {
    // make sure non of these 2 folowing variables except octaves are 0
    u64 seed;
    size_t chunkSize;
    size_t octaves;

    float freq;
    float rangeMin;
    float rangeMax;

    NoiseInterpolationVersions interVersion; // defaulted to SMOOTH_STEP
} NoiseContext;

float nl_getNoiseAt(NoiseContext ctx, size_t x, size_t y);

// chunkX and Y should be the topleft corners of a chunk
float* nl_getChunkAt(NoiseContext ctx, size_t chunkX, size_t chunkY); 
void nl_freeChunk(float* chunk);

#endif