/*
	Header file for utils.c
*/

#pragma once

#include <winsock2.h> // include the winsock library

#pragma comment(lib, "ws2_32.lib") // link the winsock library

/*
* Initialize the WinSock library. This needs to be done before calling any other WinSock functions
* NOTE: This initializes the global variable 'utilGlobals.wsa'
*
* parameters:
*	N/A
* returns:
*	0 on success, otherwise returns the result of WSAGetLastError()
*/
int initWinSock();

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
int connectTCP(char* addr, int port, SOCKET sock);