CC=gcc
CFLAGS=-std=c11 -Wall -pedantic
MAZE_C_DEPS=src/AM_Startup.c src/amazing_client.c src/walls.c
MAZE_H_DEPS=src/amazing.h src/startup.h src/amazing_client.h src/walls.h

AM_Startup: $(MAZE_C_DEPS) $(MAZE_H_DEPS)
	$(CC) $(CFLAGS) -o $@ $(MAZE_C_DEPS) -lm -lpthread

clean:
	rm -f AM_Startup
