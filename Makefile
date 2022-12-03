TARGET=rt
CC=gcc
NASM=nasm
SRC=util.c array.c hittable/List.c hittable/Box.c hittable/Plane.c hittable/Sphere.c hittable/Triangle.c Scene.c Material.c hittable/KDTree.c
INFORMESRC = $(wildcard informe/*.tex) informe/recursos.bib
OBJ=${SRC:.c=.o}
IMAGES=$(patsubst scenes/%,out/%.png,$(wildcard scenes/*))
CFLAGS=-std=c99 -pedantic -Wall -O2
LDFLAGS=-lm -no-pie
NASMFLAGS=-f elf64 -Wall
LATEXFLAGS=--halt-on-error -output-directory build

all: options targets informe.pdf

asm_core.o: asm_core.s
	${NASM} $< -o $@ ${NASMFLAGS}

.c.o:
	${CC} -c -o $@ ${CFLAGS} $<

out/%.png: scenes/%
	@scene="$<" ; \
	name="$${scene#*/}" ; \
	echo ;\
	printf "• Generating scene \"%s\"\n" "$$name" ; \
	printf "\t• C: " ; \
	( ./rt-c   "$$scene" | convert ppm:- "out/$$name-c.png" ) ; \
	echo ; \
	printf "\t• ASM: " ; \
	( ./rt-asm "$$scene" | convert ppm:- "out/$$name-asm.png" ) ; \
	echo

${TARGET}-c: main.o c_core.o ${OBJ}
	${CC} -o $@ main.o c_core.o ${OBJ} ${LDFLAGS}

${TARGET}-asm: main.o asm_core.o ${OBJ}
	${CC} -o $@ main.o asm_core.o ${OBJ} ${LDFLAGS}

perf-asm: perf.o asm_core.o ${OBJ}
	${CC} -o $@ perf.o asm_core.o ${OBJ} ${LDFLAGS}

perf-c: perf.o c_core.o ${OBJ}
	${CC} -o $@ perf.o c_core.o ${OBJ} ${LDFLAGS}

perf: perf-asm perf-c

options:
	@echo ${TARGET} build options:
	@echo "CFLAGS     = ${CFLAGS}"
	@echo "LDFLAGS    = ${LDFLAGS}"
	@echo "NASMFLAGS  = ${NASMFLAGS}"
	@echo "CC         = ${CC}"
	@echo "OBJ        = ${OBJ}"

targets: ${TARGET}-c ${TARGET}-asm

debug: CFLAGS += -g -O0 -DDEBUG
debug: NASMFLAGS += -gdwarf
debug: options targets

images: outdir options targets ${IMAGES}
	@sxiv out &

outdir:
	@mkdir -p out

informe.pdf: $(INFORMESRC)
	cd informe && \
	mkdir -p build && \
	pdflatex $(LATEXFLAGS) main.tex && \
	mv build/main.pdf ../$@

clean:
	-rm ${TARGET}-c ${TARGET}-asm perf-*
	-rm ${OBJ} main.o perf.o c_core.o asm_core.o
	-rm informe.pdf
	[ -d out ] && rm -r out || true

.PHONY: all options targets debug images outdir clean perf
