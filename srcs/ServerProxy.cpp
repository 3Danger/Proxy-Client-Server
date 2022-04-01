//
// Created by csamuro on 01.04.2022.
//

#include "ServerProxy.h"

ServerProxy::ServerProxy(char const * port, char const * ipAddres) throw() {
	clientFD = -1;
	serverFD = -1;
	addrinfo hints = makeAddrinfoHints();
	if (getaddrinfo(ipAddres, port, &hints, &ProxyInfo))
		throw std::runtime_error(strerror(errno));
	ProxySocketFD = makeSocket();
	makeBind(ProxySocketFD, ProxyInfo);
}

ServerProxy::~ServerProxy() {
	close(ProxySocketFD);
	std::cout << "ProxySocketFD was closed from destructor!" << std::endl;
	freeaddrinfo(ProxyInfo);
	std::cout << "ProxyInfo was freed from destructor!" << std::endl;
	close(clientFD);
	std::cout << "clientFD was closed from destructor!" << std::endl;
}

int ServerProxy::makeSocket() throw(){

	int res = socket(AF_INET, SOCK_STREAM, 0);
	if (res < 0)
		throw std::runtime_error("Socket error");
	fcntl(res, F_SETFD, fcntl(res, F_GETFD) | O_NONBLOCK);
	std::cout << "socket created" << std::endl;
	return res;
}

int ServerProxy::makeBind(int fd, addrinfo * addr) throw() {
	int res = bind(fd, addr->ai_addr, addr->ai_addrlen);
//	int res = bind(ProxySocketFD, ProxyInfo->ai_addr, ProxyInfo->ai_addrlen);
	if (res < 0)
		throw std::runtime_error(std::string("Bind error: ") + strerror(errno));
	std::cout << "socket was binded" << std::endl;
}

int ServerProxy::connectToClient() throw(){
	int resListen = listen(ProxySocketFD, 1);
	clientFD = accept(ProxySocketFD, ProxyInfo->ai_addr, &ProxyInfo->ai_addrlen);
	if (clientFD < 0 && errno != EAGAIN)
		std::runtime_error(std::string("connection to client err: ") + strerror(errno));
	return	clientFD;
}

int ServerProxy::connectToServer(const char *port, const char *ipAddres) throw() {
	addrinfo hints = makeAddrinfoHints();
	if (getaddrinfo(ipAddres, port, &hints, &serverInfo))
		throw std::runtime_error(strerror(errno));
	serverFD = connect(ProxySocketFD, serverInfo->ai_addr, serverInfo->ai_addrlen);
	if (serverFD < 0 && errno != EAGAIN)
		throw std::runtime_error(strerror(errno));
	return serverFD;
}

addrinfo ServerProxy::makeAddrinfoHints() {
	addrinfo hints = {0};

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	return hints;
}


