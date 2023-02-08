/*
	Header file for utils.c
*/

#pragma once

#include <stdarg.h>
#include <stdio.h>

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
*	[out] sock - pointer to the SOCKET to connect with
* returns:
*	0 on success, otherwise returns the result of WSAGetLastError()
*/
int connectTCP(char* addr, int port, SOCKET* sock);

/*
* Generic logging function
*
* parameters:
*   [in] fmt - a format string like the used by printf()
*   [in] ... - values to be inserted into the format string, again similar to using printf()
* returns:
*   N/A
*/
void logMessage(char* fmt, ...);