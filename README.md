# NOSLIB

*NOSLIB* is a 2D noise library written in C99 that aims for a simple user interface and fast generation speeds.
*NOSLIB* uses an unusual method to generate noise. the information about this method is in
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
float value = nl_getNoiseAt(ctx, 10, 10);

// generate the whole chunk
// pass the context and coordinate of the top-left corner of the chunk we want
float* chunk = nl_getChunkAt(ctx, 0, 0);

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
float nl_getNoiseAt1D(NoiseContext ctx, float x);
float* nl_getSectionAt1D(NoiseContext ctx, float x1, float x2);
float* nl_getChunkAt1D(NoiseContext ctx, float x);
```

## Sources

The main inspiration comes from this youtube video by PortalRunner:
[Watch the video](https://www.youtube.com/watch?v=p-k5MPhBSjk)
skip to about 5:00.

The inspiration is taken from that video but since PortalRunner was dealing with much more limited hardware so i added different interpolation functions, more advanced random function and such.
