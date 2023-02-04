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

// TODO - finish modifying this code and cleaning up as much as possible
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>

int startCMD();
int createCommandPrompt();


/*
* Initialize the thing
*
*
*/
int startCMD() {
    // create the child process with I/O pipes
    int retCode = createCommandPrompt();
    if (retCode != 0) {
        return retCode;
    }
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