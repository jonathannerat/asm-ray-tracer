TARGET=rt
CC=gcc
NASM=nasm
SRC=main.c util.c array.c hittable/List.c hittable/Box.c hittable/Plane.c hittable/Sphere.c hittable/Triangle.c Scene.c Material.c hittable/KDTree.c
OBJ=${SRC:.c=.o}
CFLAGS=-std=c99 -pedantic -Wall
LDFLAGS=-lm -no-pie
NASMFLAGS=-f elf64 -Wall

all: options targets

options:
	@echo ${TARGET} build options:
	@echo "CFLAGS     = ${CFLAGS}"
	@echo "LDFLAGS    = ${LDFLAGS}"
	@echo "NASMFLAGS  = ${NASMFLAGS}"
	@echo "CC         = ${CC}"
	@echo "OBJ        = ${OBJ}"

asm_core.o: asm_core.s
	${NASM} $< -o $@ ${NASMFLAGS}

.c.o:
	${CC} -c -o $@ ${CFLAGS} $<

targets: ${TARGET}-c ${TARGET}-asm

${TARGET}-c: CFLAGS += -O2
${TARGET}-c: ${OBJ} c_core.o
	${CC} -o $@ ${OBJ} c_core.o ${LDFLAGS}

${TARGET}-asm: ${OBJ} asm_core.o
	${CC} -o $@ ${OBJ} asm_core.o ${LDFLAGS}

debug-c: CFLAGS += -g -DDEBUG
debug-c: rt-c

debug-asm: CFLAGS += -g -DDEBUG
debug-asm: NASMFLAGS += -gdwarf
debug-asm: rt-asm

debug: debug-c debug-asm

scenes: options targets
	[ -d out ] || mkdir out
	for core in c asm; do \
		printf "\nUsing core %s" "$$core" ; \
		for f in scenes/*; do \
			name="$${f#*/}" ; \
			printf '\n\t- Generating scene "%s"...\n' "$$name" ; \
			./rt-"$$core" "$$f" | convert ppm:- "out/$${core}_core-$${name}.png" ; \
		done ; \
	done
	echo
	sxiv out &

clean:
	-rm ${TARGET}-c ${TARGET}-asm
	-rm ${OBJ} c_core.o asm_core.o
	[ -d out ] && rm -r out

.PHONY: all clean options targets debub debug-c debug-asm
