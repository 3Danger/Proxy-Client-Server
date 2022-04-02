//
// Created by csamuro on 01.04.2022.
//

#include "ServerProxy.h"

ServerProxy::ServerProxy(char const * port, char const * ipAddres) throw() {
	clientFD = -1;
	serverFD = -1;
	addrinfo hints = makeAddrinfoHints();
	if (getaddrinfo(ipAddres, port, &hints, &proxyInfo))
		throw std::runtime_error(strerror(errno));
	proxySocketFD = makeSocket();
	makeBind(proxySocketFD, proxyInfo);
}

ServerProxy::~ServerProxy() {
	close(proxySocketFD);
	std::cout << "proxySocketFD was closed from destructor!" << std::endl;
	freeaddrinfo(proxyInfo);
	std::cout << "proxyInfo was freed from destructor!" << std::endl;
	close(clientFD);
	std::cout << "clientFD was closed from destructor!" << std::endl;
	close(serverFD);
	std::cout << "serverFD was closed from destructor!" << std::endl;
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
//	int res = bind(proxySocketFD, proxyInfo->ai_addr, proxyInfo->ai_addrlen);
	if (res < 0)
		throw std::runtime_error(std::string("Bind error: ") + strerror(errno));
	std::cout << "socket was binded" << std::endl;
}

int ServerProxy::connectToClient() throw(){
	int resListen = listen(proxySocketFD, 1);
	if (resListen < 0)
		throw std::runtime_error("Listen error");
	clientFD = accept(proxySocketFD, proxyInfo->ai_addr, &proxyInfo->ai_addrlen);
	if (clientFD < 0 && errno != EAGAIN)
		std::runtime_error(std::string("connection to client err: ") + strerror(errno));
	fcntl(clientFD, F_SETFD, O_NONBLOCK);
	return	clientFD;
}

int ServerProxy::connectToServer(const char *port, const char *ipAddres) throw() {
	addrinfo hints = makeAddrinfoHints();
	if (getaddrinfo(ipAddres, port, &hints, &serverInfo))
		throw std::runtime_error(strerror(errno));
	serverFD = makeSocket();
	connect(serverFD, (sockaddr *)serverInfo->ai_addr, serverInfo->ai_addrlen);
//	if (serverFD == 0)
//		throw std::runtime_error(std::string("serverFD == 0: ") + strerror(errno));
	if (serverFD < 0 && errno != EAGAIN)
		throw std::runtime_error(strerror(errno));
	fcntl(serverFD, F_SETFD, O_NONBLOCK);
	return serverFD;
}

[[noreturn]] void ServerProxy::run() {
	int readedBytes;

	memset(buff, 0, SIZE_BUFF);
	fcntl(clientFD, F_SETFD, O_NONBLOCK);
	fcntl(serverFD, F_SETFD, O_NONBLOCK);
	while(true){
		readedBytes = recv(clientFD, buff, SIZE_BUFF, 0);
		if (readedBytes > 0){
			send(serverFD, buff, readedBytes, 0);
			write(1, "From client: ", 13);
			write(1, buff, readedBytes);
			memset(buff, 0, readedBytes + 1);
		}
		write(1, "timer 1\n", 8);
		usleep(125000);
		readedBytes = recv(serverFD, buff, SIZE_BUFF, 0);
		if (readedBytes > 0) {
			send(clientFD, buff, readedBytes, 0);
			write(1, "From server: ", 13);
			write(1, buff, readedBytes);
			memset(buff, 0, readedBytes + 1);
		}
		usleep(125000);
		write(1, "timer 2\n", 8);
	}
}

addrinfo ServerProxy::makeAddrinfoHints() {
	addrinfo hints = {0};

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
//	hints.ai_flags = AI_PASSIVE;
	return hints;
}


