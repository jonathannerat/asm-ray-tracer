# Ray Tracer implemented in x64 ASM using SIMD Instructions

This is a project intended to be presented as a final work for the Computer's
Architecture and Organization course at FCEN - UBA.

The original implementation in C++ is
[here](https://github.com/jonathannerat/ray-tracing-iow-cpp), and follows the
amazing [guide](https://raytracing.github.io) by Peter Shirley. This project
reimplements everything in C and x86_64 ASM using SIMD instructions to boost
performance.

## Building

Requires `python` to generate the `build.ninja`, `ninja` to build the project,
and optionally requires `dotenv` to automatically rebuild build.ninja on any
change of the environment variable file `.env` (mostly for development).

First run

```sh
python gen_ninja.py
```

, then run

```sh
ninja
```

to build the `ray_tracer` binary, which contains both C and ASM implementations.

## Usage

```
Usage: %s [-c|-a] [-s SCENE] [-r SEED]
-c        Run using C implementation (default)
-a        Run using ASM implementation
-s SCENE  Render the scene described in SCENE
-r SEED   Seed the RNG with SEED
```

The binary accepts the `-s [SCENE_FILE]` flag to specify the path to a scene,
defaulting to stdin if the flag is missing. It also accepts `-c` and `-a` to use
the C and ASM implementation respectively, defaulting to C if none is specified.
You can also modify the seed number for the RNG using `-r`.
