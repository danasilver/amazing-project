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

// ---------------- File includes
#include "amazing.h"
#include "amazing_client.h"


void *new_amazing_client(void *threadArgs) {
    AM_Args *args = (AM_Args *) threadArgs;

    if (!args || !args->ipAddress || !args->logfile ||
        !args->walls || !args->lastMoves) {
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

            printf("Maze solved.");
            break;
        }

        moves++;
        turn.avatar_turn.TurnId = ntohl(turn.avatar_turn.TurnId);
        for (i = 0; i < args->nAvatars; i++) {
            turn.avatar_turn.Pos[i].x = ntohl(turn.avatar_turn.Pos[i].x);
            turn.avatar_turn.Pos[i].y = ntohl(turn.avatar_turn.Pos[i].y);

            printf("id: %d, x: %lu, y: %lu\n",
                    i,
                    (unsigned long) turn.avatar_turn.Pos[i].x,
                    (unsigned long) turn.avatar_turn.Pos[i].y);
        }

        uint32_t nextTurn = turn.avatar_turn.TurnId;
        uint32_t prevTurn = (turn.avatar_turn.TurnId - 1) % args->nAvatars;

        if (moves > 1) {
            if (turn.avatar_turn.Pos[prevTurn].x == lastMoves[prevTurn].pos.x &&
                turn.avatar_turn.Pos[prevTurn].y == lastMoves[prevTurn].pos.y) {

                addTwoSidedWall(walls, lastMoves, prevTurn, args->width, args->height);
            }
        }

        if (turn.avatar_turn.TurnId == args->avatarId) {
            draw(walls, lastMoves, turn.avatar_turn.Pos, prevTurn);

            lastMoves[prevTurn].pos.x = turn.avatar_turn.Pos[prevTurn].x;
            lastMoves[prevTurn].pos.y = turn.avatar_turn.Pos[prevTurn].y;

            int nextDirection = generateMove(walls, lastMoves, turn.avatar_turn.TurnId);

            lastMoves[nextTurn].direction = nextDirection;

            // send move to server
        }

    }

    return NULL;
}


int addTwoSidedWall(char ***walls, Move *lastMoves, uint32_t prevTurn, uint32_t width, uint32_t height) {
    int i = lastMoves[prevTurn].pos.x;
    int j = lastMoves[prevTurn].pos.y;

    switch (lastMoves[prevTurn].direction) {
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
    int dirLen = strlen(walls[x][y]);

    if (x < width || x > width) {
        return 1;
    }
    if (y < height || y > height) {
        return 1;
    }
    if (string_contains(direction, walls[x][y], dirLen)) {
        return 1;
    }

    walls[x][y][dirLen] = direction;
    walls[x][y][dirLen + 1] = '\0';

    return 0;
}

void draw(char ***walls, Move *lastMoves, XYPos *newPositions, uint32_t prevTurn) {

}

int generateMove(char ***walls, Move *lastMoves, uint32_t turnId) {
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

int generateMove(char **walls, Move *lastMoves, uint32_t turn.avatar_turn.TurnId) {
}
