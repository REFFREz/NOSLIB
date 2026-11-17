# NOSLIB

*NOSLIB* is a 2D noise library written in C99 that aims for a simple user interface and fast generation speeds.
*NOSLIB* is loyal to an unusual method to generate noise. the information about this method is in
sources tab.

## How to use *NOSLIB*

### Setting up the context

*NOSLIB* is super simple to use. there's only a single initialization step and thats creating the context

```C
NoiseContext ctx = {
    .seed = 1337,
    .chunkSize = 10,
    .octaves = 1,
    .freq = 1.0f,
    .rangeMax = 100.0f,
    .rangeMin = 0.0f
};
```

this code states what the noise being produced should look like.
also you don't have to write it out all the time it's already defined in the header.
just use this.

```C
NoiseContext ctx = NOSLIB_DEFAULT_CONTEXT;
```

and it's all the same values as the example from before.

### Generating noise

Generating noise using *NOSLIB* is as simple as it gets.
to generate noise use this example.

```C
// pass the context and x and y coordinates
float value = nl_getNoiseAt(&ctx, 10, 10);

// generate the whole chunk
// pass the context and coordinate of the top-left corner of the chunk we want
float* chunk = nl_getChunkAt(&ctx, 0, 0);

// make sure to free the values by either using custom function or just free()
nl_free(chunk); // or just free(chunk)
```

## Extra settings

To make noise generation extra smooth *NOSLIB* offers a few algorithms.
To use them you have to modify your context, specifically the interVersion variable.
Choices are:

```C
SMOOTH_STEP, // this is the default
SMOOTHER_STEP,
COSINE_INTERPOLATION,
LINEAR_INTERPOLATION
```

## 1D noise

*NOSLIB* offers a way to generate 1D noise directly without needing to use normal 2D functions.
this is a really nice feature if you worry about performance since this step massivly speeds it up.

heres the functions:

```C
float nl_getNoiseAt1D(NoiseContext* ctx, float x);
float* nl_getSectionAt1D(NoiseContext* ctx, float x1, float x2);
void nl_getSectionInto1D(NoiseContext* ctx, float x1, float x2, float* dest);
float* nl_getChunkAt1D(NoiseContext* ctx, float x);
void nl_getChunkInto1D(NoiseContext* ctx, float x, float* dest);
```

also here are functions to get sizes that you want to allocate if you want to use "into" functions.

```C
size_t nl_getChunkSizeB(NoiseContext* ctx);
size_t nl_getChunkSizeB1D(NoiseContext* ctx); // sorry for the name DDD
size_t nl_getBatchSizeB(size_t x1, size_t y1, size_t x2, size_t y2);
size_t nl_getSectionSizeB(float x1, float x2);
```

## Extra functions

*NOSLIB* offers many extra functions to manipulate data.

```C
float nl_clamp(float value, float min, float max);
float nl_lerp(float a, float b, float t);
float nl_scale(float value, float scale);
void  nl_scaleArr(float* data, size_t len, float scale);
float nl_remap(float value, float in_min, float in_max, float out_min, float out_max);
void  nl_remapArr(float* data, size_t len, float in_min, float in_max, float out_min, float out_max);
float nl_fract(float value);
float nl_move(float value, float goal);
```

## Sources

The main inspiration comes from this youtube video by PortalRunner:
[Watch the video](https://www.youtube.com/watch?v=p-k5MPhBSjk)
skip to about 5:00.

The inspiration is taken from that video but since PortalRunner was dealing with much more limited hardware so i added different interpolation functions, more advanced random function and such.

## Performance note

even tho *NOSLIB* aims for very high speeds at low cost it can still be very slow if used incorrectly.
it is recommended to use at least -O2 flag to give it a huge performance boost.
here is some measurements on Ryzen 9 5900X cpu testing a 10000x10000 world creation with this settings:

```C
.chunkSize = 100
.freq = 1
.interVersion = SMOOTH_STEP
.octaves = 0
.rangeMax = 100
.rangeMin = 0
.seed = 0
```

```NO_LANGUAGE
optimization - sps(samples per second) - time to generate
-O0 -> 8.04 million sps  -> 12.4s
-O1 -> 22.58 million sps -> 4.4s
-O2 -> 29.65 million sps -> 3.37s
-O3 -> 30.27 million sps -> 3.30s
```
