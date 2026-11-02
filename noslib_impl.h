// this is just the normal library but without additional .c file
// you might not want to use this one if you dont want extra local functions
// and variables in your space but it still exists.

#ifndef NOSLIB_IMPL
#define NOSLIB_IMPL

#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <stdint.h>
#include <stddef.h>

typedef uint64_t u64;
typedef int64_t i64;

#define PI ((double)3.14159265359)

u64 fallbackValue = 1337;

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
void nl_freeChunk(float* chunk);

uint64_t lhash(u64 x) {
    x ^= x >> 30;
    x *= UINT64_C(0xbf58476d1ce4e5b9);
    x ^= x >> 27;
    x *= UINT64_C(0x94d049bb133111eb);
    x ^= x >> 31;

    return x;
}

u64 hash3(u64 a, u64 b, u64 c) {
    u64 h = lhash(a);
    h = lhash(h ^ lhash(b));
    h = lhash(h ^ lhash(c));

    return h;
}

float frand(u64 seed, float min, float max) {
    uint64_t x = seed;

    x ^= x >> 30;
    x *= UINT64_C(0xbf58476d1ce4e5b9);
    x ^= x >> 27;
    x *= UINT64_C(0x94d049bb133111eb);
    x ^= x >> 31;

    // convert upper 24 bits to [0, 1)
    float t = (float)(x >> 40) / 16777216.0f;

    return min + t * (max - min);
}

float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

float smoothstep(float t) {
    return t * t * (3.0f - 2.0f * t);
}

float smootherstep(float t) {
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

float cosineInterpolation(float t) {
    return (1.0f - cosf(t * PI)) * 0.5f;
}

// this is the old version that uses int coordinates instead of floats
float getNoiseAtOld(NoiseContext ctx, size_t x, size_t y) {
    u64 csize = ctx.chunkSize;
    
    // getting the coordinate of topleft corner of the chunk we are in
    u64 boundX = x - (x % csize);
    u64 boundY = y - (y % csize);

    // getting the noise values at 4 chunk corners
    // order goes:
    //    1 - 2
    //    3 - 4

    float chunkValue1 = frand(hash3(ctx.seed, boundX, boundY), ctx.rangeMin, ctx.rangeMax);
    float chunkValue2 = frand(hash3(ctx.seed, boundX + csize, boundY), ctx.rangeMin, ctx.rangeMax);
    float chunkValue3 = frand(hash3(ctx.seed, boundX, boundY + csize), ctx.rangeMin, ctx.rangeMax);
    float chunkValue4 = frand(hash3(ctx.seed, boundX + csize, boundY + csize), ctx.rangeMin, ctx.rangeMax);

    // using 2D lerp to get the final value
    float tx = (float)(x - boundX) / (float)csize;
    float ty = (float)(y - boundY) / (float)csize;

    float top = lerp(chunkValue1, chunkValue2, tx);
    float bottom = lerp(chunkValue3, chunkValue4, tx);

    float f = lerp(top, bottom, ty);

    return f;
}

float getNoiseAt(NoiseContext ctx, float x, float y) {
    float csize = (float)ctx.chunkSize;

    // Find surrounding lattice corners
    float boundX = floorf(x / csize) * csize;
    float boundY = floorf(y / csize) * csize;

    u64 bx = (u64)boundX;
    u64 by = (u64)boundY;

    float v1 = frand(hash3(ctx.seed, bx, by), ctx.rangeMin, ctx.rangeMax);
    float v2 = frand(hash3(ctx.seed, bx + ctx.chunkSize, by), ctx.rangeMin, ctx.rangeMax);
    float v3 = frand(hash3(ctx.seed, bx, by + ctx.chunkSize), ctx.rangeMin, ctx.rangeMax);
    float v4 = frand(hash3(ctx.seed, bx + ctx.chunkSize, by + ctx.chunkSize), ctx.rangeMin, ctx.rangeMax);

    float tx, ty, top, bottom;

    switch (ctx.interVersion) {
        case SMOOTH_STEP: {
            tx = smoothstep((x - boundX) / csize);
            ty = smoothstep((y - boundY) / csize);

            break;
        }

        case BILINEAR_INTERPOLATION: {
            tx = (float)(x - boundX) / (float)csize;
            ty = (float)(y - boundY) / (float)csize;

            break;
        }

        case SMOOTHER_STEP: {
            tx = smootherstep((x - boundX) / csize);
            ty = smootherstep((y - boundY) / csize);

            break;
        }

        case COSINE_INTERPOLATION: {
            tx = cosineInterpolation((x - boundX) / csize);
            ty = cosineInterpolation((y - boundY) / csize);

            break;
        }

        default: {
            tx = smoothstep((x - boundX) / csize);
            ty = smoothstep((y - boundY) / csize);

            break;
        }
    }

    top = lerp(v1, v2, tx);
    bottom = lerp(v3, v4, tx);

    return lerp(top, bottom, ty);
}

// implementation of public functions

float nl_getNoiseAt(NoiseContext ctx, size_t x, size_t y) {
    if (ctx.octaves < 1) {return getNoiseAt(ctx, x, y);}

    float amplitude = 1.0f;
    float frequency = ctx.freq;

    float total = 0.0f;
    float maxAmplitude = 0.0f;

    for (size_t i = 0; i < ctx.octaves; i++) {
        NoiseContext octave = ctx;

        octave.seed = lhash(ctx.seed + i);

        total += getNoiseAt(octave, (float)x * frequency, (float)y * frequency) * amplitude;

        maxAmplitude += amplitude;

        amplitude *= 0.5f;
        frequency *= 2.0f;
    }

    return total / maxAmplitude;
}

// chunkX and Y should be the topleft corners of a chunk
float* nl_getChunkAt(NoiseContext ctx, size_t chunkX, size_t chunkY) {
    u64 csize = ctx.chunkSize;
    
    float* out = malloc(csize * csize * sizeof(float));
    if (out == NULL) return NULL;

    for (size_t y = 0; y < csize; y++) {
        for (size_t x = 0; x < csize; x++) {
            out[y * csize + x] = nl_getNoiseAt(ctx, x + chunkX, y + chunkY);
        }
    }

    return out;
}

void nl_freeChunk(float* chunk) {
    if (chunk != NULL) free(chunk);
}

#endif