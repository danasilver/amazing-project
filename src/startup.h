/* ========================================================================== */
/* File: amazing_client.c
 *
 * Authors: Dana Silver, Shivang Sethi, Ellen Li
 *
 * Date: due 06/02/2015
 *
 * Input: N/A
 *
 * Output: N/A
 *
 * Error Conditions:
 *
 * Special Considerations:
 */
/* ========================================================================== */
// ---------------- Open Issues

// ---------------- System includes e.g., <stdio.h>

// ---------------- File includes
#include "amazing_client.h"

#ifndef STARTUP_H
#define STARTUP_H

/*
 * integerLength - calculate the number of digits in an integer
 * @x: the integer to determine length of
 *
 * Return the number of digits in x
 */
int integerLength(int x);

/*
 * initializeLastMoves - initializes an array of last moves made
 * by n avatars.
 * @moveList: a pointer to the array to create
 * @n: the number of avatars playing the game
 *
 * Returns 0 to indicate success and 1 to indicate failure
 */
int initializeLastMoves(Move **moveList, int n);


/*
 * printUsage - prints usage information for the program
 */
void printUsage();

void freeLastMoves(Move *moveArray);

#endif // STARTUP_H
