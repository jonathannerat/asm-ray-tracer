TARGET=rt
CC=gcc
NASM=nasm
SRC=main.c util.c array.c hittable/List.c hittable/Box.c hittable/Plane.c hittable/Sphere.c hittable/Triangle.c Scene.c Material.c hittable/KDTree.c
OBJ=${SRC:.c=.o}
IMAGES=$(patsubst scenes/%,out/%.png,$(wildcard scenes/*))
CFLAGS=-std=c99 -pedantic -Wall
LDFLAGS=-lm -no-pie
NASMFLAGS=-f elf64 -Wall

all: options targets

asm_core.o: asm_core.s
	${NASM} $< -o $@ ${NASMFLAGS}

.c.o:
	${CC} -c -o $@ ${CFLAGS} $<

out/%.png: scenes/%
	@scene="$<" ; \
	name="$${scene#*/}" ; \
	echo ;\
	echo "- Generating scene \"$$name\"" ; \
	echo "\t- C: " ; \
	./rt-c   "$$scene" | convert ppm:- "out/$$name-c.png" ; \
	echo ; \
	echo "\t- ASM: " ; \
	./rt-asm "$$scene" | convert ppm:- "out/$$name-asm.png" ;

${TARGET}-c: CFLAGS += -O2
${TARGET}-c: ${OBJ} c_core.o
	${CC} -o $@ ${OBJ} c_core.o ${LDFLAGS}

${TARGET}-asm: ${OBJ} asm_core.o
	${CC} -o $@ ${OBJ} asm_core.o ${LDFLAGS}

options:
	@echo ${TARGET} build options:
	@echo "CFLAGS     = ${CFLAGS}"
	@echo "LDFLAGS    = ${LDFLAGS}"
	@echo "NASMFLAGS  = ${NASMFLAGS}"
	@echo "CC         = ${CC}"
	@echo "OBJ        = ${OBJ}"

targets: ${TARGET}-c ${TARGET}-asm

debug: CFLAGS += -g -DDEBUG
debug: NASMFLAGS += -gdwarf
debug: targets

images: outdir options targets ${IMAGES}
	sxiv out &

outdir:
	mkdir -p out

clean:
	-rm ${TARGET}-c ${TARGET}-asm
	-rm ${OBJ} c_core.o asm_core.o
	[ -d out ] && rm -r out || true

.PHONY: all options targets debug images outdir clean
