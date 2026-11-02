# NOSLIB

*NOSLIB* is a 2D noise library written in C99 that aims for simple user interface and fast generation speeds.
*NOSLIB* is written in C99 and uses no external dependencies except the standard C library

## How to use *NOSLIB*

### Setting up the context

*NOSLIB* is super simple to use. theres only a single initialization step and thats creating the context.

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

this code states what the noise produced should look like.
also you dont hav to write it out all the time its already defined in the header.
just use this.

```C
NoiseContext ctx = NOSLIB_DEFAULT_CONTEXT;
```

and its all the same values as example from before.

### Generating noise

Generating noise using *NOSLIB* is as simple as it gets. there are just 2 functions
to generate noise.

```C
// pass the context and x and y coordinates
float value = nl_getNoiseAt(ctx, 10, 10);

// generate the whole chunk
// pass the context and coordinate of topleft corner of the chunk we want
float* chunk = nl_getChunkAt(ctx, 0, 0);

// make sure to free the values by either using custom function or just free()
nl_freeChunk(chunk); // or just free(chunk)
```

## Extra settings

To make noise generation extra smooth *NOSLIB* offers few algorithms.
To use them you have to modify your context, specifically the interVersion variable.
Choises are:

```C
SMOOTH_STEP, // this is the default
SMOOTHER_STEP,
COSINE_INTERPOLATION,
BILINEAR_INTERPOLATION
```
