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
                exit(EXIT_FAILURE);
            }
            if (nAvatars < 1 || nAvatars > AM_MAX_AVATAR) {
                fprintf(stderr, "Number of avatars must be between 1 and 10"
                                "inclusive.\n");
                exit(EXIT_FAILURE);
            }
            break;

        case 'd':
            if (sscanf(optarg, "%d", &difficulty) != 1) {
                fprintf(stderr, "-d DIFFICULTY must be a number.\n");
                exit(EXIT_FAILURE);
            }
            if (difficulty < 0 || difficulty > AM_MAX_DIFFICULTY) {
                fprintf(stderr, "Difficulty must 0-9, inclusive.\n");
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
            exit(EXIT_FAILURE);
        default:
            printUsage();
            return 0;
        }
    }

    // Note: hostnames this term are either: stowe.cs.dartmouth.edu OR carter.cs.dartmouth.edu

    //AMStartup needs to CONSTRUCT AND SEND message AM_INIT to the server.
    //In order to send a message, our client must first:
    //1. Create a socket (use the socket() function) -- which is an 'endpoint'
    //2. Connect with the server (use the connect() function)

    // Since we are using TCP client and IPv4 protocols, we will use SOCK_STREAM
    // as the type and AF_INET as the protocol family.
    //Protocol value is set to 0. (Lec 25, "protocol value can be set to 0 for this course.")

    int sockfd; //socket descriptor

    // We will use getaddrinfo() to get information about the server in order to
    // connect to it. NOT gethostbyname() ( https://piazza.com/class/i6mshngxse220l?cid=758 )
    // struct addrinfo *res = NULL; //After passin getaddrinfo(), res will contain the server information.
    // // res->ai_addr
    // // res->ai_addrlen
    // struct addrinfo hints;
    // //Set hints to the correct stuff
    // memset(&hints, 0, sizeof(hints));
    // hints.ai_family = AF_INET; // Following IPv4 Protocol.
    // hints.ai_socktype = SOCK_STREAM; // TCP
    // hints.ai_protocol = 0; //assignment instructions

    // if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    // printf("Error in creating socket... \n"); //We probably should never get an error for this. This is just a sanity check.
    // return -1;
    // }
    // int check = getaddrinfo(hostname,AM_SERVER_PORT,&hints,&res);
    // if(check != 0 /* || check != NULL */){
    //  printf("Something went wrong in acquiring server information... [ getaddrinfo() ] \n");
    //  return -1;
    // }
    // printf("%d, %d\n", res->ai_addr->sa_family, res->ai_addrlen);
    // int c_check;
    // c_check = connect(sockfd, res->ai_addr, res->ai_addrlen);
    //     if(c_check==-1){
    //         printf("Error in connecting to server: %s \n", hostname);
    //         return -1;
    //     }

    //~~~~~
    //WE WILL USE THIS IF WE CANNOT FIGURE OUT THE WARNING WHEN COMPILING THE ABOVE CODE BLOCK WITH -std=gnu99
    //IF WE USE THIS CODE
    struct hostent *hostinfo;
    if ((hostinfo = gethostbyname(hostname)) == NULL) {
        printf("Something went wrong in acquiring server information... [ gethostbyname() ] \n");
        return -1;
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "Error creating socket.\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    memcpy(&servaddr.sin_addr.s_addr,
           hostinfo->h_addr_list[0],
           hostinfo->h_length);
    servaddr.sin_port = htons(atoi(AM_SERVER_PORT));

    int c_check;
    c_check = connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    if(c_check==-1){
        printf("Error in connecting to server: %s \n", hostname);
        return -1;
    }
        //~~~~~~

        //Now that the client has connected to the server, we must:
        // 1. Construct the AM_INIT message.
        // 2. Sends the message.
        // 3. Receives the AM_INIT_OK message from server and recovers MazePort from the reply.

    AM_Message initMessage;
    memset(&initMessage, 0, sizeof(AM_Message));

    initMessage.type = htonl(AM_INIT);
    initMessage.init.nAvatars = htonl((uint32_t)nAvatars);
    initMessage.init.Difficulty = htonl((uint32_t)difficulty);

    send(sockfd, &initMessage, sizeof(AM_Message), 0);
    printf("initMessage Sent! \n");

    AM_Message recvMessage;
    memset(&recvMessage, 0, sizeof(AM_Message));

    recv(sockfd, &recvMessage, sizeof(AM_Message), 0);

    recvMessage.type = ntohl(recvMessage.type);
    if (recvMessage.type & AM_INIT_FAILED) {
        fprintf(stderr, "AM_INIT failed. Unable to create maze.\n");
        exit(EXIT_FAILURE);
    }

    printf("recvMessage received!\n");

    recvMessage.init_ok.MazePort = ntohl(recvMessage.init_ok.MazePort);
    recvMessage.init_ok.MazeWidth = ntohl(recvMessage.init_ok.MazeWidth);
    recvMessage.init_ok.MazeHeight = ntohl(recvMessage.init_ok.MazeHeight);

    char *userID = getenv("USER");
    int logFileNameLength = strlen(userID) +
                        integerLength(nAvatars) +
                        integerLength(difficulty) + 11;
    char logFileName[logFileNameLength];

    time_t curtime;
    time(&curtime);

    sprintf(logFileName, "AMAZING_%s_%d_%d", userID, nAvatars, difficulty);
    printf("LogName is: %s -- [AMAZING_userID_nAvatars_difficulty] \n", logFileName);

    FILE* logFile = fopen(logFileName, "w");
    if (!logFile) {
        fprintf(stderr, "Log file %s could not be opened.\n", logFileName);
        exit(EXIT_FAILURE);
    }

    fprintf(logFile, "%s, %" PRIu32 ", %s", userID, recvMessage.init_ok.MazePort, ctime(&curtime));
    printf("logFile created and written to! \n");

    char ipAddress[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(servaddr.sin_addr), ipAddress, INET_ADDRSTRLEN);

    Move *lastMoves = calloc(nAvatars, sizeof(Move));
    char **walls = calloc(MazeWidth, sizeof(char *));

    int w;
    for (w = 0; w < (int) MazeWidth; w++) {
        walls[w] = calloc(MazeHeight * 5, sizeof(char));
    }

    pthread_t *threads = calloc(nAvatars, sizeof(pthread_t));
    int i;
    for (i = 0; i  < nAvatars; i++) {
        AM_Args *params = malloc(sizeof(AM_Args));

        params->avatarId = i;
        params->nAvatars = nAvatars;
        params->difficulty = difficulty;
        params->mazePort = recvMessage.init_ok.MazePort;

        char *ipBuf = malloc(sizeof(char) * (strlen(ipAddress) + 1));
        strcpy(ipBuf, ipAddress);
        params->ipAddress = ipBuf;

        char *logfileBuf = malloc(sizeof(char) * (strlen(logFileName) + 1));
        strcpy(logfileBuf, logFileName);
        params->logfile = logfileBuf;

        params->walls = walls;
        params->lastMoves = lastMoves;

        pthread_create(&threads[i], NULL, new_amazing_client, (void *)params);
    }

    fclose(logFile);

    return 0;
}

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