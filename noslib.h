#ifndef NOSLIB_H
#define NOSLIB_H

#include <stdint.h>
#include <stddef.h>

typedef uint64_t u64;
typedef int64_t i64;

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

typedef enum {
    SMOOTH_STEP,
    SMOOTHER_STEP,
    COSINE_INTERPOLATION,
    LINEAR_INTERPOLATION
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

#define NOSLIB_DEFAULT_CONTEXT ((NoiseContext) { \
    .seed = 1337, \
    .chunkSize = 10, \
    .octaves = 1, \
    .freq = 1.0f, \
    .rangeMax = 100.0f, \
    .rangeMin = 0.0f \
})

float nl_getNoiseAt(NoiseContext ctx, size_t x, size_t y);

// chunkX and Y should be the topleft corners of a chunk
float* nl_getChunkAt(NoiseContext ctx, size_t chunkX, size_t chunkY); 
// uses for loops to iterate through the whole batch, may cost some performance.
float* nl_getBatch(NoiseContext ctx, size_t x1, size_t y1, size_t x2, size_t y2);
void nl_getBatchInto(NoiseContext ctx, size_t x1, size_t y1, size_t x2, size_t y2, float* dest);

// ************ //
// 1D functions //
// ************ //

float nl_getNoiseAt1D(NoiseContext ctx, float x);
float* nl_getSectionAt1D(NoiseContext ctx, float x1, float x2);
float* nl_getChunkAt1D(NoiseContext ctx, float x);

float nl_clamp(float value, float min, float max);
void nl_free(float* chunk);

#endif