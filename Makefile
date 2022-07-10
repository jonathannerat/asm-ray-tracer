TARGET=rt
CC=gcc
NASM=nasm
SRC=main.c util.c array.c Scene.c Material.c \
    hittable/Plane.c hittable/Sphere.c hittable/Triangle.c \
    hittable/List.c hittable/Box.c hittable/KDTree.c
OBJ=${SRC:.c=.o}
CFLAGS=-std=c99 -pedantic -Wall -O2
LDFLAGS=-lm
NASMFLAGS=-f elf64 -Wall

all: options targets

options:
	@echo ${TARGET} build options:
	@echo "CFLAGS     = ${CFLAGS}"
	@echo "LDFLAGS    = ${LDFLAGS}"
	@echo "NASMFLAGS  = ${NASMFLAGS}"
	@echo "CC         = ${CC}"
	@echo "OBJ        = ${OBJ}"

asm_core.o: asm_core.asm
	${NASM} $< -o $@ ${NASMFLAGS}

.c.o:
	${CC} -c -o $@ ${CFLAGS} $<

targets: ${TARGET}-c ${TARGET}-asm

${TARGET}-c: ${OBJ} c_core.o
	${CC} -o $@ ${OBJ} c_core.o ${LDFLAGS}

${TARGET}-asm: ${OBJ} asm_core.o
	${CC} -o $@ ${OBJ} asm_core.o ${LDFLAGS}

debug-c: CFLAGS += -g -DDEBUG
debug-c: rt-c

debug-asm: NASMFLAGS += -g -F DWARF
debug-asm: rt-asm

debug: debug-c debug-asm

clean:
	-rm ${TARGET}-c ${TARGET}-asm
	-rm ${OBJ} c_core.o asm_core.o

.PHONY: all clean options
