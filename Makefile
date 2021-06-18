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


LOOPS = 40
run: main
	./main >> results/$(LOOPS).txt

runmany: main
	for ((i=1; i <= ${LOOPS}; ++i)) do make run; done
.PHONY: loop

prog:
        # Commands for making prog


.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ main
