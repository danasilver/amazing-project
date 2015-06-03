/* ========================================================================== */
/* File: amazing_client.c
 *
 * Authors: Dana Silver, Ellen Li
 *
 * Date: due 06/02/2015
 *
 * Input:
 *
 * Output:
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
//#include <sys/types.h>              // pthread
//#include <sys/socket.h>             // socket functionality
#include <netinet/in.h>             // network functionality
#include <arpa/inet.h>
#include <unistd.h>
#include <inttypes.h>               // PRIu32

// ---------------- File includes
#include "amazing.h"
#include "amazing_client.h"


void *new_amazing_client(void *threadArgs) {
    AM_Args *args = (AM_Args *) threadArgs;

    if (!args || !args->ipAddress || !args->logfile ||
        !args->walls || !args->lastMoves) {
        return NULL;
    }

    printf("maze port: %d\n", (int) args->mazePort);

    FILE *logfp = fopen(args->logfile, "a");
    if (!logfp) {
        fprintf(stderr, "Error opening logfile");
        return NULL;
    }

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

    fprintf(logfp, "Avatar %" PRIu32 " sent ready message to server.\n",
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
                printf("Maze solved!\n");

                turn.maze_solved.nAvatars = ntohl(turn.maze_solved.nAvatars);
                turn.maze_solved.Difficulty = ntohl(turn.maze_solved.Difficulty);
                turn.maze_solved.nMoves = ntohl(turn.maze_solved.nMoves);
                turn.maze_solved.Hash = ntohl(turn.maze_solved.Hash);

                fprintf(logfp, "Solved maze with difficulty %" PRIu32
                               " with %" PRIu32
                               " avatars in %" PRIu32
                               " moves. Hash: %" PRIu32 "\n",
                        turn.maze_solved.Difficulty,
                        turn.maze_solved.nAvatars,
                        turn.maze_solved.nMoves,
                        turn.maze_solved.Hash);
            }

            break;
        }

        moves++;
        turn.avatar_turn.TurnId = ntohl(turn.avatar_turn.TurnId);
        for (i = 0; i < args->nAvatars; i++) {
            turn.avatar_turn.Pos[i].x = ntohl(turn.avatar_turn.Pos[i].x);
            turn.avatar_turn.Pos[i].y = ntohl(turn.avatar_turn.Pos[i].y);

            fprintf(logfp, "Avatar %d is now at (%d, %d)\n",
                    i,
                    (int) turn.avatar_turn.Pos[i].x,
                    (int) turn.avatar_turn.Pos[i].y);
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
            if (moves == 1) {
                for (i = 0; i < args->nAvatars; i++) {
                    lastMoves[i].pos.x = turn.avatar_turn.Pos[i].x;
                    lastMoves[i].pos.y = turn.avatar_turn.Pos[i].y;
                }
            }
            else if (moves > 1) {
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

            fprintf(logfp, "Attempting to move avatar %d direction %d\n",
                   (int) nextTurn, directionToAmazingDirection(nextDirection));

            AM_Message move_message;
            move_message.type = htonl(AM_AVATAR_MOVE);
            move_message.avatar_move.AvatarId = htonl(nextTurn);
            move_message.avatar_move.Direction = htonl(directionToAmazingDirection(nextDirection));

            send(sockfd, &move_message, sizeof(AM_Message), 0);
        }

    }

    fclose(logfp);

    return NULL;
}

char directionDiff(int from_x, int from_y, int to_x, int to_y) {
    if (from_x > to_x)      return 'W';
    else if (from_x < to_x) return 'E';
    else if (from_y > to_y) return 'N';
    else                    return 'S';
}


int addTwoSidedWall(char ***walls, Move *lastMoves, uint32_t prevTurn, uint32_t width, uint32_t height) {
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

int avatarAtLocation(int nAvatars, XYPos *positions, int x, int y) {
    int a;
    for (a = 0; a < nAvatars; a++) {
        if (positions[a].x == x && positions[a].y == y) {
            return a;
        }
    }

    return -1;
}

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


int string_contains(char value, char *array, int size) {
    int i;
    for (i = 0; i < size; i++) {
        if (array[i] == value) {
            return 1;
        }
    }
    return 0;
}

