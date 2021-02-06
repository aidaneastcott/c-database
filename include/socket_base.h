
#pragma once
#ifndef SOCKET_BASE_H
#define SOCKET_BASE_H


// Compiler guard for external C code in C++
#ifdef __cplusplus
extern "C" {
#endif


// Mandatory macro to disable infrequently used Win32 libraries
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif


#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>


// Undefine invasive min & max macros
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif


#ifdef _MSC_VER
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")
#endif


// Default winsock settings
#define DEFAULT_PORT          "27015"
#define DEFAULT_SERVER_NAME   "localhost"


#ifdef __cplusplus // extern "C"
}
#endif


#endif // SOCKET_BASE_H
