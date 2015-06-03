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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>                 // memset
#include <netinet/in.h>             // network functionality
#include <arpa/inet.h>
#include <unistd.h>
#include <inttypes.h>               // PRIu32

// ---------------- File includes
#include "amazing.h"
#include "amazing_client.h"
#include "walls.h"

/*
 * Main function for new_amazing_client
 *
 * Input: pointer to an AM_Args struct.
 *
 * Output: N/A
 *
 * Pseudocode:
 * 1. Argument checking or invalid pointers.
 * 2. Open the logfile
 * 3. Create a socket to connect to the server
 * 4. Send an AM_AVATAR_READY message to the server
 * 5. Continuously check for an IS_AM_ERROR and AM_MAZE_SOLVED
 *    message from the server. Exit upon receiving either, but
 *    first write to the log file for the latter.
 * 6. If the avatar detects that it is its turn to move, it checks
 *    to see if the previous avatar made a successful move. If the
 *    previous avatar ran into a wall, then the program will update
 *    the list of walls. If not, then the current avatar stores the
 *    previous avatar's attempt as its last move.
 * 7. The avatar generates a move to send to the server.
 * 8. Once the maze has been solved or an error has been detected,
 *    the program will close the log file and return NULL.
 *
 */
void *new_amazing_client(void *threadArgs) {
    AM_Args *args = (AM_Args *) threadArgs;

    if (!args || !args->ipAddress || !args->logfile ||
        !args->walls || !args->lastMoves) {
        return NULL;
    }

    printf("maze port: %d\n", (int) args->mazePort);

    // FILE *logfp = fopen(args->logfile, "a");
    // if (!logfp) {
    //     fprintf(stderr, "Error opening logfile");
    //     return NULL;
    // }

    int sockfd;
    struct sockaddr_in servaddr;
    char ***walls = args->walls;
    Move *lastMoves = args->lastMoves;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error:");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(args->ipAddress);
    servaddr.sin_port = htons(args->mazePort);

    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("Error:");
        exit(EXIT_FAILURE);
    }

    AM_Message ready_message;
    ready_message.type = htonl(AM_AVATAR_READY);
    ready_message.avatar_ready.AvatarId = htonl(args->avatarId);
    send(sockfd, &ready_message, sizeof(AM_Message), 0);

    fprintf(args->logfile, "Avatar %" PRIu32 " sent ready message to server.\n",
            args->avatarId);

    AM_Message turn;
    int i, moves = 0;

    for (;;) {
        memset(&turn, 0, sizeof(AM_Message));
        recv(sockfd, &turn, sizeof(AM_Message), 0);
        turn.type = ntohl(turn.type);

        if (IS_AM_ERROR(turn.type)) {
            fprintf(stderr, "Error receiving message from server.");
            break;
        }

        if (turn.type & AM_MAZE_SOLVED) {
            // if avatar 0 indicate maze solved and write to file

            if (args->avatarId == 0) {

                turn.maze_solved.nAvatars = ntohl(turn.maze_solved.nAvatars);
                turn.maze_solved.Difficulty = ntohl(turn.maze_solved.Difficulty);
                turn.maze_solved.nMoves = ntohl(turn.maze_solved.nMoves);
                turn.maze_solved.Hash = ntohl(turn.maze_solved.Hash);

                fprintf(args->logfile, "Solved maze with difficulty %" PRIu32
                               " with %" PRIu32
                               " avatars in %" PRIu32
                               " moves. Hash: %" PRIu32 "\n",
                        turn.maze_solved.Difficulty,
                        turn.maze_solved.nAvatars,
                        turn.maze_solved.nMoves,
                        turn.maze_solved.Hash);

                // fclose(logfp);
                break;

                printf("Maze solved!\n");
            }
            else {

                // fclose(logfp);
                break;
            }
        }

        moves++;
        turn.avatar_turn.TurnId = ntohl(turn.avatar_turn.TurnId);
        for (i = 0; i < args->nAvatars; i++) {
            turn.avatar_turn.Pos[i].x = ntohl(turn.avatar_turn.Pos[i].x);
            turn.avatar_turn.Pos[i].y = ntohl(turn.avatar_turn.Pos[i].y);
        }

        uint32_t nextTurn = turn.avatar_turn.TurnId;
        uint32_t prevTurn;
        if (turn.avatar_turn.TurnId - 1 != -1) {
            prevTurn = turn.avatar_turn.TurnId - 1;
        }
        else {
            prevTurn = args->nAvatars - 1;
        }

        if (turn.avatar_turn.TurnId == args->avatarId) {
            for (i = 0; i < args->nAvatars; i++) {
                if (moves == 1) {
                    lastMoves[i].pos.x = turn.avatar_turn.Pos[i].x;
                    lastMoves[i].pos.y = turn.avatar_turn.Pos[i].y;
                }

                fprintf(args->logfile, "Avatar %d is now at (%d, %d)\n",
                    i,
                    (int) turn.avatar_turn.Pos[i].x,
                    (int) turn.avatar_turn.Pos[i].y);
            }

            if (moves > 1) {
                if (turn.avatar_turn.Pos[prevTurn].x == lastMoves[prevTurn].pos.x &&
                    turn.avatar_turn.Pos[prevTurn].y == lastMoves[prevTurn].pos.y) {

                    addTwoSidedWall(walls, lastMoves, prevTurn, args->width, args->height);
                }
                else {
                    lastMoves[prevTurn].direction = directionDiff(lastMoves[prevTurn].pos.x,
                                                                  lastMoves[prevTurn].pos.y,
                                                                  turn.avatar_turn.Pos[prevTurn].x,
                                                                  turn.avatar_turn.Pos[prevTurn].y);
                }
            }

            draw(walls, lastMoves, turn.avatar_turn.Pos, prevTurn,
                 args->width, args->height, args->nAvatars);

            lastMoves[prevTurn].pos.x = turn.avatar_turn.Pos[prevTurn].x;
            lastMoves[prevTurn].pos.y = turn.avatar_turn.Pos[prevTurn].y;

            int nextDirection = generateMove(walls, lastMoves, turn.avatar_turn.TurnId);

            lastMoves[nextTurn].attemptedDirection = nextDirection;

            fprintf(args->logfile, "Attempting to move avatar %d direction %d\n",
                   (int) nextTurn, directionToAmazingDirection(nextDirection));

            AM_Message move_message;
            move_message.type = htonl(AM_AVATAR_MOVE);
            move_message.avatar_move.AvatarId = htonl(nextTurn);
            move_message.avatar_move.Direction = htonl(directionToAmazingDirection(nextDirection));

            send(sockfd, &move_message, sizeof(AM_Message), 0);
        }

    }

    return NULL;
}

/*
 * Function to determine the directional difference between two
 * coordinates.
 *
 * Input: two (x,y) coordinates––(x1, y1) and (x2, y2).
 *
 * Output: a char ('N', 'S', 'E', or 'W') indicating the directional
 * difference.
 *
 * Pseudocode: Compares the x1 to x2 and y1 and y2 to determine in
 * which direction the avatar moved.
 *
 */
char directionDiff(int from_x, int from_y, int to_x, int to_y) {
    if (from_x > to_x)      return 'W';
    else if (from_x < to_x) return 'E';
    else if (from_y > to_y) return 'N';
    else                    return 'S';
}

/*
 * Function to use ASCII graphics to depict the maze being
 * solved and the avatars traversing it.
 *
 * Input: pointer to a 3D walls array, pointer to a lastMoves
 * array, pointer to an array containing the new (x,y) positions
 * of the avatars, the avatar ID of the previous avatar, the width
 * of the maze, the height of the maze, and the number of avatars
 * playing in the maze.
 *
 * Output: N/A
 *
 * Pseudocode: Draw the maze as follows:
 *
 *    +---+---+---+---+---+---+---+
 *                              1 |
 *    +---+---+---+   +   +---+   +
 *                |       |   |   |
 *    +   +   +---+   +   +   +   +
 *                      0     |   |
 *    +   +   +   +   +   +   +---+
 *
 *
 *    '+' represents a corner
 *    '----' represents a horizontal wall
 *    ' | ' represents a vertical wall
 *    Numbers represent avatars
 *
 */
void draw(char ***walls, Move *lastMoves, XYPos *newPositions, uint32_t prevTurn, uint32_t width, uint32_t height, uint32_t nAvatars) {
    int i, j;
    for (i = 0; i < (int) height; i++) {
        for (j = 0; j < (int) width; j++) {
            if (string_contains('N', walls[j][i], strlen(walls[j][i]))) {
                printf("+---");
            }
            else {
                printf("+   ");
            }
        }
        printf("+\n");
        for (j = 0; j < (int) width; j++) {
            if (string_contains('W', walls[j][i], strlen(walls[j][i]))) {
                printf("|");
            }
            else printf(" ");

            int a;
            if ((a = avatarAtLocation(nAvatars, newPositions, j, i)) > -1) {
                printf(" %d ", a);
            }
            else {
                printf("   ");
            }

            if (j == (int) width - 1) {
                if (string_contains('E', walls[j][i], strlen(walls[j][i]))) {
                    printf("|\n");
                }
                else printf(" \n");
            }
        }

        if (i == (int) height - 1) {
            for (j = 0; j < (int) width; j++) {
                if (string_contains('S', walls[j][i], strlen(walls[j][i]))) {
                    printf("+---");
                }
                else {
                    printf("+   ");
                }
            }
            printf("+\n");
        }
    }
    printf("\n\n");
}

/*
 * Function to determine whether or not an avatar is present
 * at a location.
 *
 * Input: the number of avatars, a pointer to an array of
 * positions, and x and y values to represent a coordinate on
 * the maze.
 *
 * Output: if an avatar is found at the (x,y) location, then
 * its avatar ID is returned. Otherwise, the function returns
 * -1.
 *
 * Pseudocode:
 * 1. For every avatar, check to see if its position matches
 *    the (x,y) provided as a parameter. If so, return the
 *    avatar ID.
 * 2. If no avatar has proven to be at that (x,y) location,
 *    return -1.
 *
 */
int avatarAtLocation(int nAvatars, XYPos *positions, int x, int y) {
    int a;
    for (a = 0; a < nAvatars; a++) {
        if (positions[a].x == x && positions[a].y == y) {
            return a;
        }
    }

    return -1;
}

/*
 * Function to convert char directions to server moves.
 *
 * Input: a char direction
 *
 * Output: a uint32_t message to send to the server.
 *
 * Pseudocode: Depending on the direction, the function
 * returns the corresponding message.
 *
 */
uint32_t directionToAmazingDirection(char direction) {
    switch (direction) {
        case '\0': return M_NULL_MOVE;
        case 'N': return M_NORTH;
        case 'S': return M_SOUTH;
        case 'E': return M_EAST;
        case 'W': return M_WEST;
        default: return M_NULL_MOVE;
    }
}


/*
 * Function to generate an avatar's move to send to the server.
 *
 * Input: pointer to a 3D array, pointer to an array of last moves,
 * and the ID of the avatar whose turn it is to move.
 *
 * Output: a character representing the move to be made. '0'
 * indicates that the avatar should not move, while 'N', 'W', 'E',
 * 'S' indicate the four possible directions for the avatar to move.
 *
 * Pseudocode:
 * 1. Checks that the avatar is not Avatar 0, which will always
 *    stay in place. If the avatar is Avatar 0, return 0.
 * 2. If the current avatar has reached the same position as Avatar 0,
 *    then return 0 so that it does not move again.
 * 3. If the avatar has not made a successful move yet, store the last
 *    attempted move so that the avatar can move relative to that
 *    attempt.
 * 4. If the avatar has made a successful move, then move according to
 *    that position.
 * 5. Follow the left hand rule to generate a next move: for example,
 *    if the avatar is pointed north, first try to turn west. If the
 *    avatar finds a wall to the west, then try to move north. If there
 *    is a wall to the north, then attempt to move east. If all those
 *    fail, then try south.
 *
 */
int generateMove(char ***walls, Move *lastMoves, uint32_t turnId) {
    if (turnId == 0) {
        return 0;
    }

    if (lastMoves[turnId].pos.x == lastMoves[0].pos.x &&
        lastMoves[turnId].pos.y == lastMoves[0].pos.y) {
        return 0;
    }

    int i = lastMoves[turnId].pos.x;
    int j = lastMoves[turnId].pos.y;
    int dirLen = strlen(walls[i][j]);

    char lastDirection;
    if (lastMoves[turnId].direction == '\0') {

        lastDirection = lastMoves[turnId].attemptedDirection;
    }
    else {
        lastDirection = lastMoves[turnId].direction;
    }

    switch (lastDirection) {
    case 'N':
        if      (!string_contains('W', walls[i][j], dirLen)) return 'W';
        else if (!string_contains('N', walls[i][j], dirLen)) return 'N';
        else if (!string_contains('E', walls[i][j], dirLen)) return 'E';
        else                                                 return 'S';
        break;

    case 'S':
        if      (!string_contains('E', walls[i][j], dirLen)) return 'E';
        else if (!string_contains('S', walls[i][j], dirLen)) return 'S';
        else if (!string_contains('W', walls[i][j], dirLen)) return 'W';
        else                                                 return 'N';
        break;

    case 'E':
        if      (!string_contains('N', walls[i][j], dirLen)) return 'N';
        else if (!string_contains('E', walls[i][j], dirLen)) return 'E';
        else if (!string_contains('S', walls[i][j], dirLen)) return 'S';
        else                                                 return 'W';
        break;

    case 'W':
        if      (!string_contains('S', walls[i][j], dirLen)) return 'S';
        else if (!string_contains('W', walls[i][j], dirLen)) return 'W';
        else if (!string_contains('N', walls[i][j], dirLen)) return 'N';
        else                                                 return 'E';
        break;
    default:
        // no last direction
        return 'N';
    }

    return 0;
}

/*
 * Function to determine the presence of a character in a string.
 *
 * Input: a character to search for, a pointer to the string to
 * search within, and the size of the string.
 *
 * Output: 1 if the substring was found, 0 if not.
 *
 * Pseudocode:
 * 1. Loop through every character in the array.
 * 2. If the character being looked at matches the character
 *    the function is looking for, then return 1.
 * 3. If the function has looked through every character in the
 *    string without success, then return 0.
 *
 */
int string_contains(char value, char *array, int size) {
    int i;
    for (i = 0; i < size; i++) {
        if (array[i] == value) {
            return 1;
        }
    }
    return 0;
}

