#ifndef NOSLIB_H
#define NOSLIB_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

typedef uint8_t u8;
typedef uint64_t u64;
typedef int64_t i64;

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#ifndef NOSLIB_DEFAULT_CHUNK_SIZE
    #define NOSLIB_DEFAULT_CHUNK_SIZE 50
#endif

#ifndef NOSLIB_MOVE_RETURN
    #define NOSLIB_MOVE_RETURN 1
#endif

#define NOSLIB_DEFAULT_CONTEXT ((NoiseContext) { \
    .seed = 1337, \
    .chunkSize = 10, \
    .octaves = 1, \
    .freq = 1.0f, \
    .rangeMax = 100.0f, \
    .rangeMin = 0.0f, \
    .settings = (NoiseContextExtraSettings) { \
        .use2hash = 0, \
        .useAdvancedFrand = 0, \
    } \
})

typedef enum {
    SMOOTH_STEP,
    SMOOTHER_STEP,
    COSINE_INTERPOLATION,
    LINEAR_INTERPOLATION
} NoiseInterpolationVersions;

typedef struct {
    // this option speeds up generation by a little bit but it might hurt the randomness
    u8 use2hash; // 0 recommended
    u8 useAdvancedFrand; // 0 recommended
} NoiseContextExtraSettings;

typedef struct {
    u64 seed;
    size_t chunkSize;
    size_t octaves;

    float freq;
    float rangeMin;
    float rangeMax;

    NoiseInterpolationVersions interVersion; // defaulted to SMOOTH_STEP

    NoiseContextExtraSettings settings; 
} NoiseContext;

NoiseContext* nl_initContext();
void nl_contextSetSeed(NoiseContext* ctx, u64 seed);
void nl_contextSetChunkSize(NoiseContext* ctx, size_t chunkSize);
void nl_contextSetOctaves(NoiseContext* ctx, size_t octaves);
void nl_contextSetFrequency(NoiseContext* ctx, float freq);
void nl_contextSetRange(NoiseContext* ctx, float min, float max);
void nl_contextSetInterpolation(NoiseContext* ctx, NoiseInterpolationVersions interVersion);

float nl_getNoiseAt(NoiseContext* ctx, size_t x, size_t y);

// chunkX and Y should be the topleft corners of a chunk
float* nl_getChunkAt(NoiseContext* ctx, size_t chunkX, size_t chunkY); 
void nl_getChunkInto(NoiseContext* ctx, size_t chunkX, size_t chunkY, float* dest); 
float* nl_getBatch(NoiseContext* ctx, size_t x1, size_t y1, size_t x2, size_t y2);
void nl_getBatchInto(NoiseContext* ctx, size_t x1, size_t y1, size_t x2, size_t y2, float* dest);


// ************ //
// 1D functions //
// ************ //

float nl_getNoiseAt1D(NoiseContext* ctx, float x);
float* nl_getSectionAt1D(NoiseContext* ctx, float x1, float x2);
void nl_getSectionInto1D(NoiseContext* ctx, float x1, float x2, float* dest);
float* nl_getChunkAt1D(NoiseContext* ctx, float x);
void nl_getChunkInto1D(NoiseContext* ctx, float x, float* dest);

size_t nl_getChunkSizeB(NoiseContext* ctx);
size_t nl_getChunkSizeB1D(NoiseContext* ctx); // sorry for the name DDD
size_t nl_getBatchSizeB(size_t x1, size_t y1, size_t x2, size_t y2);
size_t nl_getSectionSizeB(float x1, float x2);

float nl_clamp(float value, float min, float max);
float nl_lerp(float a, float b, float t);
float nl_scale(float value, float scale);
void  nl_scaleArr(float* data, size_t len, float scale);
float nl_remap(float value, float in_min, float in_max, float out_min, float out_max);
void  nl_remapArr(float* data, size_t len, float in_min, float in_max, float out_min, float out_max);
float nl_fract(float value);

// returns a value either 2 if value is less than goal, returns -2 if value is bigger than goal
// and returns 1 if they are equal. also return 1 can be changed to any value with defenition of
// #define NOSLIB_MOVE_RETURN <any value>
// make sure to do this before including the library
float nl_move(float value, float goal);

void nl_free(float* chunk);

// private functions exposed
void nl_randomSetSeed(u64 seed);
u64 nl_random();
u64 nl_rand(u64 seed, u64 min, u64 max);
u64 nl_hash(u64 input);

// NOTE //
// into functions use for loops to iterate through the whole batch, may cost some performance.
// also into functions expect destenations to be exact size or bigger than expected. if else
// there is no safeguards against it and program is gonna crash. use getSize functions for this.

#endif