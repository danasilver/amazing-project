/*
 * amazing.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netint/in.h>
#include <arpa/inet.h>
#include "amazing.h"
#include "amazing_client.h"

void *new_amazing_client(amazing_client_args *args) {
    if (!args || !args->ipAddress || !args->logfile) {
        return NULL;
    }

    int sockfd;
    struct sockaddr_in servaddr;

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

    AM_Message turnMessage;
    int i, moves = 0;

    for (;;) {
        memset(&turnMessage, 0, sizeof(AM_Message));
        recv(sockfd, &turnMessage, sizeof(AM_Message), 0);
        turnMessage.type = ntohl(turnMessage.type);

        if (IS_AM_ERROR(turnMessage.type)) {
            fprintf(stderr, "Error receiving message from server.");
            break;
        }

        if (turnMessage.type & AM_MAZE_SOLVED) {
            // if avatar 0 indicate maze solved and write to file

            printf("Maze solved.");
            break;
        }

        moves++;
        turnMessage.TurnId = ntohl(turnMessage.TurnId);
        for (i = 0; i < args->nAvatars; i++) {
            turnMessage.Pos[i].x = ntohl(turnMessage.Pos[i].x);
            turnMessage.Pos[i].y = ntohl(turnMessage.Pos[i].y);
        }

        int nextTurn = turnMessage.TurnId;
        int prevTurn = (turnMessage.TurnId - 1) % args->nAvatars;

        if (moves > 1) {
            if (turnMessage.Pos[prevTurn].x == lastMoves[prevTurn].pos.x &&
                turnMessage.Pos[prevTurn].y == lastMoves[prevTurn].pos.y) {

                addTwoSidedWall(walls, lastMoves, prevTurn);
            }
        }

        if (turnMessage.TurnId == args->avatarId) {
            draw(walls, lastMoves, turnMessage.Pos, prevTurn);

            lastMoves[prevTurn].pos.x = turnMessage.Pos[prevTurn].x;
            lastMoves[prevTurn].pos.y = turnMessage.Pos[prevTurn].y;

            int nextDirection = generateMove(walls, lastMoves, turnMessage.TurnId);

            // send move to server
        }

    }

    return NULL;
}