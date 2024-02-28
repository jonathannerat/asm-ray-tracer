#include "config.h"
#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_SEED 0x20221218 //   
#define DEFAULT_MEASUREMENTS 10
#define DEFAULT_PATH NULL // stdin

Config parse_config(int argc, char **argv) {
  Config config = {DEFAULT_PATH, DEFAULT_SEED, DEFAULT_MEASUREMENTS, TRACER_C,
                   MEASURE_NONE};

  for (int i = 1; i < argc;) {
    if (argv[i][0] == '-') {
      switch (argv[i][1]) {
      case 'c':
        config.tracer_type = TRACER_C;
        i++;
        break;
      case 'a':
        config.tracer_type = TRACER_ASM;
        i++;
        break;
      case 's':
        config.scene_path = argv[i + 1];
        i += 2;
        break;
      case 'r':
        config.seed = atoi(argv[i + 1]);
        i += 2;
        break;
      case 'm':
        config.measure_type = (enum measure_type)atoi(argv[i + 1]);
        i += 2;
        break;
      case 'n':
        config.measurements = atoi(argv[i + 1]);
        i += 2;
        break;
      case 'h':
        print_usage(argv[0]);
        exit(EXIT_SUCCESS);
        break;
      default:
        fprintf(stderr, "Invalid option: %s", argv[i]);
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
        break;
      }
    } else {
      fprintf(stderr, "Invalid option: %s", argv[i]);
      print_usage(argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  return config;
}

void print_usage(char *binary) {
  fprintf(stderr, "Usage: %s [-c|-a] [-s SCENE] [-r SEED]\n", binary);
  char *usage =
    "-c        Run using C implementation (default)\n"
    "-a        Run using ASM implementation\n"
    "-s SCENE  Render the scene described in SCENE\n"
    "-r SEED   Seed the RNG with SEED\n"
    "-n NUM    Numbers of measurements to take for average\n"
    "-m TYPE   Measurement type:\n"
    "          - 0: Don't measure (default) \n"
    "          - 1: Clock cycle count\n"
    "          - 2: Real nanoseconds ellapsed\n"
    "          - 3: CPU nanoseconds ellapsed\n"
    "          If not specified (or 0), prints image to stdout. Otherwise, prints the \n"
    "          measurement\n";
  fprintf(stderr, "%s", usage);
}
