/*
 * testserver.c
 *
 *  Created on: 2012.05.26.
 *      Author: Shadow
 */

#include <stdio.h>      /* for printf(), fprintf() */
#include <winsock.h>    /* for socket(),... */
#include <stdlib.h>     /* for exit() */
#include <pthread.h>

#define MAXPENDING 5    /* Maximum outstanding connection requests */
#define RCVBUFSIZE 32
#define LN 1

char sent[20];
int servSock; /* Socket descriptor for server */
int clntSock; /* Socket descriptor for client */
struct sockaddr_in echoClntAddr; /* Client address */

typedef struct birth_date {
	int year;
	int month;
	int day;
} BORN;

typedef struct character {
	BORN born;
	char name[31], House[31], Gender;
	int height, weight, age, rep, str, dex, inte;
	int spd, vit, wis, sta, cha, hly;
	int kl[LN], pt[20], tech[20], abil[20], talent[20];
} CHAR;

extern int usleep();

int random(int t, int i = 0) {
	static int l[10];
	int r;
	do {
		r = (rand() % t);
	} while (r == l[i]);
	l[i] = r;
	return r;
}

void ans() {
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

void pic() {
	int r;
	char tmp[10];
	r = random(8, 2) + 1;
	itoa(r, tmp, 10);
	strcpy(sent, "pic/");
	strcat(sent, tmp);
	strcat(sent, ".jpg");
	//printf("%s\n", sent);
	return;
}

void DieWithError(char *errorMessage) /* Error handling function */
{
	perror(errorMessage);
	exit(1);
}

void dc(int clntSocket) {
	printf("Disconnecting client %s\n", inet_ntoa(echoClntAddr.sin_addr));
	closesocket(clntSocket);
	return;
}

int reg(char *chname) {
	FILE *p;
	CHAR new;
	*p = fopen(chname, 'w');
	fprintf(p,
			"Name: ;Gender: ;Born: ;height; weight: ; House: ; STR; DEX; INT; SPD; VIT; WIS; STA; CHA; HLY;"
					" Talents: ; Abilities: ; Techniques: ; Personality Traits: ; Reputation: ; Known Locations: ;");
	return 0;
}

int login(int clntSocket) {

	char echoBuffer[RCVBUFSIZE]; /* Buffer for echo string */
	int recvMsgSize; /* Size of received message */

	for (;;) {
		if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
			DieWithError("recv() failed");

		if (!strcmp(echoBuffer, "athy91")) {
			break;
		}

		if (!strcmp(echoBuffer, "exit")) {
			dc(clntSocket);
			return 1;
		}

		if (!strcmp(echoBuffer, "reg")) {
			reg();
		}

		if (send(clntSocket, "FALSE", 6, 0) != 6) DieWithError("send() failed");

	}
	if (send(clntSocket, "TRUE", 5, 0) != 5) DieWithError("send() failed");
	return 0;
}

void HandleTCPClient(int clntSocket) /* TCP client handling function */
{
	char echoBuffer[RCVBUFSIZE]; /* Buffer for echo string */
	int recvMsgSize; /* Size of received message */

	if (login(clntSocket)) return;

	for (;;) {
		recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0);
		echoBuffer[recvMsgSize] = '\0';

		if (!strcmp(echoBuffer, "exit")) {
			dc(clntSocket);
			return;
		} else if (!strcmp(echoBuffer, "pic")) pic();
		else ans();

		send(clntSocket, sent, strlen(sent), 0);

	}
	return;
}

void *threadFunc() {
	int clntLen; /* Length of client address data structure */
	for (;;) /* Run forever */
	{
		/* Set the size of the in-out parameter */
		clntLen = sizeof(echoClntAddr);

		/* Wait for a client to connect */
		if ((clntSock = accept(servSock, (struct sockaddr *) &echoClntAddr,
				&clntLen)) < 0) DieWithError("accept() failed");

		/* clntSock is connected to a client! */

		printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));

		HandleTCPClient(clntSock);

	}

	return NULL;
}

int main(int argc, char *argv[]) {
	struct sockaddr_in echoServAddr; /* Local address */
	unsigned short echoServPort; /* Server port */

	WSADATA wsaData; /* Structure for WinSock setup communication */
	pthread_t pth;

	echoServPort = 21; /* first arg:  Local port */

	if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) /* Load Winsock 2.0 DLL */
	{
		fprintf(stderr, "WSAStartup() failed");
		exit(1);
	}

	/* Create socket for incoming connections */
	if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		DieWithError("socket() failed");

	/* Construct local address structure */
	memset(&echoServAddr, 0, sizeof(echoServAddr)); /* Zero out structure */
	echoServAddr.sin_family = AF_INET; /* Internet address family */
	echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
	echoServAddr.sin_port = htons(echoServPort); /* Local port */

	/* Bind to the local address */
	if (bind(servSock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr))
			< 0) DieWithError("bind() failed");

	/* Mark the socket so it will listen for incoming connections */

	for (;;) {
		if (listen(servSock, MAXPENDING) < 0) DieWithError("listen() failed");
		pthread_create(&pth, NULL, threadFunc, NULL);
		if (listen(servSock, MAXPENDING) > 0)
			pthread_create(&pth, NULL, threadFunc, NULL);
		usleep(1);
	}

	return EXIT_SUCCESS;
}
