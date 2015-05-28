CC=gcc
CFLAGS=-std=c11 -Wall -pedantic
MAZE_C_DEPS=src/AM_Startup.c
MAZE_H_DEPS=src/amazing.h

AM_Startup: $(MAZE_C_DEPS) $(MAZE_H_DEPS)
	$(CC) $(CFLAGS) -o $@ $(MAZE_C_DEPS)