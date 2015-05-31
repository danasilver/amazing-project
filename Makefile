CC=gcc
CFLAGS=-std=c11 -Wall -pedantic
MAZE_C_DEPS=src/AM_Startup.c src/amazing_client.c
MAZE_H_DEPS=src/amazing.h src/amazing_client.h

AM_Startup: $(MAZE_C_DEPS) $(MAZE_H_DEPS)
	$(CC) $(CFLAGS) -o $@ $(MAZE_C_DEPS)