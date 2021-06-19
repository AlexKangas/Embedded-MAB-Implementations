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
MAKEFLAGS += -j$(LOOPS)
WINDOW = 50
TIME = 51
run: main
	./main $(TIME) >> results/$(TIME).txt

rundata: main
	./main >> results/dataset/dataset.txt

runmanydata: main
	#for ((i=1; i <= ${LOOPS}; ++i)) do ./main >> results/$(WINDOW).txt & ; done
	for ((i=1; i <= ${LOOPS}; ++i)) do $(MAKE) rundata; done

runmany: main
	#for ((i=1; i <= ${LOOPS}; ++i)) do ./main >> results/$(WINDOW).txt & ; done
	for ((i=1; i <= ${LOOPS}; ++i)) do $(MAKE) run; done

runmanytime: main
	#for ((i=1; i <= ${LOOPS}; ++i)) do ./main >> results/$(TIME).txt & ; done
	for ((i=1; i <= ${LOOPS}; ++i)) do $(MAKE) run; done
.PHONY: loop

prog:
        # Commands for making prog

parse: results/40.txt
	sed 's/|/\n/g' results/40.txt | sed 's/,/ /g' | sort -V | awk 'NR>1 { k = $1 OFS $2 OFS $3 OFS $4 }{ arr[k] += $5; count[k]++ }END{ for (i in arr){ print i, arr[i]/count[i]}}' | sort -V >> res50_complete.txt

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ main
