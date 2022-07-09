TARGET=rt
CC=gcc
SRC=main.c vec3.c  util.c camera.c ray.c array.c Scene.c Material.c \
    hittable/Plane.c hittable/Sphere.c hittable/Triangle.c \
    hittable/List.c hittable/Box.c hittable/KDTree.c
OBJ=${SRC:.c=.o}
CFLAGS=-std=c99 -pedantic -Wall -O2
LDFLAGS=-lm

all: options ${TARGET}

options:
	@echo ${TARGET} build options:
	@echo "CFLAGS     = ${CFLAGS}"
	@echo "LDFLAGS    = ${LDFLAGS}"
	@echo "CC         = ${CC}"
	@echo "OBJ        = ${OBJ}"

.c.o:
	${CC} -c -o $@ ${CFLAGS} $<

${TARGET}: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

debug: CFLAGS += -g -DDEBUG
debug: ${TARGET}

clean:
	-rm ${TARGET}
	-rm ${OBJ}

.PHONY: all clean options
