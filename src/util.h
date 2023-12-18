#ifndef UTIL_H
#define UTIL_H

void d(const char *fmt, ...);

#ifdef DEBUG
#define D(...) d("[debug] " __VA_ARGS__)
#else
#define D(...) d(NULL)
#endif

#define EPS 1e-4
#define MAX_BUF_SIZE 256

#define MAX(a, b) ((a) < (b) ? (b) : (a))
#define MIN(a, b) ((a) > (b) ? (b) : (a))
#define MIN3(a, b, c) (MIN(MIN((a), (b)), (c)))
#define MAX3(a, b, c) (MAX(MAX((a), (b)), (c)))

float frand();
char *strfind(char *c, char f);
#endif // UTIL_H
