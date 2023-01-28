/*
	Various utility functions that are used in other files
*/

#include "utils.h"
#include "common.h"

// Create a structure (and global instance of it) that holds all the globals used in this file
// This helps organize the globals and make it more obvious where each global is coming from
struct UtilsGlobals {
	WSADATA wsa;

} utilsGlobals;

/*
* Initialize the WinSock library. This needs to be done before calling any other WinSock functions
* NOTE: This initializes the global variable 'utilGlobals.wsa'
* 
* parameters:
*	N/A
* returns:
*	0 on success, otherwise returns the result of WSAGetLastError()
*/
int initWinSock() {
	if (WSAStartup(MAKEWORD(2, 2), &utilGlobals.wsa) != 0)
	{
		return WSAGetLastError();
	}
	return 0;
}

/*
* Connect to a server using TCP
* 
* parameters:
*	[in] addr - a string containing the IPv4 address of the server to connect to
*	[in] port -  which port to connect to
*	[out] sock - a SOCKET structure to hold the connected socket
* returns:
*	0 on success, otherwise returns the result of WSAGetLastError()
*/
int connectTCP(char* addr, int port, Socket* sock) {
	// create the socket and make sure it worked
	*sock = socket(AF_INET, SOCK_STREAM, 0)
	if (*sock == INVALID_SOCKET)
	{
		return WSAGetLastError();
	}
	// create the server info structure
	struct sockaddr_in server
	server.sin_addr.s_addr = inet_addr(addr);
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	// connect to the server
	if (connect(cmdSock, (struct sockaddr*)&server, sizeof(server)) < 0)
	{
		return WSAGetLastError();
	}
	// return success
	return 0;
}