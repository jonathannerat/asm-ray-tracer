# Ray Tracer implemented in x64 ASM using SIMD Instructions

This is a project intended to be presented as a final work for the Computer's
Architecture and Organization course at FCEN - UBA.

The original implementation in C++ is
[here](https://github.com/jonathannerat/ray-tracing-iow-cpp), and follows the
amazing [guide](https://raytracing.github.io) by Peter Shirley. This project
reimplements everything in C, and then reimplements the core (where vector-heavy
operations are handled) in x86_64 asm using SIMD instructions to boost performance.
