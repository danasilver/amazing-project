/* ========================================================================== */
/* File: amazing_client.h - header file for amazing_client.c
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
#include "amazing.h"


#ifndef AMAZING_CLIENT_H
#define AMAZING_CLIENT_H

// ------------------ Data Structures
typedef struct Move {
    XYPos pos;
    char direction;
    char attemptedDirection;
} Move;

typedef struct AM_Args {
    int avatarId;       // integer generated by AMStartup for each avatar
    int nAvatars;       // total number of avatars
    int difficulty;     // difficulty of tha maze
    char *ipAddress;    // IP address of the server
    uint32_t mazePort;  // MazePort returned in AM_INIT_OK
    uint32_t width;     // maze width
    uint32_t height;    // maze height
    char ***walls;
    int ***visits;
    Move *lastMoves;
    FILE *logfile;      // filename to append to
} AM_Args;

/*
 * new_amazing_client - main function for amazing_client.c
 * @args: AMArgs struct containing appropriate arguments
 *
 * Return void
 */
void *new_amazing_client(void *args);

/*
 * freeAMArgs - frees all arguments in the struct AM_Args that
 * were allocated
 * @args: pointer to the struct of arguments to be freed
 *
 * Return void
 */
void freeAMArgs(AM_Args *args);

/*
 * string_contains - determines presence of character in a string
 * @value: the character to be searched for
 * @array: pointer to the string to search within
 * @size: length of the string
 *
 * Return 1 if character was found, 0 otherwise
 *
 */
int string_contains(char value, char *array, int size);

/*
 * draw - draws maze with ASCII
 * @walls: pointer to the array of known walls
 * @lastMoves: pointer to the array of last moves made by avatars
 * @newPositions: pointer to the array of new (x,y) updated avatar
 *  positions
 * @prevTurn: the last avatar to make a move
 * @width: width of the maze
 * @height: height of the maze
 * @nAvatars: number of avatars playing
 *
 * Return void
 *
 */
void draw(char ***walls, Move *lastMoves, XYPos *newPositions,
          uint32_t prevTurn, uint32_t width, uint32_t height,
          uint32_t nAvatars);

/*
 * generateMove - generates a move to send to the server
 * @walls: pointer to the array of known walls
 * @lastMoves: pointer to the array of last moves made by avatars
 * @turnId: avatar ID of the current avatar about to move
 *
 * Return char representing direction in which to move
 *
 */
int generateMove(char ***walls, Move *lastMoves, uint32_t turnId);

/*
 * directionToAmazingDirection - converts a char direction to a
 * message to a move to send to the server
 * @direction: the character to convert
 *
 * Return uint32_t message
 *
 */
uint32_t directionToAmazingDirection(char direction);

/*
 * directionDiff - calculate the directional difference between
 * (x1, y1) and (x2, y2)
 * @from_x: previous x coordinate value
 * @from_y: previous y coordinate value
 * @to_x: current x coordinate value
 * @to_y: current y coordinate value
 *
 * Return char indicating directional difference
 *
 */
char directionDiff(int from_x, int from_y, int to_x, int to_y);

/*
 * avatarAtLocation - determines whether an avatar is present at
 * at maze (x,y) location
 * @nAvatars: number of avatars playing
 * @positions: pointer to an array of (x,y) positions of avatars
 * @x: x location to look at
 * @y: y location to look at
 *
 * Return -1 to indicate no avatar present, otherwise avatar ID
 * of the avatar found at that location.
 *
 */
int avatarAtLocation(int nAvatars,
                     XYPos *positions, int x, int y);

#endif // AMAZING_CLIENT_H

