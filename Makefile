TARGET=rt
CC=gcc
SRC=main.c vec3.c  util.c camera.c ray.c Scene.c hittable/List.c hittable/Plane.c hittable/Box.c
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

debug: CFLAGS += -g
debug: ${TARGET}

clean:
	-rm ${TARGET}
	-rm ${OBJ}

.PHONY: all clean options
