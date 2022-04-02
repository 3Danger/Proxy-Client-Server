//
// Created by csamuro on 01.04.2022.
//

#ifndef PROXY_CLIENT_SERVER_SEREVERPROXY_H
#define PROXY_CLIENT_SERVER_SEREVERPROXY_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <exception>
#include <string>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>

#define SIZE_BUFF 256

class ServerProxy {
	int proxySocketFD;
	int clientFD;
	int serverFD;
	addrinfo * proxyInfo;
	addrinfo * serverInfo;
	char buff[SIZE_BUFF];
public:
	ServerProxy(char const * port, char const * ipAddres) throw();
	~ServerProxy();

	[[noreturn]] void run();
	int connectToClient() throw();
	int connectToServer(char const * port, char const * ipAddres) throw();




private:
	static int makeSocket() throw();
	static int makeBind(int fd, addrinfo * addr) throw();
	addrinfo makeAddrinfoHints();
};


#endif //PROXY_CLIENT_SERVER_SEREVERPROXY_H
