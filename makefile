CC = gcc
CFLAGS += -std=c99 -Wall -pedantic -pthread -g
INCDIR = -I src/header

TARGETS = farm generafile

.PHONY: all exec test clean
.SUFFIXES: .c .h

%.o: ./src/%.c
	$(CC) $(CFLAGS) $(INCDIR) -c -o ./src/$@ $<

all: $(TARGETS)

exec: farm generafile

farm: ./src/main.o ./src/collector.o ./src/list.o ./src/master.o ./src/tpool.o ./src/tree.o ./src/util.o
	$(CC) $(CFLAGS) -o $@ $^

generafile: generafile.c
	gcc -std=c99 -o $@ $^

test: 
	@echo "running test suite..."
	./test.sh

clean clear: 
	@echo "cleaning..."
	rm -f farm 
	rm -f generafile
	rm -f *.dat
	rm -f -r testdir
	rm -f *.txt
	rm -f ./src/*.o 
	rm -f ./farm2.sck