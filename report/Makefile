PICFILES=$(wildcard *.pic)
PICPDFS=$(PICFILES:.pic=_pic.pdf)

PLOTFILES=$(wildcard *.plt)
PLOTPDFS=$(PLOTFILES:.plt=_plt.pdf)

TEXFILES=$(shell find . -type f -name '*.tex')

.PHONY: all

all: main.pdf

.PHONY: debug
debug:
	echo ${PICFILES}

main.pdf: $(PICPDFS) $(PLOTPDFS) $(TEXFILES)
	pdflatex main.tex
	pdflatex main.tex

%_plt.pdf: %_plt.eps
	epstopdf $^ > $@

%_plt.eps: %.plt
	gnuplot $^

%_pic.pdf: %.pic
	pic $< | groff | ps2eps -c -n -P | epstopdf --filter > $@

.PHONY: clean

clean:
	-rm main.pdf $(PICPDFS)
