/* File: walls.c - Utilities for working with the maze walls data structure
 *
 * Authors: Dana Silver, Shivang Sethi, Ellen Li
 *
 * Date: due 06/02/2015
 */

#include <stdlib.h>             // calloc
#include <stdio.h>              // fprintf
#include <stdint.h>             // uint32_t
#include <string.h>             // strlen
#include "walls.h"
#include "amazing_client.h"     // string_contains, Move

/*
 * Function to add a two sided wall to the list of known walls.
 *
 * Input: pointer to the walls array to add to, pointer to the array
 * of last moves, the avatar ID of the avatar that just moved, the
 * width of the maze, and the height of the maze.
 *
 * Output: 0 to indicate success.
 *
 * Pseudocode:
 * 1. Checks to determine which directions it should store.
 * 2. Calls 'addOneSidedWall' twice––once for the direction passed
 *    as a parameter, and once for its opposite direction at an
 *    adjacent maze cell.
 *
 */
int addTwoSidedWall(char ***walls, Move *lastMoves, uint32_t prevTurn,
                    uint32_t width, uint32_t height) {
    int i = lastMoves[prevTurn].pos.x;
    int j = lastMoves[prevTurn].pos.y;

    switch (lastMoves[prevTurn].attemptedDirection) {
    case 'N':
        addOneSidedWall(walls, i, j, 'N', width, height);
        addOneSidedWall(walls, i, j - 1, 'S', width, height);
        break;

    case 'S':
        addOneSidedWall(walls, i, j, 'S', width, height);
        addOneSidedWall(walls, i, j + 1, 'N', width, height);
        break;

    case 'E':
        addOneSidedWall(walls, i, j, 'E', width, height);
        addOneSidedWall(walls, i + 1, j, 'W', width, height);
        break;

    case 'W':
        addOneSidedWall(walls, i, j, 'W', width, height);
        addOneSidedWall(walls, i - 1, j, 'E', width, height);
        break;
    }

    return 0;
}

/*
 * Function to add a one-sided wall to the list of known walls.
 *
 * Input: a pointer to a 3D array of maze walls, the (x,y) location
 * of the wall, a char direction in which to erect the wall, and
 * the width and height of the maze.
 *
 * Output: 0 to indicate success, 1 otherwise.
 *
 * Pseudocode:
 * 1. Check that the wall location is within the maze bounds.
 * 2. Add the direction character to the end of the
 *    appropriate element in the walls array.
 *
 */
int addOneSidedWall(char ***walls, uint32_t x, uint32_t y, char direction,
                    uint32_t width, uint32_t height) {
    if (x < 0 || x >= width) {
        return 1;
    }
    if (y < 0 || y >= height) {
        return 1;
    }

    int dirLen = strlen(walls[x][y]);

    if (string_contains(direction, walls[x][y], dirLen)) {
        return 1;
    }

    walls[x][y][dirLen] = direction;
    walls[x][y][dirLen + 1] = '\0';

    return 0;
}

/*
 * Add border walls to the maze
 *
 * Pseudocode:
 * 1. Go across and add a north wall to every top cell and a south wall to
 *    every bottom cell.
 * 2. Go down and add a west wall to every leftmost cell and an east wall to
 *    every rightmost cell
 */
void addBorders(char ***walls, uint32_t width, uint32_t height) {
    if (walls) {
        int w, h;
        for (w = 0; w < (int) width; w++) {
            addOneSidedWall(walls, w, 0, 'N', width, height);
            addOneSidedWall(walls, w, height - 1, 'S', width, height);
        }

        for (h = 0; h < (int) height; h++) {
            addOneSidedWall(walls, 0, h, 'W', width, height);
            addOneSidedWall(walls, width - 1, h, 'E', width, height);
        }
    }
}

/*
 * Initialize a 3d array of maze information
 * Either the walls in the maze or the visits
 * Wall information is a char array and sizeof(char) == sizeof(int)
 * so either will work here with some casting
 *
 * Pseudocode:
 * 1. Allocate space for the top level array (width)
 * 2. Allocate space for each column in the maze
 * 3. Allocate n spaces at each column for the information
 */
int initializeMazeInfo(int ****arrayPtr, uint32_t width,
                     uint32_t height, int n) {

    *arrayPtr = calloc(width, sizeof(int **));
    if (!*arrayPtr) {
        fprintf(stderr, "Error: Out of memory.\n");
        return 1;
    }

    int w;
    for (w = 0; w < (int) width; w++) {
        (*arrayPtr)[w] = calloc(height, sizeof(int *));
        if (!(*arrayPtr)[w]) {
            fprintf(stderr, "Error: Out of memory.\n");
            return 1;
        }
    }

    int h;
    for (w = 0; w < (int) width; w++) {
        for (h = 0; h < (int) height; h++) {
            (*arrayPtr)[w][h] = calloc(n, sizeof(int));

            if (!(*arrayPtr)[w][h]) {
                fprintf(stderr, "Error: Out of memory.\n");
                return 1;
            }
        }
    }

    return 0;
}

/*
 * Free the walls data structure
 *
 * Pseudocode:
 * 1. Free char array cells
 * 2. Free columns of the maze
 * 3. Free the top level walls array
 */
void freeMazeInfo(int ***arrayPtr, uint32_t width, uint32_t height) {
    int w, h;
    if (arrayPtr) {
        for (w = 0; w < (int) width; w++) {
            for (h = 0; h < (int) height; h++) {
                if (arrayPtr[w][h]) {
                    free(arrayPtr[w][h]);
                    arrayPtr[w][h] = NULL;
                }
            }
            if (arrayPtr[w]) {
                free(arrayPtr[w]);
                arrayPtr[w] = NULL;
            }
        }
        free(arrayPtr);
        arrayPtr = NULL;
    }
}