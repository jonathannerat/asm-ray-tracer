#include "config.h"
#include "scene/Scene.h"
#include "structures/array.h"
#include "tracer.h"
#include "util.h"
#include <inttypes.h>
#include <stdio.h>
#include <time.h>

uint64_t get_time_measure(enum measure_type);
void run_tracer(enum tracer_type, Scene *, bool);

int main(int argc, char **argv) {
  Scene *s;
  Config c = parse_config(argc, argv);

  // Parse scene config
  if (c.scene_path)
    s = scene_new_from_file(c.scene_path);
  else
    s = scene_new_from_stdin();

  // Seed RNG
  mtsrand(c.seed);

  if (c.measure_type == MEASURE_NONE) {
    run_tracer(c.tracer_type, s, false);
    scene_dump(s, stdout);
  } else {
    uint64_t acc = 0;

    for (int i = 0; i < c.measurements; i++) {
      fprintf(stderr, "Taking measurement %d/%d\t\t\r", i + 1, c.measurements);
      fflush(stderr);
      uint64_t start = get_time_measure(c.measure_type);
      run_tracer(c.tracer_type, s, true);
      uint64_t end = get_time_measure(c.measure_type);

      acc += end - start;
    }
    fputc('\n', stderr);

    printf("%" PRIu64 "\n", acc / (int64_t)c.measurements);
  }

  scene_free(s);

  return 0;
}

uint64_t exec_rdtsc() {
  uint32_t lo, hi;
  __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
  return ((uint64_t)lo) | (((uint64_t)hi) << 32);
}

uint64_t get_time_measure(enum measure_type mtype) {
  uint64_t measure;

  if (mtype == MEASURE_NONE) {
    measure = 0;
  } else if (mtype == MEASURE_CLOCK_CYCLES) {
    measure = exec_rdtsc();
  } else {
    struct timespec t;
    clockid_t clock =
      mtype == MEASURE_REAL_TIME_NANO ? CLOCK_REALTIME : CLOCK_PROCESS_CPUTIME_ID;
    clock_gettime(clock, &t);
    measure = t.tv_sec * 1e9 + t.tv_nsec;
  }

  return measure;
}

void run_tracer(enum tracer_type ttype, Scene *s, bool measuring) {
  if (ttype == TRACER_C)
    tracer_c(s, measuring);
  else
    tracer_asm(s->camera, s->output, s->world, s->framebuffer, measuring);
}
