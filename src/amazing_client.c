/*
 * amazing.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "amazing.h"
#include "amazing_client.h"

void *new_amazing_client(AM_Args *args) {
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
        }

        int nextTurn = turn.avatar_turn.TurnId;
        int prevTurn = (turn.avatar_turn.TurnId - 1) % args->nAvatars;

        if (moves > 1) {
            if (turn.avatar_turn.Pos[prevTurn].x == lastMoves[prevTurn].pos.x &&
                turn.avatar_turn.Pos[prevTurn].y == lastMoves[prevTurn].pos.y) {

                addTwoSidedWall(walls, lastMoves, prevTurn);
            }
        }

        if (turn.avatar_turn.TurnId == args->avatarId) {
            draw(walls, lastMoves, turn.avatar_turn.Pos, prevTurn);

            lastMoves[prevTurn].pos.x = turn.avatar_turn.Pos[prevTurn].x;
            lastMoves[prevTurn].pos.y = turn.avatar_turn.Pos[prevTurn].y;

            int nextDirection = generateMove(walls, lastMoves, turn.avatar_turn.TurnId);

            // send move to server
        }

    }

    return NULL;
}