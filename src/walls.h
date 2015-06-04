/* File: walls.h - declarations for walls.c
 *
 * Authors: Dana Silver, Shivang Sethi, Ellen Li
 *
 * Date: due 06/02/2015
 */

#ifndef WALLS_H
#define WALLS_H

#include <stdint.h>             // uint32_t
#include "amazing_client.h"     // Move

/*
 * addTwoSidedWall - adds a two sided wall at an (x,y) coordinate
 * in the maze
 * @walls: pointer to the array of known walls to add to
 * @lastMoves: pointer to the array of last moves made by each
 *  avatar
 * @prevTurn: avatar ID of the last avatar to make a move
 * @width: width of the maze
 * @height: height of the maze
 *
 * Return 0 to indicate success
 *
 */
int addTwoSidedWall(char ***walls, Move *lastMoves,
                    uint32_t prevTurn, uint32_t width, uint32_t height);
/*
 * addOneSidedWall - adds a one sided wall at an (x,y) coordinate
 * in the maze
 * @walls: pointer to the array of known walls to add to
 * @x: x coordinate of place in maze to erect wall
 * @y: y coordiante of place in maze to erect wall
 * @direction: direction in which to erect wall
 * @width: width of the maze
 * @height: height of the maze
 *
 * Return 0 to indicate success, 1 otherwise
 *
 */
int addOneSidedWall(char ***walls, uint32_t x, uint32_t y,
                    char direction, uint32_t width, uint32_t height);

/*
 * addBorders - add border walls to the maze
 *
 * @walls: pointer to the array of walls
 * @width: width of the maze
 * @height: height of the maze
 */
void addBorders(char ***walls, uint32_t width, uint32_t height);

/*
 * initializeMazeInfo - inititalize an array of maze information
 * This consists of information for each cell about the walls or
 * avatar visits to the cell. It facilitates looking up the walls
 * at that location or quick constant time access whether an avatar
 * has visited a cell.
 *
 * @arrayPtr: the pointer to the array to initialize
 * @width: the width of the maze
 * @height: the height of the maze
 * @n: the number of pieces of data (array size) at each cell
 */
int initializeMazeInfo(int ****arrayPtr, uint32_t width,
                     uint32_t height, int n);

/*
 * Free the walls array and all internal arrays
 * @walls: pointer to top level walls array
 * @width: width of maze
 * @height: height of maze
 */
void freeMazeInfo(int ***arrayPtr, uint32_t width, uint32_t height);

#endif // WALLS_H