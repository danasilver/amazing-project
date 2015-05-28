#include <stdio.h>
#include <stdlib.h>
#include <string.h> //strlen, strcpy, memset
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h> //getopt, getlogin
#include <getopt.h>
#include <pwd.h> //getpwduid
#include <math.h>
#include <time.h> // time() and ctime()
#include "amazing.h"



int main(int argc, char* argv[]){

	int nAvatars;
	int difficulty;
	char* hostname;
	int option;
	while((option = getopt(argc, argv, "n:d:h:")) != -1){
		switch(option){
			case 'n' :
						if(sscanf(optarg, "%d", &nAvatars) == 0){
							printf("nAvatars is not a number... \n");
							return -1;
						}
						if(nAvatars <1 || nAvatars > AM_MAX_AVATAR){ //AM_MAX_AVATAR == 10. (amazing.h)
							printf("Number of avatars must be between 1 and 10 inclusive \n");
							return -1;
						}
						break;
			case 'd' :
						if(sscanf(optarg, "%d", &difficulty) == 0){
							printf("Difficulty is not a number...\n");
							return -1;
						}
						if(difficulty < 0 || difficulty > AM_MAX_DIFFICULTY){
							printf("Difficulty must 0-9, inclusive. \n");
							return -1;
						}
						break;
			case 'h' :
						hostname=calloc(strlen(optarg)+1, sizeof(char));
						strcpy(hostname, optarg);
						break;
			default:
				printf("[Usage] -n nAvatars -d Difficulty -h Hostname \n");
				return -1;
		}
	}

	//AMStartup has now validated the arguments: -n nAvatars, -d Difficulty and -h Hostname
	// Note: hostnames this term are either: stowe.cs.dartmouth.edu OR carter.cs.dartmouthe.edu

	//AMStartup needs to CONSTRUCT AND SEND message AM_INIT to the server.
	//In order to send a message, our client must first:
	//1. Create a socket (use the socket() function) -- which is an 'endpoint'
	//2. Connect with the server (use the connect() function)

	//Since we are using TCP client and IPv4 protocols, we will use SOCK_STREAM as the type and AF_INET as the protocol family. 
	//Protocol value is set to 0. (Lec 25, "protocol value can be set to 0 for this course.")
	
	int sockfd; //socket descriptor

	//We will use getaddrinfo() to get information about the server in order to connect to it. NOT gethostbyname() ( https://piazza.com/class/i6mshngxse220l?cid=758 )
	struct addrinfo *res = NULL; //After passin getaddrinfo(), res will contain the server information.
	// res->ai_addr
	// res->ai_addrlen
	struct addrinfo hints;
	//Set hints to the correct stuff
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; // Following IPv4 Protocol.
	hints.ai_socktype = SOCK_STREAM; // TCP
	hints.ai_protocol = 0; //assignment instructions

	int check = getaddrinfo(hostname,AM_SERVER_PORT,&hints,&res);
	if(check != 0 /* || check != NULL */){
		printf("Something went wrong in acquiring server information... [ getaddrinfo() ] \n");
		return -1;
	}
	
	if((sockfd = socket(res->ai_family, res->ai_socktype, 0)) < 0){
	printf("Error in creating socket... \n"); //We probably should never get an error for this. This is just a sanity check.
	return -1;
	}

	printf("%d, %d\n", res->ai_addr->sa_family, res->ai_addrlen);
	int c_check;
	c_check = connect(sockfd, res->ai_addr, res->ai_addrlen);
		if(c_check==-1){
		printf("Error in connecting to server: %s \n", hostname);
		return -1;
		}

	//Now that the client has connected to the server, we must:
	// 1. Construct the AM_INIT message.
	// 2. Sends the message.
	// 3. Receives the AM_INIT_OK message from server and recovers MazePort from the reply.

	AM_Message initMessage;
	memset(&initMessage, 0, sizeof(AM_Message));

	initMessage.type = htonl(AM_INIT);
	initMessage.init.nAvatars = htonl((uint32_t)nAvatars); // we can simply typecast an int to a uint32_t https://piazza.com/class/i6mshngxse220l?cid=783
	initMessage.init.Difficulty = htonl((uint32_t)difficulty);

	//We convery nAvatars and difficulty to uint32_t and then convert them to Network Byte Order using htonl(uint32_t num)

	send(sockfd, &initMessage, sizeof(AM_Message), 0); //we set the flag to 0 according to assgnment.
	printf("initMessage Sent! \n");

	AM_Message recvMessage;
	memset(&recvMessage, 0, sizeof(AM_Message));
	
	recv(sockfd, &recvMessage, sizeof(AM_Message),0);

	recvMessage.type = ntohl(recvMessage.type); //Get the 'type' of message.
	if(recvMessage.type == AM_INIT_FAILED){ //superfluous check... Im not sure if " != AM_INIT_OK" is a valid logical comparison.
		printf("Received message AM_INIT_FAILED. AM_INIT has not been successfully processed. \n");
		return -1;
	}
	printf("recvMessage received! \n");
	//we need to convert the Network Byte Order data back to Host Byte Order
	recvMessage.init_ok.MazePort = ntohl(recvMessage.init_ok.MazePort);
	recvMessage.init_ok.MazeWidth = ntohl(recvMessage.init_ok.MazeWidth);
	recvMessage.init_ok.MazeHeight = ntohl(recvMessage.init_ok.MazeHeight);

	//Creating the logFile
	// char username[256];

	// getlogin_r(username, 256); // http://linux.die.net/man/3/getlogin_r

	// struct passwd pwd;
	// struct passwd* result;
	// char buf[16384];
	// getpwnam_r(username, &pwd, buf, 16384, &result);
	// if(result == NULL){
	// 	printf("getpwnam_r went wrong \n");
	// 	return -1;
	// }

	// int userID = pwd.pw_uid; // http://linux.die.net/man/3/getpwnam
	char* userID = getenv("USER");

	char logFileName[200];
	sprintf(logFileName, "AMAZING_%s_%d_%d", userID, nAvatars, difficulty);
	printf("LogName is : %s -- [AMAZING_userID_nAvatars_difficulty \n", logFileName);
	FILE* logFile = fopen(logFileName, "w");
	if(logFile == NULL){
		printf("File -->%s<--could not be opened \n", logFileName);
		return -1;
	}

	time_t curtime;
	time(&curtime);

	fprintf(logFile, "%s, %d, %s", userID, (int)recvMessage.init_ok.MazePort, ctime(&curtime));
	printf("logFile created and written to! \n");
	fclose(logFile);
	return 1;
}