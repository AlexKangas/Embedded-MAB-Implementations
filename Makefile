IDIR = src
CC=gcc
CFLAGS=-I$(IDIR) -Wall -ggdb -pedantic -g
SHELL=/bin/bash

C_OPTIONS	      = -Wall -ggdb -pedantic -g
CUNIT_LINK	    = -lcunit
C_LCOV	    	  = --coverage
C_VALGRIND  	  = valgrind --leak-check=full --show-leak-kinds=all

ODIR=src/obj
LDIR =lib

LIBS=-lm

_DEPS = swucb-fixed.h swucb-float.h fixed-point.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))


_OBJ = main.o swucb-fixed.o swucb-float.o fixed-point.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: src/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

main: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

run: main
	./main  >> results/result.txt

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ main results/result.txt
