
#include "extra.h"
#include "socket.h"

#include <string.h>


SOCKET createServer(const char *serverName) {

	struct addrinfo hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;


	struct addrinfo *address = NULL;

	// Resolve the server address and port
	if (getaddrinfo(serverName, DEFAULT_PORT, &hints, &address) != 0) {
		return INVALID_SOCKET;
	}

	// Create a SOCKET for connecting to server
	SOCKET ListenSocket = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {

		freeaddrinfo(address);
		return INVALID_SOCKET;
	}

	// Setup the TCP listening socket
	if (bind(ListenSocket, address->ai_addr, (int)address->ai_addrlen) == SOCKET_ERROR) {

		freeaddrinfo(address);
		closesocket(ListenSocket);

		return INVALID_SOCKET;
	}

	freeaddrinfo(address);

	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {

		closesocket(ListenSocket);
		return INVALID_SOCKET;
	}

	// Accept a client socket
	SOCKET ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {

		closesocket(ListenSocket);

		return INVALID_SOCKET;
	}

	// No longer need server socket
	closesocket(ListenSocket);

	return ClientSocket;
}


SOCKET createClient(const char *serverName) {

	assert_assume(serverName != NULL);

	struct addrinfo hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	struct addrinfo *address = NULL;

	// Resolve the server address and port
	if (getaddrinfo(serverName, DEFAULT_PORT, &hints, &address) != 0) {
		return INVALID_SOCKET;
	}

	SOCKET ConnectSocket = INVALID_SOCKET;
	// Attempt to connect to an address until one succeeds
	for (struct addrinfo *ptr = address; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			return INVALID_SOCKET;
		}

		// Connect to server.
		if (connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen) == SOCKET_ERROR) {

			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}

		break;
	}

	freeaddrinfo(address);

	return ConnectSocket;
}
