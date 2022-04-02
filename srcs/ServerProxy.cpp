//
// Created by csamuro on 01.04.2022.
//

#include "ServerProxy.h"

ServerProxy::ServerProxy(char const * port, char const * ipAddres)  {
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

int ServerProxy::makeSocket() {

	int res = socket(AF_INET, SOCK_STREAM, 0);
	if (res < 0)
		throw std::runtime_error("Socket error");
	fcntl(res, F_SETFD, fcntl(res, F_GETFD) | O_NONBLOCK);
	std::cout << "socket created" << std::endl;
	return res;
}

void ServerProxy::makeBind(int fd, addrinfo * addr)  {
	int res = bind(fd, addr->ai_addr, addr->ai_addrlen);
	if (res < 0)
		throw std::runtime_error(std::string("Bind error: ") + strerror(errno));
	std::cout << "socket was binded" << std::endl;
}

int ServerProxy::connectToClient() {
	int resListen = listen(proxySocketFD, 1);
	if (resListen < 0)
		throw std::runtime_error("Listen error");
	clientFD = accept(proxySocketFD, proxyInfo->ai_addr, &proxyInfo->ai_addrlen);
	if (clientFD < 0 && errno != EAGAIN)
		std::runtime_error(std::string("connection to client err: ") + strerror(errno));
	return	clientFD;
}

int ServerProxy::connectToServer(const char *port, const char *ipAddres)  {
	addrinfo hints = makeAddrinfoHints();
	if (getaddrinfo(ipAddres, port, &hints, &serverInfo))
		throw std::runtime_error(strerror(errno));
	serverFD = makeSocket();
	if (connect(serverFD, (sockaddr *)serverInfo->ai_addr, serverInfo->ai_addrlen))
		throw std::runtime_error(std::string("cannot connect to server: ") + strerror(errno));
	if (serverFD < 0 && errno != EAGAIN)
		throw std::runtime_error(strerror(errno));
	fcntl(serverFD, F_SETFD, O_NONBLOCK);
	return serverFD;
}

void ServerProxy::run() {
	int readedBytes;

	memset(buff, 0, SIZE_BUFF);
	fcntl(clientFD, F_SETFD, O_NONBLOCK);
	fcntl(serverFD, F_SETFD, O_NONBLOCK);
	while(loop){
		readedBytes = recv(clientFD, buff, SIZE_BUFF, 0);
		if (readedBytes > 0){
			send(serverFD, buff, readedBytes, 0);
			write(1, "From client: ", 13);
			write(1, buff, readedBytes);
			memset(buff, 0, readedBytes + 1);
		}
		usleep(525000);
		readedBytes = recv(serverFD, buff, SIZE_BUFF, 0);
		if (readedBytes > 0) {
			send(clientFD, buff, readedBytes, 0);
			write(1, "From server: ", 13);
			write(1, buff, readedBytes);
			memset(buff, 0, readedBytes + 1);
		}
		usleep(525000);
	}
}

addrinfo ServerProxy::makeAddrinfoHints() {
	addrinfo hints = {0};

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
//	hints.ai_flags = AI_PASSIVE;
	return hints;
}


