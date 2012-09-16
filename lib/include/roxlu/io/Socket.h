#ifndef ROXLU_IO_SOCKETH
#define ROXLU_IO_SOCKETH

#ifdef _WIN32 

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#else
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

namespace roxlu {

class Socket {
public:
	Socket();
	~Socket();
	bool connect(const char* ip, unsigned short int port);
	int read(char* buf, int count);
	int send(const char* buf, int count);
	void close();
private:
#ifdef _WIN32
	WSADATA wsa_data;
	SOCKET sock;
	struct sockaddr_in addr_in;
#else
	int sock;
	struct sockaddr_in addr_in;
#endif
};

} // roxlu

#endif