#include "core.h"
#include <stdio.h>

Scene *scene_new();
Scene *scene_new_from_file(const char *path);
void scene_dump(Scene *s, FILE *f);
void scene_free(Scene *s);
