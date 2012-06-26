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

char sent[20];
int servSock; /* Socket descriptor for server */
int clntSock; /* Socket descriptor for client */
struct sockaddr_in echoClntAddr; /* Client address */

int random(int t){
	static int l=0;
	int r;
	do {
		r = (rand() % t);
	}while (r == l);
	l=r;
	return r;
}

void ans() {
	int r;
	r = random(6);
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
	r = random(8);
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
		if (send(clntSocket, "FALSE", 6, 0) != 6)
			DieWithError("send() failed");

	}
	if (send(clntSocket, "TRUE", 5, 0) != 5)
		DieWithError("send() failed");
	return 0;
}

void HandleTCPClient(int clntSocket) /* TCP client handling function */
{
	char echoBuffer[RCVBUFSIZE]; /* Buffer for echo string */
	int recvMsgSize; /* Size of received message */

	if (login(clntSocket))
		return;

	for (;;) {
		/* Receive message from client */
		if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0) {
			DieWithError("recv() failed");
		}

		echoBuffer[recvMsgSize] = '\0';

		//printf("%s\n", echoBuffer);

		if (!strcmp(echoBuffer, "exit")) {
			dc(clntSocket);
			return;
		} else if (!strcmp(echoBuffer, "pic")) {
			pic();
		} else ans();

		/* Echo message back to client */
		if (send(clntSocket, sent, strlen(sent), 0) != strlen(sent))
			DieWithError("send() failed");
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
				&clntLen)) < 0)
			DieWithError("accept() failed");

		/* clntSock is connected to a client! */

		printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));

		HandleTCPClient(clntSock);

		usleep(1);

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
			< 0)
		DieWithError("bind() failed");

	/* Mark the socket so it will listen for incoming connections */

	for (;;) {
		if (listen(servSock, MAXPENDING) < 0)
			DieWithError("listen() failed");
		pthread_create(&pth, NULL, threadFunc, NULL);
		if (listen(servSock, MAXPENDING) > 0)
			pthread_create(&pth, NULL, threadFunc, NULL);
		usleep(1);
	}

	return EXIT_SUCCESS;
}
