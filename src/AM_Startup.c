#include <stdio.h>
#include <stdlib.h>
#include <string.h> //strlen, strcpy, memset
#include <ctype.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h> //getopt, getlogin
#include <getopt.h>
#include <pwd.h> //getpwduid
#include <math.h>
#include <time.h> // time() and ctime()
#include <pthread.h>
#include <arpa/inet.h>
#include "amazing.h"
#include "startup.h"
#include "amazing_client.h"
#include "walls.h"

int main(int argc, char *argv[]){
    int nAvatars;
    int difficulty;
    char* hostname;
    int option;

    int argindex;
    for (argindex = 0; argindex < argc; argindex++) {
        if (strncmp(argv[argindex], "--help", 6) == 0) {
            printUsage();
            exit(EXIT_SUCCESS);
        }
    }

    if (argc != 7) {
        printUsage();
        exit(EXIT_FAILURE);
    }

    while((option = getopt(argc, argv, "n:d:h:")) != -1){
        switch(option){
        case 'n':
            if (sscanf(optarg, "%d", &nAvatars) != 1) {
                fprintf(stderr, "nAvatars is not a number... \n");
                if (hostname) free(hostname);
                exit(EXIT_FAILURE);
            }
            if (nAvatars < 1 || nAvatars > AM_MAX_AVATAR) {
                fprintf(stderr, "Number of avatars must be between 1 and 10 "
                                "inclusive.\n");
                if (hostname) free(hostname);
                exit(EXIT_FAILURE);
            }
            break;

        case 'd':
            if (sscanf(optarg, "%d", &difficulty) != 1) {
                fprintf(stderr, "-d DIFFICULTY must be a number.\n");
                if (hostname) free(hostname);
                exit(EXIT_FAILURE);
            }
            if (difficulty < 0 || difficulty > AM_MAX_DIFFICULTY) {
                fprintf(stderr, "Difficulty must be 0-9, inclusive.\n");
                if (hostname) free(hostname);
                exit(EXIT_FAILURE);
            }
            break;

        case 'h':
            hostname = calloc(strlen(optarg) + 1, sizeof(char));
            strcpy(hostname, optarg);
            break;

        case '?':
            if (optopt == 'n' || optopt == 'd' || optopt == 'h') {
                fprintf(stderr, "option -%c requires an argument.\n", optopt);
            }
            else if (isprint(optopt)) {
                fprintf(stderr, "Unknown option -%c.\n", optopt);
            }
            else {
                fprintf(stderr, "Unknown option character \\x%x.", optopt);
            }
            if (hostname) free(hostname);
            exit(EXIT_FAILURE);
        default:
            printUsage();
            if (hostname) free(hostname);
            return 0;
        }
    }

    // Resolve hostname
    struct hostent *hostinfo;
    if ((hostinfo = gethostbyname(hostname)) == NULL) {
        fprintf(stderr, "Unable to resolve hostname.\n");
        free(hostname);
        exit(EXIT_FAILURE);
    }

    // Create socket with socket file descriptor
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "Error creating socket.\n");
        free(hostname);
        exit(EXIT_FAILURE);
    }

    // Connect to the socket
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    memcpy(&servaddr.sin_addr.s_addr,
           hostinfo->h_addr_list[0],
           hostinfo->h_length);
    servaddr.sin_port = htons(atoi(AM_SERVER_PORT));

    int c_check;
    c_check = connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    if(c_check == -1){
        fprintf(stderr, "Error in connecting to server: %s\n", hostname);
        close(sockfd);
        free(hostname);
        exit(EXIT_FAILURE);
    }

    free(hostname);

    // Send the init message
    AM_Message initMessage;
    memset(&initMessage, 0, sizeof(AM_Message));

    initMessage.type = htonl(AM_INIT);
    initMessage.init.nAvatars = htonl((uint32_t)nAvatars);
    initMessage.init.Difficulty = htonl((uint32_t)difficulty);

    send(sockfd, &initMessage, sizeof(AM_Message), 0);

    AM_Message recvMessage;
    memset(&recvMessage, 0, sizeof(AM_Message));

    recv(sockfd, &recvMessage, sizeof(AM_Message), 0);

    recvMessage.type = ntohl(recvMessage.type);
    if (recvMessage.type & AM_INIT_FAILED) {
        fprintf(stderr, "AM_INIT failed. Unable to create maze.\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    recvMessage.init_ok.MazePort = ntohl(recvMessage.init_ok.MazePort);
    recvMessage.init_ok.MazeWidth = ntohl(recvMessage.init_ok.MazeWidth);
    recvMessage.init_ok.MazeHeight = ntohl(recvMessage.init_ok.MazeHeight);

    // Get USERID for log
    char *userID = getenv("USER");

    // Get time for log
    time_t curtime;
    time(&curtime);

    int logFileNameLength = strlen(userID) +
                        integerLength(nAvatars) +
                        integerLength(difficulty) + 11;
    char logFileName[logFileNameLength];

    sprintf(logFileName, "AMAZING_%s_%d_%d", userID, nAvatars, difficulty);

    FILE* logFile = fopen(logFileName, "w");
    if (!logFile) {
        fprintf(stderr, "Log file %s could not be opened.\n", logFileName);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    fprintf(logFile, "%s, %" PRIu32 ", %s",
            userID, recvMessage.init_ok.MazePort, ctime(&curtime));

    char ipAddress[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(servaddr.sin_addr), ipAddress, INET_ADDRSTRLEN);

    close(sockfd);

    Move *lastMoves;
    if (initializeLastMoves(&lastMoves, nAvatars)) {
        fclose(logFile);
        exit(EXIT_FAILURE);
    }

    int ***intWalls;
    if (initializeMazeInfo(&intWalls, recvMessage.init_ok.MazeWidth,
        recvMessage.init_ok.MazeHeight, 5)) {
        fclose(logFile);
        freeLastMoves(lastMoves);
        exit(EXIT_FAILURE);
    }

    char ***walls = (char ***)intWalls;

    int ***visits;
    if (initializeMazeInfo(&visits, recvMessage.init_ok.MazeWidth,
        recvMessage.init_ok.MazeHeight, nAvatars)) {
        fclose(logFile);
        freeLastMoves(lastMoves);
        freeMazeInfo((int ***)walls,
                     recvMessage.init_ok.MazeWidth,
                     recvMessage.init_ok.MazeHeight);
    }

    addBorders(walls, recvMessage.init_ok.MazeWidth,
                      recvMessage.init_ok.MazeHeight);

    pthread_t *threads = calloc(nAvatars, sizeof(pthread_t));
    if (!threads) {
        fprintf(stderr, "Error: Out of memory.\n");
        fclose(logFile);
        freeLastMoves(lastMoves);
        freeMazeInfo((int ***)walls,
                     recvMessage.init_ok.MazeWidth,
                     recvMessage.init_ok.MazeHeight);
        freeMazeInfo(visits,
                     recvMessage.init_ok.MazeWidth,
                     recvMessage.init_ok.MazeHeight);
    }
    int i;
    for (i = 0; i  < nAvatars; i++) {
        AM_Args *params = malloc(sizeof(AM_Args));
        if (!params) {
            fprintf(stderr, "Error: Out of memory.\n");
            break;
        }

        params->avatarId = i;
        params->nAvatars = nAvatars;
        params->difficulty = difficulty;
        params->mazePort = recvMessage.init_ok.MazePort;

        char *ipBuf = malloc(sizeof(char) * (strlen(ipAddress) + 1));
        if (!ipBuf) {
            fprintf(stderr, "Error: Out of memory.\n");
            break;
        }
        strcpy(ipBuf, ipAddress);
        params->ipAddress = ipBuf;

        params->logfile = logFile;

        params->width = recvMessage.init_ok.MazeWidth;
        params->height = recvMessage.init_ok.MazeHeight;

        params->walls = walls;
        params->lastMoves = lastMoves;
        params->visits = visits;

        int s;
        s = pthread_create(&threads[i], NULL, new_amazing_client, (void *)params);
        if (s != 0) {
            fprintf(stderr, "Error creating threads.\n");
            break;
        }
    }

    // Wait for avatars to return
    for (i = 0; i < nAvatars; i++) {
        pthread_join(threads[i], NULL);
    }

    // Free memory
    fclose(logFile);
    freeMazeInfo((int ***)walls,
              recvMessage.init_ok.MazeWidth,
              recvMessage.init_ok.MazeHeight);
    freeMazeInfo(visits,
              recvMessage.init_ok.MazeWidth,
              recvMessage.init_ok.MazeHeight);
    freeLastMoves(lastMoves);
    free(threads);

    return 0;
}

/*
 * Allocate space for the array of last moves
 *
 * Pseudocode:
 * 1. Check if pointer is null
 * 2. Calloc space for n elements, each with a Move struct
 */
int initializeLastMoves(Move **moveArray, int n) {
    *moveArray = calloc(n, sizeof(Move));
    if (!*moveArray) {
        fprintf(stderr, "Out of memory!\n");
        return 1;
    }

    return 0;
}

/*
 * Free the lastmoves array.
 *
 * Pseudocode:
 * 1. Check if the array pointer is NULL
 * 2. If not, free it and set NULL
 */
void freeLastMoves(Move *moveArray) {
    if (moveArray) {
        free(moveArray);
        moveArray = NULL;
    }
}

/*
 * Return the number of digits in an integer.
 * http://stackoverflow.com/questions/6655754
 *
 * Pseudocode:
 * 1. Return 1 if x is 0 since can't take log(0)
 * 2. Else return floor(log_10(|x|)) + 1
 */
int integerLength(int x) {
    if (x == 0) return 1;

    return (int) floor(log10(abs(x))) + 1;
}

void printUsage() {
    printf("[Usage] ./AM_Startup -n nAvatars -d Difficulty -h Hostname.\n"
           "Solve a multiplayer maze by having avatars find each other.\n\n"
           "  -n nAvatars    The number of avatars to play with\n"
           "  -d Difficulty  Difficulty determines the size and complexity of\n"
           "                 the maze\n"
           "  -h Hostname    Hostname to reach the maze server\n"
           "  --help         Display this help and exit\n");
}