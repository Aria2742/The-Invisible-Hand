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
	initWinSock();
	SOCKET cmdSock;
	connectTCP("127.0.0.1", 8080, cmdSock);
}