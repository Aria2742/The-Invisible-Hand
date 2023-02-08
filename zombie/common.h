/*
	Header file containing common macros and imports (such as standard/windows libraries)
*/

#pragma once

#define WIN32_LEAN_AND_MEAN // exclude less common APIs from windows.h - also prevents winsock redefinition errors

#include <windows.h>
#include <winsock2.h> // include the winsock library

#pragma comment(lib, "ws2_32.lib") // link the winsock library