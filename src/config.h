#ifndef CONFIG_H
#define CONFIG_H

enum tracer_type { TRACER_C, TRACER_ASM };
enum measure_type { MEASURE_NONE = 0, MEASURE_CLOCK_CYCLES, MEASURE_REAL_TIME_NANO, MEASURE_CPU_TIME_NANO };

typedef struct {
  char *scene_path;
  int seed;
  int measurements;
  enum tracer_type tracer_type;
  enum measure_type measure_type;
} Config;

Config parse_config(int argc, char **argv);
void print_usage(char *);

#endif // CONFIG_H
