CFLAGS= -g -Og -pthread -I include -ldl

all: bin/checker Makefile

clean:
	$(RM) -rf bin/*

bin/checker: src/checker.c
	$(CC) $(CFLAGS) src/checker.c lib/* -o bin/checker
