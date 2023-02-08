/*
	Main code
	Handles startup, main control loop, calls to other functions, etc
*/

#include "common.h"
#include "utils.h"
#include "cmd.h"

/*
	Main function
*/
int main() {
	// test code for socket - sends and receives "Hello World!"
	printf("Running...\n");
	initWinSock();
	SOCKET cmdSock = INVALID_SOCKET;
	connectTCP("127.0.0.1", 8080, &cmdSock);
	printf("Connected!\n");
	int res;
	char* msg = "Hello World!";
	res = send(cmdSock, msg, strlen(msg), 0);
	if (res == SOCKET_ERROR) {
		printf("send failed with error %d\n", WSAGetLastError());
		closesocket(cmdSock);
		WSACleanup();
		return;
	}
	printf("Sent info!\n");
	char buff[4096];
	ZeroMemory(buff, 4096);
	res = recv(cmdSock, buff, 4096, 0);
	if (res == SOCKET_ERROR) {
		printf("recv failed with error %d\n", WSAGetLastError());
		closesocket(cmdSock);
		WSACleanup();
		return;
	}
	printf("%s", buff);
	closesocket(cmdSock);
	WSACleanup();
}