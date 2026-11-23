#include "noslib.h"

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <math.h>

#define PI ((double)3.14159265359)

u64 fallbackValue = 1337;
u64 randState = 1337;

uint64_t lhash(u64 x) {
    if (x == 0) x = UINT64_C(0x94d049bb133111eb);

    x ^= x >> 30;
    x *= UINT64_C(0xbf58476d1ce4e5b9);
    x ^= x >> 27;
    x *= UINT64_C(0x94d049bb133111eb);
    x ^= x >> 31;

    return x;
}

static inline u64 hash3(u64 seed, u64 x, u64 y) {
    u64 h = seed ^ (x * 0x9e3779b185ebca87ULL) ^ (y * 0xc2b2ae3d27d4eb4fULL);
    
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccdULL;
    h ^= h >> 33;
    h *= 0xc4ceb9fe1a85ec53ULL;
    h ^= h >> 33;
    
    return h;
}

static inline u64 hash2(u64 a, u64 b) {
    u64 h = 0x9e3779b185ebca87ULL ^ (a * 0xc2b2ae3d27d4eb4fULL) ^ (b * 0x165667b19e3779f9ULL);
    
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccdULL;
    h ^= h >> 33;
    h *= 0xc4ceb9fe1a85ec53ULL;
    h ^= h >> 33;
    
    return h;
}

float frand(u64 seed, float min, float max) {
    uint64_t x = seed;
    if (x == 0) x = 0x9404bf1331b;

    x ^= x >> 30;
    x *= UINT64_C(0xbf58476d1ce4e5b9);
    x ^= x >> 27;
    x *= UINT64_C(0x94d049bb133111eb);
    x ^= x >> 31;

    // convert upper 24 bits to [0, 1)
    float t = (float)(x >> 40) / 16777216.0f;

    return min + t * (max - min);
}

// expects already hashed value does not mix it by itself
static inline float frand_instant(u64 hash, float min, float max) {
    float t = (float)(hash >> 40) * 0.000000059604645f; // 1.0f / 16777216.0f
    
    return min + t * (max - min);
}

u64 urand(u64 seed, u64 min, u64 max) {
    u64 x = seed;
    if (x == 0) x = 0x9404bf1331b;

    x ^= x >> 30;
    x *= UINT64_C(0xbf58476d1ce4e5b9);
    x ^= x >> 27;
    x *= UINT64_C(0x94d049bb133111eb);
    x ^= x >> 31;

    return min + x % (max - min);
}

void nl_randomSetSeed(u64 seed) {
    randState = seed;
}

u64 nl_random() {
    randState = urand(randState, 0, UINT64_MAX);
    return randState;
}

u64 nl_rand(u64 seed, u64 min, u64 max) {
    return urand(seed, min, max);
}

u64 nl_hash(u64 input) {
    return lhash(input);
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

float getNoiseAt(NoiseContext* ctx, float x, float y) {
    if (ctx->chunkSize == 0) ctx->chunkSize = NOSLIB_DEFAULT_CHUNK_SIZE;
    u64 csize = ctx->chunkSize;

    u64 boundX = floorf(x / csize) * csize;
    u64 boundY = floorf(y / csize) * csize;

    u64 bx = (u64)boundX;
    u64 by = (u64)boundY;

    float min = ctx->rangeMin;
    float max = ctx->rangeMax;
    
    float v1, v2, v3, v4;
    float tx, ty, top, bottom;
    
    // using settings instead of pre-processor macros drops the performance by about 2%
    if (ctx->settings.use2hash != 0) {
        // this version is a little faster but it may come at a cost of worst randomness
        // none of the statistics measured in the README use this version
        if (ctx->settings.useAdvancedFrand) {
            v1 = frand(ctx->seed ^ hash2(bx, by), min, max);
            v2 = frand(ctx->seed ^ hash2(bx + csize, by), min, max);
            v3 = frand(ctx->seed ^ hash2(bx, by + csize), min, max);
            v4 = frand(ctx->seed ^ hash2(bx + csize, by + csize), min, max);
        } else {
            v1 = frand_instant(ctx->seed ^ hash2(bx, by), min, max);
            v2 = frand_instant(ctx->seed ^ hash2(bx + csize, by), min, max);
            v3 = frand_instant(ctx->seed ^ hash2(bx, by + csize), min, max);
            v4 = frand_instant(ctx->seed ^ hash2(bx + csize, by + csize), min, max);
        }
    } else {
        if (ctx->settings.useAdvancedFrand) {
            v1 = frand(hash3(ctx->seed, bx, by), min, max);
            v2 = frand(hash3(ctx->seed, bx + csize, by), min, max);
            v3 = frand(hash3(ctx->seed, bx, by + csize), min, max);
            v4 = frand(hash3(ctx->seed, bx + csize, by + csize), min, max);
        } else {
            v1 = frand_instant(hash3(ctx->seed, bx, by), min, max);
            v2 = frand_instant(hash3(ctx->seed, bx + csize, by), min, max);
            v3 = frand_instant(hash3(ctx->seed, bx, by + csize), min, max);
            v4 = frand_instant(hash3(ctx->seed, bx + csize, by + csize), min, max);
        }
    }

    switch (ctx->interVersion) {
        case SMOOTH_STEP:
            tx = smoothstep((x - boundX) / csize);
            ty = smoothstep((y - boundY) / csize);
            break;

        case LINEAR_INTERPOLATION:
            tx = (float)(x - boundX) / (float)csize;
            ty = (float)(y - boundY) / (float)csize;
            break;

        case SMOOTHER_STEP:
            tx = smootherstep((x - boundX) / csize);
            ty = smootherstep((y - boundY) / csize);
            break;

        case COSINE_INTERPOLATION:
            tx = cosineInterpolation((x - boundX) / csize);
            ty = cosineInterpolation((y - boundY) / csize);
            break;

        default:
            tx = smoothstep((x - boundX) / csize);
            ty = smoothstep((y - boundY) / csize);
            break;
    }

    top = lerp(v1, v2, tx);
    bottom = lerp(v3, v4, tx);

    return lerp(top, bottom, ty);
}

float nl_getNoiseAt(NoiseContext* ctx, size_t x, size_t y) {
    if (ctx->chunkSize == 0) ctx->chunkSize = NOSLIB_DEFAULT_CHUNK_SIZE;
    if (ctx->octaves < 1) {return getNoiseAt(ctx, x, y);}

    float amplitude = 1.0f;
    float frequency = ctx->freq;

    float total = 0.0f;
    float maxAmplitude = 0.0f;

    for (size_t i = 0; i < ctx->octaves; i++) {
        NoiseContext octave = *ctx;

        octave.seed = lhash(ctx->seed + i);

        total += getNoiseAt(&octave, (float)x * frequency, (float)y * frequency) * amplitude;

        maxAmplitude += amplitude;

        amplitude *= 0.5f;
        frequency *= 2.0f;
    }

    return total / maxAmplitude;
}

// chunkX and Y should be the topleft corners of a chunk
float* nl_getChunkAt(NoiseContext* ctx, size_t chunkX, size_t chunkY) {
    if (ctx->chunkSize == 0) ctx->chunkSize = NOSLIB_DEFAULT_CHUNK_SIZE;
    u64 csize = ctx->chunkSize;
    
    float* out = malloc(csize * csize * sizeof(float));
    if (out == NULL) return NULL;

    for (size_t y = 0; y < csize; y++) {
        for (size_t x = 0; x < csize; x++) {
            out[y * csize + x] = nl_getNoiseAt(ctx, x + chunkX, y + chunkY);
        }
    }

    return out;
}

// chunkX and Y should be the topleft corners of a chunk
void nl_getChunkInto(NoiseContext* ctx, size_t chunkX, size_t chunkY, float* dest) {
    if (dest == NULL) return;

    if (ctx->chunkSize == 0) ctx->chunkSize = NOSLIB_DEFAULT_CHUNK_SIZE;
    u64 csize = ctx->chunkSize;

    for (size_t y = 0; y < csize; y++) {
        for (size_t x = 0; x < csize; x++) {
            dest[y * csize + x] = nl_getNoiseAt(ctx, x + chunkX, y + chunkY);
        }
    }
}

float* nl_getBatch(NoiseContext* ctx, size_t x1, size_t y1, size_t x2, size_t y2) {
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

void nl_getBatchInto(NoiseContext* ctx, size_t x1, size_t y1, size_t x2, size_t y2, float* dest) {
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
float getNoiseAt1D(NoiseContext* ctx, float x) {
    if (ctx->chunkSize == 0) ctx->chunkSize = NOSLIB_DEFAULT_CHUNK_SIZE;
    size_t sz = ctx->chunkSize;

    float sectionStart = floorf(x / sz) * sz;
    float sectionEnd = sectionStart + sz;

    float v1, v2;

    if (ctx->settings.useAdvancedFrand != 0) {
        v1 = frand(hash2(ctx->seed, sectionStart), ctx->rangeMin, ctx->rangeMax);
        v2 = frand(hash2(ctx->seed, sectionEnd), ctx->rangeMin, ctx->rangeMax);
    } else {
        v1 = frand_instant(hash2(ctx->seed, sectionStart), ctx->rangeMin, ctx->rangeMax);
        v2 = frand_instant(hash2(ctx->seed, sectionEnd), ctx->rangeMin, ctx->rangeMax);
    }
    
    float t = (x - sectionStart) / sz;

    switch (ctx->interVersion) {
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

float nl_getNoiseAt1D(NoiseContext* ctx, float x) {
    if (ctx->chunkSize == 0) ctx->chunkSize = NOSLIB_DEFAULT_CHUNK_SIZE;
    if (ctx->octaves < 1) {return getNoiseAt1D(ctx, x);}

    float amplitude = 1.0f;
    float frequency = ctx->freq;

    float total = 0.0f;
    float maxAmplitude = 0.0f;

    for (size_t i = 0; i < ctx->octaves; i++) {
        NoiseContext octave = *ctx;

        octave.seed = lhash(ctx->seed + i);

        total += getNoiseAt1D(&octave, x * frequency) * amplitude;

        maxAmplitude += amplitude;

        amplitude *= 0.5f;
        frequency *= 2.0f;
    }

    return total / maxAmplitude;
}

float* nl_getSectionAt1D(NoiseContext* ctx, float x1, float x2) {
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

void nl_getSectionInto1D(NoiseContext* ctx, float x1, float x2, float* dest) {
    size_t minX = (size_t)MIN(x1, x2);
    size_t maxX = (size_t)MAX(x1, x2);

    if (maxX - minX == 0) return;

    for (size_t i = minX; i < maxX; i++) {
        dest[i - minX] = nl_getNoiseAt1D(ctx, (float)i);
    }
}

float* nl_getChunkAt1D(NoiseContext* ctx, float x) {
    if (ctx->chunkSize == 0) ctx->chunkSize = NOSLIB_DEFAULT_CHUNK_SIZE;
    size_t sz = ctx->chunkSize;

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

void nl_getChunkInto1D(NoiseContext* ctx, float x, float* dest) {
    if (ctx->chunkSize == 0) ctx->chunkSize = NOSLIB_DEFAULT_CHUNK_SIZE;
    size_t sz = ctx->chunkSize;

    if (sz == 0 || x < 0.0f) return;

    size_t sectionStart = ((size_t)x / sz) * sz;
    size_t sectionEnd = sectionStart + sz;

    for (size_t i = sectionStart; i < sectionEnd; i++) {
        dest[i - sectionStart] = nl_getNoiseAt1D(ctx, (float)i);
    }
}

size_t nl_getChunkSizeB(NoiseContext* ctx) {
    if (ctx->chunkSize == 0) ctx->chunkSize = NOSLIB_DEFAULT_CHUNK_SIZE;
    return (sizeof(float) * ctx->chunkSize * ctx->chunkSize);
}

size_t nl_getChunkSizeB1D(NoiseContext* ctx) {
    if (ctx->chunkSize == 0) ctx->chunkSize = NOSLIB_DEFAULT_CHUNK_SIZE;
    return (sizeof(float) * ctx->chunkSize);
}

size_t nl_getBatchSizeB(size_t x1, size_t y1, size_t x2, size_t y2) {
    size_t lx = MAX(x1, x2) - MIN(x1, x2);
    size_t ly = MAX(y1, y2) - MIN(y1, y2);

    if (lx == 0 || ly == 0) return 0;

    return (sizeof(float) * lx * ly);
}

size_t nl_getSectionSizeB(float x1, float x2) {
    return (sizeof(float) * MAX(x1, x2) - MIN(x1, x2));
}

float nl_clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

float nl_lerp(float a, float b, float t) {
    return lerp(a, b, t);
}

float nl_scale(float value, float scale) {
    return value * scale;
}

void nl_scaleArr(float* data, size_t len, float scale) {
    if (data == NULL) return;
    if (len == 0) return;

    for (size_t i = 0; i < len; i++) {
        data[i] = nl_scale(data[i], scale);
    }
}

float nl_remap(float value, float in_min, float in_max, float out_min, float out_max) {
    return out_min + ((value - in_min) / (in_max - in_min)) * (out_max - out_min);
}

void nl_remapArr(float* data, size_t len, float in_min, float in_max, float out_min, float out_max) {
    if (data == NULL) return;
    if (len == 0) return;

    for (size_t i = 0; i < len; i++) {
        data[i] = nl_remap(data[i], in_min, in_max, out_min, out_max);
    }
}

float nl_fract(float value) {
    return value - floorf(value);
}

float nl_move(float value, float goal) {
    if (value == goal) return NOSLIB_MOVE_RETURN;
    if (value < goal) return 2;
    if (value > goal) return -2;

    return 0;
}

NoiseContext* nl_initContext() {
    NoiseContext* out = malloc(sizeof(NoiseContext));
    if (out == NULL) return NULL;

    *out = NOSLIB_DEFAULT_CONTEXT;

    return out;
}

void nl_contextSetSeed(NoiseContext* ctx, u64 seed) {
    if (ctx == NULL) return;
    ctx->seed = seed;
}

void nl_contextSetChunkSize(NoiseContext* ctx, size_t chunkSize) {
    if (ctx == NULL) return;
    if (chunkSize == 0) chunkSize = NOSLIB_DEFAULT_CHUNK_SIZE;

    ctx->chunkSize = chunkSize;
}

void nl_contextSetOctaves(NoiseContext* ctx, size_t octaves) {
    if (ctx == NULL) return;

    ctx->octaves = octaves;
}

void nl_contextSetFrequency(NoiseContext* ctx, float freq) {
    if (ctx == NULL) return;

    if (freq == 0) freq = 1.0f;

    ctx->freq = freq;
}

void nl_contextSetRange(NoiseContext* ctx, float min, float max) {
    if (ctx == NULL) return;
    ctx->rangeMin = MIN(min, max);
    ctx->rangeMax = MAX(min, max);
}

void nl_contextSetInterpolation(NoiseContext* ctx, NoiseInterpolationVersions interVersion) {
    if (ctx == NULL) return;
    ctx->interVersion = interVersion;
}

float nl_min(float* data, size_t len) {
    if (data == NULL || len == 0) return -1;
    
    float min = data[0];

    for (size_t i = 0; i < len; i++) {
        if (data[i] < min) min = data[i];
    }

    return min;
}

float nl_max(float* data, size_t len) {
    if (data == NULL || len == 0) return -1;

    float max = data[0];

    for (size_t i = 0; i < len; i++) {
        if (data[i] > max) max = data[i];
    }

    return max;
}

float nl_avg(float* data, size_t len) {
    if (data == NULL || len == 0) return -1;

    float avg = 0;

    for (size_t i = 0; i < len; i++) {
        avg += data[i];
    }

    return avg / len;
}

float* nl_blend(float* noise1, float* noise2, size_t len) {
    if (noise1 == NULL || noise2 == NULL || len == 0) return NULL;

    float* out = malloc(sizeof(float) * len);
    if (out == NULL) return NULL;

    for (size_t i = 0; i < len; i++) {
        out[i] = (noise1[i] + noise2[i]) / 2;
    }

    return out;
}