#include "noslib.h"

#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define PI ((double)3.14159265359)

// local functions needed for noise generation

u64 fallbackValue = 1337;

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

u64 hash2(u64 a, u64 b) {
    return (u64)(lhash(lhash(a) ^ (lhash(b))));
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
// it is a little bit faster too.
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

        case LINEAR_INTERPOLATION: {
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

float nl_clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
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

float* nl_getBatch(NoiseContext ctx, size_t x1, size_t y1, size_t x2, size_t y2) {
    size_t minX = MIN(x1, x2);
    size_t minY = MIN(y1, y2);
    size_t maxX = MAX(x1, x2);
    size_t maxY = MAX(y1, y2);

    size_t lx = maxX - minX;
    size_t ly = maxY - minY;

    if (lx == 0 || ly == 0) return NULL;

    float *out = malloc(lx * ly * sizeof(float));
    if (out == NULL) return NULL;

    for (size_t y = 0; y < ly; y++) {
        for (size_t x = 0; x < lx; x++) {
            out[y * lx + x] = nl_getNoiseAt(ctx, minX + x, minY + y);
        }
    }

    return out;
}

void nl_getBatchInto(NoiseContext ctx, size_t x1, size_t y1, size_t x2, size_t y2, float* dest) {
    size_t minX = MIN(x1, x2);
    size_t minY = MIN(y1, y2);
    size_t maxX = MAX(x1, x2);
    size_t maxY = MAX(y1, y2);

    size_t lx = maxX - minX;
    size_t ly = maxY - minY;

    if (lx == 0 || ly == 0) return;

    for (size_t y = 0; y < ly; y++) {
        for (size_t x = 0; x < lx; x++) {
            dest[y * lx + x] = nl_getNoiseAt(ctx, minX + x, minY + y);
        }
    }
}

void nl_free(float* chunk) {
    if (chunk != NULL) free(chunk);
}

// ************ //
// 1D functions //
// ************ //

// stripped and faster version of the algorithm behind this library
float getNoiseAt1D(NoiseContext ctx, float x) {
    size_t sz = ctx.chunkSize;

    float sectionStart = floorf(x / sz) * sz;
    float sectionEnd = sectionStart + sz;

    float v1 = frand(hash2(ctx.seed, sectionStart), ctx.rangeMin, ctx.rangeMax);
    float v2 = frand(hash2(ctx.seed, sectionEnd), ctx.rangeMin, ctx.rangeMax);

    float t = (x - sectionStart) / sz;

    switch (ctx.interVersion) {
        case SMOOTH_STEP:
            t = smoothstep(t);
            break;
        case SMOOTHER_STEP:
            t = smootherstep(t);
            break;
        case COSINE_INTERPOLATION:
            t = cosineInterpolation(t);
            break;
        case LINEAR_INTERPOLATION:
            break;
        default:
            t = smoothstep(t);
            break;
    }

    return lerp(v1, v2, t);
}

float nl_getNoiseAt1D(NoiseContext ctx, float x) {
    if (ctx.octaves < 1) {return getNoiseAt1D(ctx, x);}

    float amplitude = 1.0f;
    float frequency = ctx.freq;

    float total = 0.0f;
    float maxAmplitude = 0.0f;

    for (size_t i = 0; i < ctx.octaves; i++) {
        NoiseContext octave = ctx;

        octave.seed = lhash(ctx.seed + i);

        total += getNoiseAt1D(octave, x * frequency) * amplitude;

        maxAmplitude += amplitude;

        amplitude *= 0.5f;
        frequency *= 2.0f;
    }

    return total / maxAmplitude;
}

float* nl_getSectionAt1D(NoiseContext ctx, float x1, float x2) {
    size_t minX = (size_t)MIN(x1, x2);
    size_t maxX = (size_t)MAX(x1, x2);

    if (maxX - minX == 0) return NULL;

    float* out = malloc((maxX - minX) * sizeof(float));
    if (out == NULL) return NULL;

    for (size_t i = minX; i < maxX; i++) {
        out[i - minX] = nl_getNoiseAt1D(ctx, (float)i);
    }

    return out;
}

float* nl_getChunkAt1D(NoiseContext ctx, float x) {
    size_t sz = ctx.chunkSize;

    if (sz == 0 || x < 0.0f) return NULL;

    size_t sectionStart = ((size_t)x / sz) * sz;
    size_t sectionEnd = sectionStart + sz;

    float* out = malloc(sz * sizeof(float));
    if (out == NULL) return NULL;

    for (size_t i = sectionStart; i < sectionEnd; i++) {
        out[i - sectionStart] = nl_getNoiseAt1D(ctx, (float)i);
    }

    return out;
}