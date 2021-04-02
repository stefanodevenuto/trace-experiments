CFLAGS= -g -Og -pthread -I include -ldl

all: bin/checker Makefile

clean:
	$(RM) -rf bin/*
	$(RM) -rf build/*

build/sorting.o: src/sorting.c include/sorting.h
	$(CC) $(CFLAGS) -c src/sorting.c -o build/sorting.o

build/dyn_array.o: src/dyn_array.c include/dyn_array.h
	$(CC) $(CFLAGS) -c src/dyn_array.c -o build/dyn_array.o

bin/checker: src/checker.c build/sorting.o build/dyn_array.o build/* include/*
	$(CC) $(CFLAGS) src/checker.c build/* lib/* -o bin/checker
