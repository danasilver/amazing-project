#ifndef STARTUP_H
#define STARTUP_H

#include "amazing_client.h"

/*
 * integerLength - calculate the number of digits in an integer
 * @x: the integer to determine length of
 *
 * Return the number of digits in x
 */
int integerLength(int x);

int initializeLastMoves(Move **moveList, int n);

void printUsage();

#endif // STARTUP_H
