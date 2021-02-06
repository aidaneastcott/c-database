
#pragma once
#ifndef SOCKET_H
#define SOCKET_H


// Compiler guard for external C code in C++
#ifdef __cplusplus
extern "C" {
#endif


#include "socket_base.h"


// Prototypes for creating different types of sockets
SOCKET createServer(const char *);
SOCKET createClient(const char *);


#ifdef __cplusplus // extern "C" {
}
#endif


#endif // SOCKET_H
