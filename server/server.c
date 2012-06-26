/*
 * server.c
 *
 *  Created on: 2012.05.25.
 *      Author: Shadow
 */

#include <stdio.h>      /* for printf(), fprintf() */
#include <winsock.h>    /* for socket(),... */
#include <stdlib.h>     /* for exit() */

#define MAXPENDING 5    /* Maximum outstanding connection requests */
#define RCVBUFSIZE 32

void DieWithError(char *errorMessage) /* Error handling function */
{
	perror(errorMessage);
	exit(1);
}
void HandleTCPClient(int clntSocket) /* TCP client handling function */
{
	char echoBuffer[RCVBUFSIZE], sent[20]; /* Buffer for echo string */
	int recvMsgSize, r; /* Size of received message */

	r = (rand() % 6);
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

	/* Receive message from client */
	if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
		DieWithError("recv() failed");

	/* Send received string and receive again until end of transmission */
	while (recvMsgSize > 0) /* zero indicates end of transmission */
	{
		/* Echo message back to client */
		if (send(clntSocket, sent, strlen(sent), 0) != strlen(sent))
			DieWithError("send() failed");

		/* See if there is more data to receive */
		if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
			DieWithError("recv() failed");
	}

	closesocket(clntSocket); /* Close client socket */
}

int main(int argc, char *argv[]) {
	int servSock; /* Socket descriptor for server */
	int clntSock; /* Socket descriptor for client */
	struct sockaddr_in echoServAddr; /* Local address */
	struct sockaddr_in echoClntAddr; /* Client address */
	unsigned short echoServPort; /* Server port */
	int clntLen; /* Length of client address data structure */
	WSADATA wsaData; /* Structure for WinSock setup communication */

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
	if (listen(servSock, MAXPENDING) < 0) DieWithError("listen() failed");

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
	/* NOT REACHED */
}
