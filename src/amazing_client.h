/*
 * amazing_client.h
 *
 */

#include "amazing.h"

typedef struct Move {
    XYPos pos;
    char direction;
} Move;

typedef struct AM_Args {
    int avatarId;       // integer generated by AMStartup for each avatar
    int nAvatars;       // total number of avatars
    int difficulty;     // difficulty of tha maze
    char *ipAddress;    // IP address of the server
    uint32_t mazePort;  // MazePort returned in AM_INIT_OK
    uint32_t width;     // maze width
    uint32_t height;    // maze height
    char **walls;
    Move *lastMoves;
    char *logfile;      // filename to append to
} AM_Args;

void *new_amazing_client(AM_Args *args);

void freeAMArgs(AM_Args *args);

int addTwoSidedWall(char **walls, Move *lastMoves, uint32_t prevTurn, uint32_t width, uint32_t height);

int addOneSidedWall(char **walls, uint32_t x, uint32_t y, char direction, uint32_t width, uint32_t height);

int string_contains(char value, char *array, int size)
