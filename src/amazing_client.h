/*
 * amazing_client.h
 *
 */

typedef struct amazing_client_args {
    int avatarId;       // integer generated by AMStartup for each avatar
    int nAvatars;       // total number of avatars
    int difficulty;     // difficulty of tha maze
    char *ipAddress;    // IP address of the server
    int mazePort;       // MazePort returned in AM_INIT_OK
    char *logfile;      // filename to append to
} amazing_client_args;

typedef struct Move {
    XYPos pos;
    char direction;
} Move;

void new_amazing_client(amazing_client_args *args);