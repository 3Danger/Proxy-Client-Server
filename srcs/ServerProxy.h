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
#include <vector>
#include <iostream>
#include <fstream>


#define SIZE_BUFF 128

class ServerProxy {
	bool loop;

	int proxySocketFD;
	int clientFD;
	int serverFD;
	addrinfo * proxyInfo;
	addrinfo * serverInfo;
	char buff[SIZE_BUFF];
	int maxFd;
	fd_set fds;
	std::string logFileName = "log.txt";
public:
	ServerProxy(char const * port, char const * ipAddres);
	~ServerProxy();

	[[noreturn]] void run();
	int connectToClient();
	int connectToServer(char const * port, char const * ipAddres);
private:
	static int makeSocket();
	static void makeBind(int fd, addrinfo * addr);
	addrinfo makeAddrinfoHints();
	void Send(int from, int to, fd_set * fds_set, std::ofstream & outFileLog);
};


#endif //PROXY_CLIENT_SERVER_SEREVERPROXY_H
