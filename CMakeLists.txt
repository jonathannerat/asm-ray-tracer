cmake_minimum_required(VERSION 3.16)

project(ray-tracer)

enable_language(C ASM_NASM)

if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE Release)
endif ()

set (CMAKE_C_STANDARD 99)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic -Wpedantic -Wall -Wextra")
set (CMAKE_C_FLAGS_DEBUG "-g")
set (CMAKE_EXE_LINKER_FLAGS "-no-pie")
set (CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
set (CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -Wall")
set (CMAKE_ASM_FLAGS_DEBUG "-g")

set(C_TRACER
    src/c/tracer.c
    src/c/vec3.c
    src/c/intersection.c
    src/c/shading.c)

set(SOURCE_FILES
    src/util.c
    src/structures/array.c
    src/scene/Scene.c
    src/scene/Camera.c
    src/surfaces.c)

add_executable(rt-c src/main.c ${SOURCE_FILES} ${C_TRACER})
target_link_libraries(rt-c m)

# add_executable(rt-asm src/main.c src/asm_core.nasm ${SOURCE_FILES})
# target_link_libraries(rt-asm m)
#
# add_executable(perf-c src/perf.c src/c_core.c ${SOURCE_FILES})
# target_link_libraries(perf-c m)
#
# add_executable(perf-asm src/perf.c src/asm_core.nasm ${SOURCE_FILES})
# target_link_libraries(perf-asm m)
