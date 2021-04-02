CFLAGS= -g -Og -pthread -I include -ldl

all: bin/checker Makefile

clean:
	$(RM) -rf bin/*
	$(RM) -rf build/*

build/libkshark.o: src/libkshark.c include/libkshark.h
	$(CC) $(CFLAGS) -c src/libkshark.c -o build/libkshark.o

build/libkshark-hash.o: src/libkshark-hash.c
	$(CC) $(CFLAGS) -c src/libkshark-hash.c -o build/libkshark-hash.o

build/libkshark-plugin.o: src/libkshark-plugin.c
	$(CC) $(CFLAGS) -c src/libkshark-plugin.c -o build/libkshark-plugin.o

build/libkshark-tepdata.o: src/libkshark-tepdata.c
	$(CC) $(CFLAGS) -c src/libkshark-tepdata.c -o build/libkshark-tepdata.o

build/libkshark-collection.o: src/libkshark-collection.c
	$(CC) $(CFLAGS) -c src/libkshark-collection.c -o build/libkshark-collection.o

build/parse-filter.o: src/parse-filter.c
	$(CC) $(CFLAGS) -c src/parse-filter.c -o build/parse-filter.o

bin/checker: src/checker.c build/libkshark.o build/libkshark-hash.o build/libkshark-plugin.o build/libkshark-tepdata.o build/libkshark-collection.o build/parse-filter.o
	$(CC) $(CFLAGS) src/checker.c build/* -o bin/checker
