SOURCES = $(wildcard *.tex) $(wildcard imgs/*) recursos.bib
TARGET = informe.pdf
BUILDDIR = build
LATEXFLAGS = --halt-on-error -output-directory ${BUILDDIR}

${TARGET}: ${SOURCES}
	mkdir -p build && \
	pdflatex ${LATEXFLAGS} main.tex && \
	mv build/main.pdf $@

clean:
	-rm -r ${BUILDDIR} ${TARGET}
