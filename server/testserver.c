/*
 * testserver.c
 *
 *  Created on: 2012.05.26.
 *      Author: Péter Attila Némethy
 *
 *      I'll be using glib strings most of the time
 */

#include <stdio.h>      // for input-output
#include <winsock.h>    // for socket()
#include <stdlib.h>     // for exit()
#include <pthread.h>	// for threading
#include <unistd.h>		// for access()
#include <glib.h>		// for gchar, ...
#include <glib/gprintf.h>	// for g_printf()
/*
 * DEFINED VALUES
 */
#define MAXPENDING 5    // Maximum outstanding connection requests
#define RCVBUFSIZE 32	// Maximum transferable bytes
#define LN 1			// Number of locations -> game mechanics
/*
 * TYPE DEFINITIONS
 */
typedef struct birth_date {     //birth date -> need to be separated for age
	int year;
	int month;
	int day;
} BORN;

typedef struct character {     //character data -> game mechanics
	BORN born;
	char name[31], House[31], Gender;
	int height, weight, age, rep, str, dex, inte;
	int spd, vit, wis, sta, cha, hly;
	int kl[LN], pt[20], tech[20], abil[20], talent[20];
} caracter;

/*
 * DECLARATIONS
 */
gchar sent[32];     //reply to client
caracter used[500];     //not in use yet PS:Need a better name
int servSock;     // Socket descriptor for server
int clntSock;     // Socket descriptor for client
struct sockaddr_in ClntAddr;     // Client address
int logged = -1;     //connected clients not in use yet

/*
 * FUNCTIONS
 */
int random(int t, int i) {     // An attempt to make sure no number is generated twice in a row PS: array used because its called from multiple places
	static int l[10];
	int r;
	do {
		r = (rand() % t);
	} while (r == l[i]);
	l[i] = r;
	return r;
}

void ans() {     // Trash only to see if the connection works
	int r;
	r = random(6, 1);
	switch (r) {
		case 0: {
			strcpy(sent, "anyád");
			break;
		}
		case 1: {
			strcpy(sent, "gennyláda");
			break;
		}
		case 2: {
			strcpy(sent, "apád");
			break;
		}
		case 3: {
			strcpy(sent, "buzi");
			break;
		}
		case 4: {
			strcpy(sent, "faszom");
			break;
		}
		case 5: {
			strcpy(sent, "geci");
			break;
		}
	}
}

void pic() {     // A way to show a picture chosen by the server will be useful later
	int r;
	gchar tmp[10];
	r = random(8, 2) + 1;     //generate a number between 1 and 8
	itoa(r, tmp, 10);
	strcpy(sent, "pic/");     //crate the path PS: I'm waiting for a better way
	strcat(sent, tmp);
	strcat(sent, ".jpg");     //it ends here
	//printf("%s\n", sent);
	return;
}

void DieWithError(char *errorMessage) {     //Error handling function
	perror(errorMessage);     // print error code
	exit(1);     // shut down program
}

void dc(int clntSocket) {     // Function to disconnect client
	printf("Disconnecting client %s\n", inet_ntoa(ClntAddr.sin_addr));     //print the IP of the one disconnected
	closesocket(clntSocket);     // close connection
	return;
}

void reg(int clntSocket) {     //A registration function -> game mechanics
	FILE *p;
	gchar path[36], name[31], pass[31];

	g_stpcpy(path, "user/");     //prepare the path
	recv(clntSocket, name, RCVBUFSIZE, 0);     //receive name
	g_strlcat(path, name, 36);     //make the path
	recv(clntSocket, pass, RCVBUFSIZE, 0);     //receive password
	//g_printf("%s", path);

	if (!access(path, F_OK)) {     //check if file is present -> as in already registered or not
		//file was present
		send(clntSocket, "FALSE", 6, 0);		//send the failed signal
		return;     //end function
	}
	//file was NOT present
	send(clntSocket, "TRUE", 5, 0);     //send the OK signal
	p = fopen(path, "w");     //make the user's file
	fprintf(
			p,     //if we made it we should write something to it
			"Name: ;Gender: ; Born: ;height;weight: ;House: ;STR ;DEX ;INT ;VIT ;WIS ;STA ;SPD ;CHA ;HLY ;"
					" Talents: ; Abilities: ; Techniques: ; Personality Traits: ; Reputation: ; Known Locations: ;");
	fclose(p);     //close the file
	return;     //end function
}

int login(int clntSocket, int ID) {
	gchar echoBuffer[RCVBUFSIZE], pass[RCVBUFSIZE], path[36];     //data receiving strings
	FILE *p;
	//int recvMsgSize;     //Size of received message

	for (;;) {
		recv(clntSocket, echoBuffer, RCVBUFSIZE, 0);     //receive command or login name

		if (!strcmp(echoBuffer, "exit")) {     //in case someone wants to disconnect will be reworked
			dc(clntSocket);     //disconnect client
			return 1;     //return error
		}

		if (!strcmp(echoBuffer, "reg")) {     //registration instead looking for a better way of solving this fork
			reg(clntSocket);     //register
			continue;     //try to login again
		}

		recv(clntSocket, pass, RCVBUFSIZE, 0);     //receive password

		if (!access(path, F_OK)) {     //check if file is present -> as in already registered or not
			//file was present
			break;
		}

		if (!strcmp(echoBuffer, "athy91")) break;     //check who wants to login -> to be overhauled

		if (send(clntSocket, "FALSE", 6, 0) != 6) DieWithError("send() failed");     //send failed signal

	}

	g_stpcpy(path, "user/");     //prepare the path
	g_strlcat(path, echoBuffer, 36);     //make the path
	//g_printf("%s", path);

	p = fopen(path, "r");     //make the user's file
	fscanf(
			p,     //if we made it we should write something to it
			"%s ;%c ;%d.%d.%d ;%d ;%d ;%s ;%d ;%d ;%d ;%d ;%d ;%d ;%d ;%d ;%d ;"
					" Talents: ; Abilities: ; Techniques: ; Personality Traits: ; Reputation: ; Known Locations: ;",
			used[ID].name, &used[ID].Gender, &used[ID].born.year,
			&used[ID].born.month, &used[ID].born.day, &used[ID].height,
			&used[ID].weight, used[ID].House, &used[ID].str, &used[ID].dex,
			&used[ID].inte, &used[ID].vit, &used[ID].wis, &used[ID].sta,
			&used[ID].spd, &used[ID].cha, &used[ID].hly);
	fclose(p);     //close the file

	send(clntSocket, "TRUE", 5, 0);     //send OK signal
	send(clntSocket, (const char *) used + ID * sizeof(caracter),
			sizeof(caracter), 0);
	return 0;
}

void HandleTCPClient(int clntSocket)     //TCP client handling function
{
	gchar echoBuffer[RCVBUFSIZE];     //Buffer for echo string
	int recvMsgSize;     //Size of received message
	static int ID = 0;     //not in use yet

	if (login(clntSocket, ID)) return;     //try to log in client and check if it was successful
	++ID;     //make sure next time we don't overwrite this user

	for (;;) {     //if login was successful keep connection and do the job
		recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0);     //receive commands
		echoBuffer[recvMsgSize] = '\0';     //add closing \0 to be corrected

		if (!strcmp(echoBuffer, "exit")) {     //check if client wants to disconnect
			dc(clntSocket);     //disconnect
			return;
		} else if (!strcmp(echoBuffer, "pic")) pic();     //check if picture change is needed
		else ans();     //otherwise just send something

		send(clntSocket, sent, strlen(sent), 0);     //send the data we made before

	}
	return;
}

void *threadFunc() {
	int clntLen;     //Length of client address data structure

	for (;;) {     //Run forever
		clntLen = sizeof(ClntAddr);     //Set the size of the in-out parameter

		clntSock = accept(servSock, (struct sockaddr *) &ClntAddr,     //Wait for a client to connect
				&clntLen);

		printf("Handling client %s\n", inet_ntoa(ClntAddr.sin_addr));     //clntSock is connected to a client!

		HandleTCPClient(clntSock);
	}
	return NULL;
}

int main(int argc, char *argv[]) {
	struct sockaddr_in echoServAddr;     // Local address
	unsigned short echoServPort;     // Server port

	WSADATA wsaData;     // Structure for WinSock setup communication
	pthread_t pth;     //Structure for threads

	echoServPort = 21;     //Local port

	if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)     // Load Winsock DLL
			{
		fprintf(stderr, "WSAStartup() failed");
		exit(1);
	}

	if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)     // Create socket for incoming connections
		DieWithError("socket() failed");

	// Construct local address structure
	memset(&echoServAddr, 0, sizeof(echoServAddr));     // Zero out structure
	echoServAddr.sin_family = AF_INET;     // Internet address family
	echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY);     // Any incoming interface
	echoServAddr.sin_port = htons(echoServPort);     // Local port

	if (bind(servSock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr))     // Bind to the local address
	< 0) DieWithError("bind() failed");

	for (;;) {
		if (listen(servSock, MAXPENDING) < 0) DieWithError("listen() failed");     // Mark the socket so it will listen for incoming connections
		pthread_create(&pth, NULL, threadFunc, NULL);     //if connection is found create a thread
		usleep(1);     //lets see what our theads are doing
	}

	return EXIT_SUCCESS;
}
